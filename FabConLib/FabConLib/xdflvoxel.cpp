#include "xdflvoxel.h"

QScriptValue objFromVoxel(QScriptEngine *engine,const XDFLVoxel &voxel) {
    QScriptValue obj = engine->newObject();
    obj.setProperty("id",voxel.id);
    obj.setProperty("volume",voxel.volume);
    obj.setProperty("x",voxel.x);
    obj.setProperty("y",voxel.y);
    obj.setProperty("z",voxel.z);
    return obj;
}

void voxelFromObj(const QScriptValue &obj,XDFLVoxel &voxel) {
    if (!obj.isObject()) {return;}
    voxel.id = obj.property("id").toInt32();
    voxel.volume = obj.property("volume").toNumber();
    voxel.x = obj.property("x").toNumber();
    voxel.y = obj.property("y").toNumber();
    voxel.z = obj.property("z").toNumber();
}

XDFLVoxel voxFromQDom(QDomNode node) {
    XDFLVoxel v;
    QDomNodeList voxChildren = node.childNodes();
    QDomElement vchild;
    for (int i=0; i < voxChildren.size(); i++) {
        vchild = voxChildren.at(i).toElement();
        if (vchild.isComment()) {continue;}
        if ("materialid"==vchild.nodeName().toLower()) {
            v.id = vchild.text().toInt();
        } else if ("volume"==vchild.nodeName().toLower()) {
            v.volume = vchild.text().toDouble();
        } else if ("x"==vchild.nodeName().toLower()) {
            v.x = vchild.text().toDouble();
        } else if ("y"==vchild.nodeName().toLower()) {
            v.y = vchild.text().toDouble();
        } else if ("z"==vchild.nodeName().toLower()) {
            v.z = vchild.text().toDouble();
        }
    }
    return v;
}
