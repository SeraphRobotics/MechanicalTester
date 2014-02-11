#include "jsxyz.h"
#include "jsnpath.h"
#include "xdflpath.h"
#include "testing/util.cpp"

JsXYZ::JsXYZ() {}

//QScriptValue JsXYZ::pathTo(QScriptContext * context,QScriptEngine *engine) {
//    if (4!=context->argumentCount()) {return engine->newArray(0);}
//    double x,y,z,speed;
//    x = context->argument(0).toNumber();
//    y = context->argument(1).toNumber();
//    z =context->argument(2).toNumber();
//    speed = context->argument(3).toNumber();
//    NPath n = xyz_->pathTo(x,y,z,speed);
//    return matrixFromNPath(engine,n);
//}

QScriptValue JsXYZ::pathTo(QScriptValue xs,QScriptValue ys,QScriptValue zs, QScriptValue speeds) {
    double x,y,z,speed;

    x = xs.toNumber();
    y = ys.toNumber();
    z =zs.toNumber();
    speed = speeds.toNumber();
    NPath n = xyz_->pathTo(x,y,z,speed);
    QVector<double> distances(n.numberOfStates(),0.0);
    for (int i=0; i<n.numberOfStates();i++) {
        QVector<double> s(n.getState(i));
        distances[i] = xyz_->distanceFromState(&s);//(&n.getState(i));
    }
    QScriptValue jsnpath = matrixFromNPath(xs.engine(),n,distances);
    return jsnpath;
}

//QScriptValue JsXYZ::pathAlong(QScriptContext * context,QScriptEngine *engine) {}
QScriptValue JsXYZ::pathAlong(QScriptValue JSPath,QScriptValue JSspeed) {
    double speed = JSspeed.toNumber();
    XDFLPath xpath;
    pathFromObj(JSPath,xpath);

    NPath n = xyz_->pathAlong(xpath,speed);
    QVector<double> distances(n.numberOfStates(),0.0);
    for (int i=0; i < n.numberOfStates(); i++) {
        QVector<double> s(n.getState(i));
        distances[i] = xyz_->distanceFromState(&s);//(&n.getState(i));
    }
    QScriptValue jsnpath = matrixFromNPath(JSPath.engine(),n,distances);
    return jsnpath;
}

void JsXYZ::setXYZ(XYZMotion* xyz) {
    xyz_ = xyz;
}
