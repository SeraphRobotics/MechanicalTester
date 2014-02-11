#ifndef JSXYZ_H
#define JSXYZ_H

#include <QObject>
#include <QtScript>
#include "xyzmotion.h"

class JsXYZ : public QObject
{
Q_OBJECT
public:
    /**
     * Default Constructor for the javascript binding of the xyzmotion class
     */
    JsXYZ();

    /**
     * sets the xyz motion instance of the object
     */
    void setXYZ(XYZMotion* xyz);

public slots:

    /**
     * JavaScript interface for the xyzmotion.pathTo function
     */
    QScriptValue pathTo(QScriptValue x,QScriptValue y,QScriptValue z, QScriptValue speed);

    /**
     * Javascript interface for the xyzmotion.pathAlong
     */
    QScriptValue pathAlong(QScriptValue JSPath,QScriptValue JSspeed);

private:
    XYZMotion* xyz_;
};

#endif // JSXYZ_H
