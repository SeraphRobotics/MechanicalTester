#include "loadconfigthread.h"
#include <QCoreApplication>
#include <QApplication>
#include <stdio.h>
LoadConfigThread::LoadConfigThread():ready_(false)
{
}


LoadConfigThread::LoadConfigThread(VMPrototype* vm,QDomDocument config):ready_(false){
    setVM(vm);
    setConfig(config);
}
void LoadConfigThread::setVM(VMPrototype* vm){
    vm_ = vm;
    vmThread_ = vm->thread();
    checkReady();
}

void LoadConfigThread::setConfig(QDomDocument config){
    config_ = config;
    checkReady();
}

void LoadConfigThread::checkReady(){
    if (config_.isNull()){//!vm_->isInitialized() &&
        ready_ = true;
        emit ready();
    }

}

void LoadConfigThread::run(){
    checkReady();
    if (ready_){
        vm_->loadConfig(config_);
        vm_->moveToThread(QApplication::instance()->thread());
        if(vm_->isInitialized()){
            emit loaded();
        }

    }else{
        vm_->moveToThread(QApplication::instance()->thread());
    }
}
