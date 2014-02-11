#ifndef MATERIAL_H
#define MATERIAL_H

#include <QString>
#include <QMap>
#include <QDomNode>
#include <QtScript>

/**
 * A struct for defining a material
 */
struct Material{

    Material(){
        id =0;
        name = "";
        Q=1;
    }

    int id;
    QString name;
    double Q;// Flow rate in mm^3/min
    QMap<QString,QString> property;
};

Q_DECLARE_METATYPE(Material)

/**
 * Constructs a material form a DomNode of an XDFL file
 */
Material materialFromDom(QDomNode node);

/**
 * Constructs a Javascript object form a material
 */
QScriptValue objFromMat(QScriptEngine *engine, const Material &mat);


/**
 * Constructs a material form a Javascipt object
 */
void matFromObj(const QScriptValue &obj, Material &mat);

#endif // MATERIAL_H
