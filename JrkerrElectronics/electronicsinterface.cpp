#include "electronicsinterface.h"
#include <QTextStream>
#include <QMap>
#include <QDebug>

ElectronicsInterface::ElectronicsInterface():initialized_(false),BUFF_SIZE(10)
{
}

bool ElectronicsInterface::cleanUp()
{
    bool result;
    if (initialized_)
    {
        //Perform cleanup for all motors.
        bool result = true;

        for (int i = 0; i < motors_.length(); i++) {
            result = result && motors_[i]->cleanUp();
        }

        if (result)
        {
            error_string_="";
            NmcHardReset(0xFF,&error_string_);
            NmcShutdown(&error_string_);
        }
    }
    else
    {
        result = true;
    }
    return result;
}

QList<Motor*> ElectronicsInterface::getMotors() {
    return motors_;
}

Motor* ElectronicsInterface::getMotor(int id) {
     return motors_[id];
}

bool ElectronicsInterface::isInitialized() {
    return initialized_;
}

void ElectronicsInterface::reset() {
    error_string_="";
    NmcHardReset(X_Y_Z_GROUP_ADDRESS,&error_string_);
}

void ElectronicsInterface::resetPosition(){
    foreach(Motor* m,getMotors()){
        m->resetPosition();
    }

    cmotion_.resetPosition();
    NP::resetStates();
    qDebug()<<"motors reset";
}

void ElectronicsInterface::forceStop(){
    foreach(Motor* m,getMotors()){
        m->foreceStop();
    }

    initialized_=false;
}

QString ElectronicsInterface::initialize(QDomNode node, QString portLocation) {
    if (!initialized_) {
        QString result = "";
        QTextStream ss(&result,QIODevice::ReadWrite);

        // Load the settings for the printer
        result = loadSettings(node);
        if (result.compare("") != 0) {
            qDebug()<<"Load Setttings: "<<result;
            return result;
        }

        // Check if a valid port location was passed; if not, load the one from the loadSettings command
        if (""==portLocation && ""!=COM_PORT) {
            QTextStream port(&portLocation,QIODevice::WriteOnly);
            port<<"COM"<<COM_PORT; //// This is Windows Specific
        }

        // Connect to printer
        QString error_string="";
        unsigned int numModulesFound = NmcInit(portLocation, BAUD_RATE,&error_string);
        if (numModulesFound < NUM_MODULES ) { //|| !error_string.isNull())
            ss << "Found " << numModulesFound << " modules but need " << NUM_MODULES << " modules.";
            ss << " Error string from NmcInit is "<<error_string;

            qDebug() << "Found " << numModulesFound << " modules but need " << NUM_MODULES << " modules.";
            qDebug() << " Error string from NmcInit is "<<error_string;
            return result;
        }

        // Initialize all motors.
        foreach (Motor* m, motors_) {
            result = m->initialize();
            if (result !="") {
                qDebug()<<result;
                return result;
            }
        }
        createCoodinatedMotion();// make the cmotion_ instance if motors_.size()>=3
        initialized_ = true;
    }
    return "";
}

QString ElectronicsInterface::loadSettings(QDomNode node) {
    QString result="";
    QTextStream ss(&result,QIODevice::WriteOnly);
    QString temp;

    bool haskids;

    temp=node.nodeName();
    if (!("electronics" == temp.toLower())) {
        ss << "\nBad node passed to load settings expected 'electronics' found '" <<temp <<"'\n";
        qDebug() << "\nBad node passed to load settings expected 'electronics' found '" <<temp <<"'\n";

        return result;
    }

    haskids=node.hasChildNodes();
    if (!haskids) {
        ss <<"\n electronics has no children, needs controller and actuator"<<"\n";
        return result;
    }


    QDomNodeList electronicsChildren = node.childNodes();
    for (unsigned int i = 0; i < electronicsChildren.length(); i++) {
        QDomNode echild = electronicsChildren.at(i);
        if (echild.isComment()) {continue;}
        if ("controller"==echild.nodeName().toLower()) {
            if (!echild.hasChildNodes()) {return result="\nCONTROLLER HAS NO CHILDREN";}
            QDomNodeList controllerChildren = echild.childNodes();
            for (unsigned int j = 0; j < controllerChildren.length(); j++) {
                QDomNode cchild = controllerChildren.at(j);
                QDomElement el;
                QString name = cchild.nodeName();
                if (!cchild.isElement()) {
                    continue;
                }
                if ("comPort"==name) {
                    el=cchild.toElement();
                    COM_PORT = el.text();
                } else if ("baudRate" == name) {
                    el=cchild.toElement();
                    BAUD_RATE = el.text().toUInt();
                } else if ("xyzGroupAddress"==name) {
                    el=cchild.toElement();
                    X_Y_Z_GROUP_ADDRESS = (byte)el.text().toUInt();
                }else if ("buffSize" ==name){
                    el=cchild.toElement();
                    BUFF_SIZE  = el.text().toUInt();
                }else if ("numModules"==name){
                    el=cchild.toElement();
                    NUM_MODULES = el.text().toUInt();
                }else if("bufferRate"==name){
                    el=cchild.toElement();
                    int value  = el.text().toInt();
                    if (value>120) {frequency_ = P_120HZ;}
                    else if (value>=60) {frequency_=P_60HZ;}
                    else {frequency_ = P_30HZ;}
                } else {
                      continue;
                }
            }
        } else if ("actuator" == echild.nodeName()) {
            QDomElement t=echild.namedItem("type").toElement();
            QDomElement idNode=echild.namedItem("id").toElement();
            int id = idNode.text().toInt();
            if ("DC/Encoder"==t.text()) {

                createMotor(echild);
            } else {
                ss<<"\n Unknown type of actuator id#"<<id<< "\n type is "<<t.text()<<"\n";
                return result;
            }
        } else {
            continue;
        }
    }
    return result;
}

