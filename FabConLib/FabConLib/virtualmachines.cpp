#include "virtualmachines.h"
#include <stdio.h>
#include <QTextStream>
#include <QFile>
#include "jsnpath.h"

#include "testing/util.h"
#include "xdflvoxel.h"

VMPrototype::VMPrototype():initialized_(false),statesize_(1),laststate_(statesize_,0.0){}

bool VMPrototype::isInitialized() {
    return initialized_;
}

int VMPrototype::nstatesize() {
    return statesize_;
}

void VMPrototype::setComPort(QString comPort) {
    if (!comPort.isNull()) {comPort_=comPort;}
}

QString VMPrototype::getErrors() {
    return error_string_;
}

void VMPrototype::dumpstates(){
}

bool VMPrototype::executeNPath(NPath path) {
    return false;
}

bool VMPrototype::executeRelativeNPath(NPath path) {
    return false;
}

State VMPrototype::currentState() {
    return State(statesize_,0.0);
}

QVector<double> VMPrototype::currentPosition(){
    State cp = currentState();
    return xyzmotion->positionFromState(&cp);
}

bool VMPrototype::moveTo(double x, double y, double z, double speed) {
    State startstate = currentState();
    FabPoint cp = pointFromQVector(xyzmotion->positionFromState(&startstate));
    FabPoint tp;
    tp.x = x;
    tp.y = y;
    tp.z = z;
    FabPoint delta = subtractpoints(tp,cp);
    NPath n = xyzmotion->pathTo(delta.x,delta.y,delta.z,speed);
    return executeRelativeNPath(n);
}

bool VMPrototype::move(double x, double y, double z, double speed) {
    NPath n = xyzmotion->pathTo(x,y,z,speed);
    return executeRelativeNPath(n);
}

bool VMPrototype::forceStop(){
    return true;
}

void VMPrototype::resetPosition(){
    laststate_= State(statesize_,0.0);
    return;
}

void VMPrototype::stop(){
    return;
}


QScriptEngine* VMPrototype::makeEngine(){


    QScriptEngine* engine = new QScriptEngine();

    //Add XYZMotion to engine_.

    QScriptValue scriptXYZ = engine->newQObject(&jsxyz_);
    engine->globalObject().setProperty("xyz",scriptXYZ);

    // State object
    int statesize = nstatesize();
    engine->globalObject().setProperty("statesize",statesize);
    QScriptValue stateProto = engine->newArray(statesize);
    for(int j=0;j<statesize;j++) {
        stateProto.setProperty(j,0);
    }
    stateProto.setProperty("distance", 0);
    engine->globalObject().setProperty("State", engine->newFunction(newJsState,stateProto));

    QScriptValue npathProto = engine->newArray(0);
    engine->globalObject().setProperty("NPath",engine->newFunction(newJsNPath,npathProto));

    return engine;
}


void VMPrototype::loadConfig(QDomDocument document) {
    QDomElement root = document.documentElement();

//    frequency_ = 30;
//    idtostatemap_.clear();
//    idtostatemap_[0]=1;
//    statesize_ = 2;



    // XYZMOTION
    QDomNode motion = root.namedItem("motion");
    xyzmotion = new XYZMotion(motion);
    xyzmotion->setFrequency(frequency_);
    xyzmotion->setIdMap(idtostatemap_);
    jsxyz_.setXYZ(xyzmotion);

    // BAYS
    QDomNode tools = root.namedItem("tool");
    QDomNodeList toolChildren  = tools.childNodes();
    bays.clear();
    bays = QList<Bay*>();
    for (unsigned int k = 0; k < toolChildren.length(); k++) {
        if ("bay"==toolChildren.at(k).nodeName().toLower()) {
            bays.append(new Bay(toolChildren.at(k)));
            bays.last()->setIdMap(idtostatemap_);
            bays.last()->setEngine(makeEngine());
        }
    }



    initialized_ = true;
}

void VMPrototype::waitMove(){return;}



///////////////////////////////////////////////////////////////////////////////////////////

