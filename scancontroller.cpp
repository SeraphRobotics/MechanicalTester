#include "scancontroller.h"
#include <QDomDocument>
#include <QDebug>

ScanController::ScanController(QObject *parent):QObject(parent), axis_("x"),ready_(0),scandistance_(0),
    scanspeed_(0),scangoal_(0),scannumber_(0),position_(0),xvector_(QVector<float>(3,0)),count_(0)
{    
    timer_ = new QTimer();
    vm_ = new VirtualPrinter();
    connect(timer_,SIGNAL(timeout()),this,SLOT(ScanStep()));
    connect(this,SIGNAL(scanComplete()),timer_,SLOT(stop()));

    // Used in motion commands durring scan
    axes_["x"]=0;
    axes_["y"]=1;
    axes_["z"]=2;

}

void ScanController::loadVM(VirtualPrinter* vm)
{
    vm_ = vm;
    isReady();
}



void ScanController::setAxis(QString Axis)
{
    axis_ = Axis.toLower();
    isReady();
}


bool ScanController::isReady()
{
    QString db;
    // ready only if webcam works, vm ready, and values are set
    if( (vm_->isInitialized()==false)||(axis_=="")||(scandistance_==0)||(scanspeed_==0)||(scangoal_==0)){
        ready_=false;
        db=QString("\nNot ready: ")+QString("VM is ")+QString(vm_->isInitialized() ? "Initialized":"Not Initialized")+
                  QString(", Axis: ")+QString(axis_)+QString(", ScanDistance: ")+
                  QString::number(scandistance_)+QString(", Scan Goal: ")+
                  QString::number(scangoal_)+QString(", scanspeed: ")+QString::number(scanspeed_);
        qDebug()<<db;
        emit error(db);
    }else{
        ready_=true;
        db="ready";
        emit updateStatus(db);
    }
    return ready_;
}


void ScanController::setScan(float scandistance, float scanspeed, int scannumber)
{
    scandistance_=scandistance;
    scanspeed_=scanspeed;
    scangoal_=scannumber;
    float time = 1;//1000*scandistance/scanspeed+10;
    timer_->setInterval(time);
    isReady();
}

void ScanController::StartScan()
{
    if(ready_){
        vm_->resetPosition();

        QVector<double> pos = vm_->currentPosition();
        position_= pos[axes_[axis_]];
        emit updateStatus(QString("Start Position: ")+QString::number(position_));


        xvector_[0]=0;
        xvector_[1]=0;
        xvector_[2]=0;
        xvector_[axes_[axis_]]=scandistance_;



        vm_->move(xvector_[0],xvector_[1],xvector_[2],scanspeed_);

        timer_->start();

        emit scanRunning(true);
    }else{
        emit error(QString("not ready"));
    }
}

void ScanController::StopScan()
{

    timer_->stop();
    vm_->stop();
    emit updateStatus("Stopping and returning to 0");
    // Return to Origin
    float speed = scanspeed_;
    QVector<double> p=vm_->currentPosition();
    vm_->move(-p[0],-p[1],-p[2],speed);
    emit updateStatus("Stopped");
    vm_->waitMove();
    scannumber_=0;
    emit scanRunning(false);
}



void ScanController::ScanStep()
{
    if(!ready_){
        emit error(QString("Scanner not ready"));
        return;
    }

    QVector<double> pos = vm_->currentPosition();
    position_= pos[axes_[axis_]];
    float tolerance=1;
    float p_error=position_-scandistance_;
    qDebug()<<"position: "<<position_<<"error: "<<p_error<<"\ttarget:"<<scandistance_;


    //AT end point move to 0,0,0
    if(fabs(p_error)<tolerance ){
        vm_->move(-xvector_[0],-xvector_[1],-xvector_[2],scanspeed_);
        emit update(position_);
        emit updateStatus(QString("Position is now ")+QString::number(position_));
        vm_->waitMove();
    }

    if( (fabs(position_)<tolerance) && (scannumber_<scangoal_) ){
        scannumber_++;
        vm_->move(xvector_[0],xvector_[1],xvector_[2],scanspeed_);
        emit update(position_);
        emit updateStatus(QString("Position is now ")+QString::number(position_));
        emit updateStatus(QString("Number: ")+QString::number(scannumber_));
        vm_->waitMove();
    }

    if((scannumber_>=scangoal_)){
        QString db = QString("DONE");
        emit updateStatus(db);
        StopScan();
        emit scanComplete();
        return;
    }
}


