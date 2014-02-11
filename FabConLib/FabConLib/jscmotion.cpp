#include "jscmotion.h"


JsCmotion::JsCmotion():hasCM(false),cmotion_() {}

void JsCmotion::setCmotion(CoordinatedMotion* cmotion){
    cmotion_ = cmotion;
}


int JsCmotion::getNumberOfAxes(){
    return cmotion_->getNumberOfAxes();
}

void JsCmotion::moveAlongPath(QScriptValue points){

    NPath npath(cmotion_->getNumberOfAxes()+1,true);
    npathFromMatrix(points,npath);
    cmotion_->moveAlongPath(npath);
}