VirtualPrinter::VirtualPrinter():VMPrototype() {}

void VirtualPrinter::loadConfig(QDomDocument document) {

    QDomElement root = document.documentElement();
    QDomNode electronics = root.namedItem("electronics");

    qDebug()<<"Node:"<<electronics.nodeName();
    // ELECTRONICS INTERFACE
    eInterface.initialize(electronics,comPort_);
    initialized_  = eInterface.isInitialized();
    qDebug()<<"Electronics: "<< QString(initialized_ ? "initialized" : "not initialized");
    idtostatemap_ = eInterface.getCoordinatedMotion()->getIdToStateIndexMap();
    frequency_ = eInterface.getCoordinatedMotion()->getFrequency();
    statesize_ = eInterface.getCoordinatedMotion()->getNumberOfAxes()+1;

    laststate_ = State(statesize_,0);


    // XYZMOTION
    QDomNode motion = root.namedItem("motion");
    xyzmotion =new XYZMotion(motion);
    xyzmotion->setFrequency(frequency_);
    xyzmotion->setIdMap(idtostatemap_);
    jsxyz_.setXYZ(xyzmotion);


    //BAYS
    QDomNode tools = root.namedItem("tool");
    QDomNodeList toolChildren  = tools.childNodes();

    bays.clear();
    bays = QList<Bay*>();

    for(uint k=0; k<toolChildren.length();k++){
        if ("bay"==toolChildren.at(k).nodeName().toLower()){
            bays.append(new Bay(toolChildren.at(k)));
            bays.last()->setIdMap(idtostatemap_);
            bays.last()->setStateSize(statesize_);
            bays.last()->setEngine(makeEngine());
        }
    }


}

void VirtualPrinter::dumpstates(){}

QString VirtualPrinter::getErrors(){

    QString returnstring = "";
    QTextStream ss(&returnstring,QIODevice::WriteOnly);
    foreach(Motor* m,eInterface.getMotors()) {
        ss<<"\nMotor "<<m->getID()<<":"<<m->getErrors();
    }
    foreach(Bay* b,bays) {
        ss<<"\nBay "<<b->getId()<<":"<<b->getErrors();
    }
    ss<<"\nCMotion:"<<eInterface.getCoordinatedMotion()->getErrors();
    ss<<"\nVM:"<<error_string_;


    return returnstring;
}


bool VirtualPrinter::move(double x, double y, double z, double speed){
    QMap<QString,axis> axes = xyzmotion->getAxes();

    // If more than three axes, used the default move command
    if (3<=axes.keys().size()){
        return VMPrototype::move(x,y,z,speed);
    }

    //ERRROR this is a kludge
    //if less than three axes, command a each motor to move independantly

    double d = sqrt(x*x+y*y+z*z);
    foreach(QString s,axes.keys()){
        Motor* m = eInterface.getMotor(axes[s].actuatorID);
        double scale = axes[s].revPerDist;
        double to_move=0;
        if (s.toLower()=="x"){
            to_move=x;
        }else if (s.toLower()=="y"){
            to_move=y;
        }else if (s.toLower()=="z"){
            to_move=z;
        }
        //Each motor moves its distance at a scalled speed and acceleration
        qDebug()<<"moving";
        m->moveRelative(scale*to_move,scale*to_move/d*speed,scale*xyzmotion->getAcceleration());
    }
    return true;
}
void VirtualPrinter::waitMove(){
    foreach(Motor* m,eInterface.getMotors()){
        m->waitMove();
    }
}


bool VirtualPrinter::executeNPath(NPath path) {
//    State test = path.getState(0);
//    qDebug()<<"VP Start State is "<<test;

    if(!initialized_) {return false;}
    eInterface.getCoordinatedMotion()->initializePathMode();
    laststate_ = path.lastAbsolute();
    return eInterface.getCoordinatedMotion()->moveAlongPath(path);
}

bool VirtualPrinter::executeRelativeNPath(NPath path) {
    qDebug()<<"Moving relative";
    path.setOrigin(laststate_);
    return executeNPath(path);
}

