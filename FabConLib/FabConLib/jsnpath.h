#ifndef JSNPATH_H
#define JSNPATH_H

#include <QtScript>
#include "npath.h"




//QScriptValue NPath_ctor(QScriptContext *context,QScriptEngine *engine);


/**
 * Creates a relative npath from a javascript array
 */
void npathFromMatrix(const QScriptValue &obj,NPath &npath);

/**
 * Creates a javascript array from an npath
 */
QScriptValue matrixFromNPath(QScriptEngine *engine,NPath &npath);

/**
 * Creates a javascript array from an npath and assigns a distance property to it.
 */
QScriptValue matrixFromNPath(QScriptEngine *engine,NPath &npath, QVector<double> distances);

Q_DECLARE_METATYPE(NPath)


/**
 * Javascript constructor for a state
 */
QScriptValue newJsState(QScriptContext *context, QScriptEngine *engine);


/**
 * Javascript function which is added to the javascript npath to police which states are added
 */
QScriptValue addState(QScriptContext * context,QScriptEngine *engine);

/**
 * Javascript constructor for an NPath
 */
QScriptValue newJsNPath(QScriptContext *context, QScriptEngine *engine);

#endif // JSNPATH_H