void ElectronicsInterface::createMotor(QDomNode sourceDomNode) {
        QString tempString;
        QDomElement el;

        int ID;
        byte ADDRESS;
        double COUNTS_PER_REV; //Number of counts for 1 U of motion.
        double TICKS_PER_SECOND; //Number of ticks that a motor makes per second.

        short KP, KD, KI, IL, EL; //Damping contraints (look up procedure for critically damping motors for more details, also JKerr has documentation)
        byte OL, CL, SR, DB; //See JKerr's Document for more details on his motor byte

        double minComPos, maxComPos, minComVel, maxComVel, minComAcc, maxComAcc; //used to set min max values for motor
        bool reversed;

        // XML parsing to make the motor
        {
        QDomNodeList actuatorChildren = sourceDomNode.childNodes();
        for (unsigned int i = 0; i < actuatorChildren.length(); i++) {
            QDomNode achild = actuatorChildren.at(i);
            if (achild.isComment() || !achild.isElement()) {continue;}
            if ("id"==achild.nodeName()) {
                el=achild.toElement();
                tempString=el.text();
                ID=tempString.toUInt();
            } else if ("address"==achild.nodeName()) {
                el=achild.toElement();
                tempString=el.text();
                ADDRESS = tempString.toUInt();
            } else if ("countsPerRevolution"==achild.nodeName()) {
                 el=achild.toElement();
                 tempString = el.text();
                 COUNTS_PER_REV = tempString.toDouble();
            } else if ("kp"==achild.nodeName()) {
                el=achild.toElement();
                tempString = el.text();
                KP = tempString.toShort();
            } else if ("kd"==achild.nodeName()) {
                el=achild.toElement();
                tempString = el.text();
                KD = tempString.toShort();
            } else if ("ki"==achild.nodeName()) {
                el=achild.toElement();
                tempString = el.text();
                KI = tempString.toShort();
            } else if ("il"==achild.nodeName()) {
                el=achild.toElement();
                tempString = el.text();
                IL = tempString.toShort();
            } else if ("ol"==achild.nodeName()) {
                el=achild.toElement();
                tempString = el.text();
                OL = (byte)tempString.toUInt();
            } else if ("cl"==achild.nodeName()) {
                el=achild.toElement();
                tempString = el.text();
                CL = (byte)tempString.toUInt();
            } else if ("el"==achild.nodeName()) {
                el=achild.toElement();
                tempString = el.text();
                EL = tempString.toShort();
            } else if ("sr"==achild.nodeName()) {
                el=achild.toElement();
                tempString = el.text();
                SR = (byte)tempString.toUInt();
            } else if ("db"==achild.nodeName()) {
                el=achild.toElement();
                tempString = el.text();
                DB = (byte)tempString.toUInt();
            } else if ("ticksPerSecond"==achild.nodeName()) {
                el=achild.toElement();
                tempString = el.text();
                TICKS_PER_SECOND = tempString.toDouble();
            } else if ("reverse"==achild.nodeName()) {
                el=achild.toElement();
                tempString = el.text();
                reversed = (bool)tempString.toInt();
            }else if("minCommandedPosition"==achild.nodeName()){
                el=achild.toElement();
                tempString = el.text();
                minComPos = tempString.toDouble();
            }else if("maxCommandedPosition"==achild.nodeName()){
                el=achild.toElement();
                tempString = el.text();
                maxComPos = tempString.toDouble();
            }else if("minCommandedVelocity"==achild.nodeName()){
                el=achild.toElement();
                tempString = el.text();
                minComVel = tempString.toDouble();
            }else if("maxCommandedVelocity"==achild.nodeName()){
                el=achild.toElement();
                tempString = el.text();
                maxComVel = tempString.toDouble();
            }else if("minCommandedAcceleration"==achild.nodeName()){
                el=achild.toElement();
                tempString = el.text();
                minComAcc = tempString.toDouble();
            }else if("maxCommandedAcceleration"==achild.nodeName()){
                el=achild.toElement();
                tempString = el.text();
                maxComAcc = tempString.toDouble();
            }else{
                continue;
            }

        }
    }
        motors_.append(new Motor(ID,ADDRESS,COUNTS_PER_REV,TICKS_PER_SECOND,KP,KD,KI,IL,OL,CL,EL,SR,DB,
                                 minComPos,maxComPos, minComVel, maxComVel, minComAcc, maxComAcc));
        motors_.last()->setReversed(reversed);
}

CoordinatedMotion* ElectronicsInterface::getCoordinatedMotion() {
    return &cmotion_;
}

void ElectronicsInterface::createCoodinatedMotion() {
    QMap<int,Motor*> tempM;
    for(int i=0;i<motors_.size();i++) {
        tempM[i+1]=motors_[i];
    }
    cmotion_.setMotors(tempM);
    cmotion_.setGroupAddress(X_Y_Z_GROUP_ADDRESS);
    cmotion_.setBufferSize(BUFF_SIZE);

}

void ElectronicsInterface::waitOnMove() {
    if (initialized_==false) {return;}
    bool ismoving=true;
    while(ismoving) {
        ismoving = false;
        foreach(Motor* m,motors_) {
            ismoving = ismoving||m->moving();
        }
    }
}

QString ElectronicsInterface::getErrors() {
    QString returnstring = "";
    QTextStream ss(&returnstring,QIODevice::WriteOnly);
    ss<<"\nEInterface: "<<error_string_;
    foreach(Motor* m,motors_) {
        ss<<"\n\tMotor: "<<m->getID()<<":"<<m->getErrors();
    }

    ss<<"\n\tCMotion:"<<cmotion_.getErrors();

    return returnstring;
}