State VirtualPrinter::currentState() {

    State cp(statesize_,0.0);
    QMapIterator<int,int> i(idtostatemap_);
    while(i.hasNext()) {
        i.next();
        double p = eInterface.getMotor(i.key())->getPosition();
        cp[i.value()]=p;
    }
    return cp;
}


bool VirtualPrinter::forceStop(){
    eInterface.forceStop();
    initialized_ = false;
    return true;
}

void VirtualPrinter::stop(){
    QList<Motor*> ms = eInterface.getMotors();
    QListIterator<Motor*> motors(ms);
    while(motors.hasNext()){
        Motor* m = motors.next();
        m->stop();
    }
//    eInterface.getCoordinatedMotion()->stop();
}

void VirtualPrinter::resetPosition(){
    eInterface.resetPosition();
    laststate_= State(statesize_,0.0);

}

///////////////////////////////////////////////////////

TestPrinter::TestPrinter():VMPrototype() {
    statesize_=5;
    totalprintcommands_.append(NPath(statesize_,false));
}

void TestPrinter::loadConfig(QDomDocument document) {
//    QDomElement root = document.documentElement();
//    QDomNode electronics = root.namedItem("electronics");

    // THIS SHOULD BE BASED ON THE NODES
    frequency_ = 30;
    idtostatemap_.clear();
    idtostatemap_[0]=1;
    idtostatemap_[1]=2;
    idtostatemap_[2]=3;
    idtostatemap_[3]=4;
    statesize_ = 5;
//    totalprintcommands_ = NPath(statesize_,false);
    VMPrototype::loadConfig(document);
	
}

bool TestPrinter::executeNPath(NPath path) {
    path.toAbsolute();
//    for(int i=0;i<path.numberOfStates();i++) {
//        totalprintcommands_.addState(path.getState(i));
//    }
    totalprintcommands_.append(path);
    laststate_ = path.lastAbsolute();
    return true;
}

bool TestPrinter::executeRelativeNPath(NPath path) {
    path.setOrigin(laststate_);
    return executeNPath(path);
}

void TestPrinter::dumpstates() {
    qDebug("dumping");
    for(int i=0;i<totalprintcommands_.size();i++) {
        QString s("");
        QTextStream ss(&s,QFile::WriteOnly);
        ss<<"Total"<<i;
        test(s,totalprintcommands_.at(i));
    }
}

State TestPrinter::currentState() {
    return State(totalprintcommands_.last().lastAbsolute());
}

void TestPrinter::resetPosition(){
    laststate_ = State(statesize_,0.0);
    NPath n(statesize_,false);
//    n.addState(laststate_);
    totalprintcommands_.append(n);
}

void TestPrinter::waitMove(){return;}


QString runScript(VirtualPrinter *vm, QString script_) {
    QString error="";
    QTextStream ss(&error,QIODevice::ReadOnly);

    QScriptEngine* engine = vm->makeEngine();

    // Add motors to engine environment
    QString name="";
    QTextStream ns(&name,QIODevice::WriteOnly);
    QScriptValue motorArray = engine->newArray(vm->eInterface.getMotors().size());
    engine->globalObject().setProperty("motors",motorArray);
    foreach(Motor* m, vm->eInterface.getMotors()) {
        name.clear();
        QScriptValue scriptmotor = engine->newQObject(m);
        ns << "" << m->getID();
        motorArray.setProperty(name,scriptmotor);
    }

    // Add coordinated motion object to engine
    JsCmotion jscm_;
    jscm_.setCmotion(vm->eInterface.getCoordinatedMotion());
    QScriptValue scriptCMotion = engine->newQObject(&jscm_);
    engine->globalObject().setProperty("cmotion",scriptCMotion);

    if (engine->checkSyntax(script_).state()==QScriptSyntaxCheckResult::Error) {
       ss<<engine->checkSyntax(script_).errorMessage();
    }

    engine->evaluate(script_);

    if (engine->hasUncaughtException()) {
       ss<<engine->uncaughtException().toString();
    }
    return error;
}
