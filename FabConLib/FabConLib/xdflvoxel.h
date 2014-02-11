#ifndef XDFLVOXEL_H
#define XDFLVOXEL_H
#include <QtScript>
#include <QDomNode>

/**
 * A struct representation of an XDFLVoxel
 */
struct XDFLVoxel{
    int id;
    double volume;
    double x;
    double y;
    double z;
};
Q_DECLARE_METATYPE(XDFLVoxel)

/**
 * constructs a voxel from an XDFLDom
 */
XDFLVoxel voxFromQDom(QDomNode node);


/**
 * Creates a Javascript representation of a voxel
 */
QScriptValue objFromVoxel(QScriptEngine *engine, const XDFLVoxel &voxel);

/**
 * Creates a voxel from a javascript object
 */
void voxelFromObj(const QScriptValue &obj, XDFLVoxel &voxel);

#endif // XDFLVOXEL_H
