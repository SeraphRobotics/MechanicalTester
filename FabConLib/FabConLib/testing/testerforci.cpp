#include "testerforci.h"
#include <QDebug>

testerforci::testerforci():connected_(false){
    ci = new CoreInterface();
    makeConnections();

}

testerforci::testerforci(CoreInterface* coreinterface):connected_(false){
    ci = coreinterface;
    makeConnections();
}


void testerforci::makeConnections(){
    connect(ci,SIGNAL(currentPosition(double,double,double)),this,SLOT(updatedPosition(double,double,double)));
    connect(ci,SIGNAL(moving()),this,SLOT(moving()));
    connect(ci,SIGNAL(stateChaged(int)),this,SLOT(chagedState(int)));
    connect(ci,SIGNAL(estimated(double,double,int)),this,SLOT(estimated(double,double,int)));
    connect(ci,SIGNAL(materialsAvailable(QMap<int,Material>)),this,SLOT(materialsAvailable(QMap<int,Material>)));
    connect(ci,SIGNAL(bayMaterial(int,int)),this,SLOT(bayMaterial(int,int)));
    connect(ci,SIGNAL(currentCommand(int)),this,SLOT(currentCommand(int)));
    connect(ci,SIGNAL(needMaterialLoaded(int)),this,SLOT(needMaterialLoaded(int)));

    connect(ci,SIGNAL(printPaused()),this,SLOT(onPaused()));
}


void testerforci::setConfig(){
    qDebug("attemping to make a connection...");
    QString configFilePath="../../../media/JrKerr-Single-deposition.config";
    QDomDocument document;
    {
      QFile configFile(configFilePath);
      if (!configFile.open(QFile::ReadOnly)) {
          qDebug("FAILED TO OPEN CONFIG FILE");
          return;
      }
      document.setContent(&configFile);
      configFile.close();
    }
    ci->setConfig(document.toString(),"COM6");
}

void testerforci::chagedState(int state){
    if ((state== CoreInterface::Connected)&&!connected_){
        qDebug("state is connected");
        connected_=true;
        ci->move(0,5,-20,50);
//        disconnect(ci,SIGNAL(currentPosition(double,double,double)),this,SLOT(updatedPosition(double,double,double)));

        ci->resetPosition();
        qDebug()<<"STATE is now"<<ci->getCurrentPosition();
        ci->resetPosition();
        qDebug()<<"STATE is now"<<ci->getCurrentPosition();
        loadXDFL();

    }else if (state == CoreInterface::FileLoaded){
        qDebug("file loaded");
        int materialid =1;
        int bayid = 0;
        ci->setMaterial(bayid,materialid);

        ci->resetPosition();
        qDebug()<<"Position before print is "<<ci->getCurrentPosition();
        ci->startPrint();
    }else if(state ==CoreInterface::Printing){

        qDebug("PRINTING");
//        QTimer::singleShot(3000,ci,SLOT(pausePrint()));
//        QTimer::singleShot(10000,this,SLOT(onPaused()));
//        QTimer::singleShot(15000,ci,SLOT(resumePrint()));
//        QTimer::singleShot(9000,ci,SLOT(cancelPrint()));
    }
}

void testerforci::onPaused(){
    qDebug()<<"ON PAUSE CALLED";
    ci->setMaterial(0,2);
    QTimer::singleShot(10,ci,SLOT(resumePrint()));
}


void testerforci::updatedPosition(double x, double y, double z){
//    qDebug("New Position is %f,%f,%f",x,y,z);
}
void testerforci::moving(){
    qDebug("moving");
}

void testerforci::loadXDFL(){
    QDomDocument xdflDom;
    QString xdflFilePath="../../../media/testxdfl-mid.xdfl";
    // load the XDFL file into the DOM document
    {
      QFile xdflFile(xdflFilePath);
      if (!xdflFile.open(QFile::ReadOnly)) {
          qDebug("FAILED TO OPEN XDFL FILE");
          return;
      }
      xdflDom.setContent(&xdflFile);
      xdflFile.close();
    }
    ci->setXDFL(xdflDom.toString());


}

void testerforci::estimated(double t, double v, int cmds){
    qDebug("Time = %f\tVolume = %f",t,v);
}

void testerforci::materialsAvailable(QMap<int,Material> ms){
    materials_ = ms;
    qDebug("The following materials are avalible");
    QMapIterator<int,Material> i(ms);
    while(i.hasNext()){
        i.next();
        qDebug("\t%s, id:%i",i.value().name.toStdString().c_str(),i.key());
    }
}

void testerforci::bayMaterial(int bayid, int materialid){
    qDebug("bay %i is equiped with material %s with id %i",bayid,
           materials_[materialid].name.toStdString().c_str(),materialid);
}

void testerforci::currentCommand(int cmd){
    qDebug("Current command is %i",cmd);
}

void testerforci::needMaterialLoaded(int i){
    qDebug("Need Material id:%i %s",i,materials_[i].name.toStdString().c_str());
    nap(2000);
    ci->setMaterial(0,1);
    ci->resumePrint();
}
