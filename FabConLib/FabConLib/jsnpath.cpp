#include "jsnpath.h"
#include <stdio.h>


void npathFromMatrix(const QScriptValue &obj,NPath &npath) {
    int statesize = obj.property("statesize").toInt32();
    npath.setStateSize(statesize);
    int len = obj.property("length").toInt32();
    for(int i=0;i<len;i++) {
        QScriptValue scriptvector = obj.property(i);
        QVector<double> v(statesize);
        for (int j=0;j<statesize;j++) {
            v[j] = scriptvector.property(j).toNumber();
        }
        npath.addState(QVector<double>(v));
    }
}

QScriptValue matrixFromNPath(QScriptEngine *engine,NPath &npath) {
    QScriptValue obj = engine->newArray(npath.numberOfStates());
    for(int i=0;i<npath.numberOfStates();i++) {
        QVector<double> v = npath.getState(i);
        QScriptValue scriptVector = engine->newArray(npath.stateSize());
        for(int j=0;j<npath.stateSize();j++) {
            scriptVector.setProperty(j,v.at(j));
        }
        scriptVector.setProperty("distance",0);
        obj.setProperty(i,scriptVector);
        obj.setProperty("statesize",npath.stateSize());
        obj.setProperty("addState",engine->newFunction(addState));
    }
    return obj;
}

QScriptValue matrixFromNPath(QScriptEngine *engine,NPath &npath, QVector<double> distances) {
    QScriptValue obj = engine->newArray(npath.numberOfStates());
    for (int i = 0; i < npath.numberOfStates(); i++) {
        QVector<double> v = npath.getState(i);
        QScriptValue scriptVector = engine->newArray(npath.stateSize());
        for (int j = 0; j < npath.stateSize(); j++) {
            scriptVector.setProperty(j,v.at(j));
        }
        scriptVector.setProperty("distance",distances.at(i));
        obj.setProperty(i,scriptVector);
        obj.setProperty("statesize",npath.stateSize());
        obj.setProperty("addState",engine->newFunction(addState));
    }
    return obj;
}


QScriptValue newJsState(QScriptContext *context, QScriptEngine *engine) {
    int i = context->engine()->globalObject().property("statesize").toInt32();
    if (context->isCalledAsConstructor()) {
        // initialize the new object
        context->thisObject().setProperty("distance", 0);
        context->thisObject().setProperty("length", i);
        for (int j = 0; j < i; j++) {
            context->thisObject().setProperty(j,0);
        }
        // ...
        // return a non-object value to indicate that the
        // thisObject() should be the result of the "new Foo()" expression
        return engine->undefinedValue();
    } else {
        // not called as "new Foo()", just "Foo()"
        // create our own object and return that one

        QScriptValue object = engine->newArray(i);
        for(int j=0;j<i;j++) {
            object.setProperty(j,0);
        }
        object.setProperty("length",i);
        object.setPrototype(context->callee().property("prototype"));
        object.setProperty("distance", 0);
        return object;
    }
}

QScriptValue addState(QScriptContext * context,QScriptEngine *engine) {
    if (0==context->argumentCount() || 2<context->argumentCount()) {return QScriptValue();}

    int len = context->thisObject().property("length").toInt32();
    if (1==context->argumentCount()) {
        QScriptValue v =context->argument(0);
        context->thisObject().setProperty(len,v);

    } else if (2==context->argumentCount()) {
        int index = context->argument(1).toInt32();
        if (index>len) {return QScriptValue();}
        for(int j=len;j>index;j--) {// shift everything down 1
            QScriptValue last = context->thisObject().property(j-1);
            context->thisObject().setProperty(j,last);
        }
        context->thisObject().setProperty(index,context->argument(0));// insert state
    }
    context->thisObject().setProperty("length",len+1);
    return QScriptValue();

}

QScriptValue newJsNPath(QScriptContext *context, QScriptEngine *engine) {
    int i = context->engine()->globalObject().property("statesize").toInt32();
    if (context->isCalledAsConstructor()) {
        // initialize the new object
        context->thisObject().setProperty("addState",engine->newFunction(addState));
        context->thisObject().setProperty("statesize",i);
        // ...
        // return a non-object value to indicate that the
        // thisObject() should be the result of the "new Foo()" expression
        return engine->undefinedValue();
    } else {
        // not called as "new Foo()", just "Foo()"
        // create our own object and return that one
        QScriptValue object = engine->newArray(0);
        object.setPrototype(context->callee().property("prototype"));
        object.setProperty("statesize",i);
        object.setProperty("addState",engine->newFunction(addState));
        return object;
    }
}
