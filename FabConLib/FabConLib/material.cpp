#include "material.h"

Material materialFromDom(QDomNode node) {
    Material mat;
    QString name ="";
    int id=0;
    double areaconstant=0,pathwidth=0,pathheight=0,pathspeed=0,Q=0;
    if ("material"!=node.nodeName().toLower()) {
        mat.id=id;
        mat.name=name;
        mat.Q=Q;
        return mat;
    }
    QDomNodeList matChildren = node.childNodes();
    for (unsigned int i=0; i < matChildren.length(); i++) {
        QDomNode mchild = matChildren.at(i);
        if (mchild.isComment()) {continue;}
        //if (mchild.isCDATASection()) {continue;}
        if ("id" == mchild.nodeName().toLower()) {
            id = mchild.toElement().text().toInt();

        } else if ("name" == mchild.nodeName().toLower()) {
            name = mchild.toElement().text();

        } else if ("pathwidth" == mchild.nodeName().toLower()) {
            pathwidth = mchild.toElement().text().toDouble();
            mat.property["pathwidth"] = mchild.toElement().text();

        } else if ("pathheight" == mchild.nodeName().toLower() || "sliceheight"== mchild.nodeName().toLower()) {
            pathheight = mchild.toElement().text().toDouble();
            mat.property["pathheight"] = mchild.toElement().text();

        } else if ("pathspeed" == mchild.nodeName().toLower()) {
            pathspeed = mchild.toElement().text().toDouble();
            mat.property["pathspeed"]=mchild.toElement().text();

        } else if ("areaconstant" == mchild.nodeName().toLower()) {
            areaconstant = mchild.toElement().text().toDouble();
            mat.property["areaconstant"] = mchild.toElement().text();

        } else if ("compressionvolume" == mchild.nodeName().toLower()) {
            mat.property["compressionVolume"] = mchild.toElement().text();

        } else if ("property" == mchild.nodeName().toLower()) {
            QDomElement e= mchild.toElement();
            QString name = e.namedItem("name").toElement().text();
            QString value = e.namedItem("value").toElement().text();
            mat.property[name]=value;
        } else {
            QDomElement e= mchild.toElement();
            QString name = e.nodeName().toLower();
            QString value = e.text().toLower();
            mat.property[name]=value;
        }
    }
    Q = pathwidth*pathheight*pathspeed*areaconstant;
    mat.id=id;
    mat.name=name;
    mat.Q=Q;
    return mat;
}

QScriptValue objFromMat(QScriptEngine *engine, const Material &mat) {
    QScriptValue obj = engine->newObject();
    obj.setProperty("id",mat.id);
    obj.setProperty("name",mat.name);
    obj.setProperty("Q",mat.Q);

    QScriptValue array = engine->newArray(mat.property.size());
    QStringList keys= mat.property.keys();
    for(int i=0;i<mat.property.size();i++) {
        array.setProperty(i,keys.at(i));
        obj.setProperty(keys.at(i),mat.property[keys.at(i)].toDouble());
    }
    obj.setProperty("keys",array);
    return obj;
}
void matFromObj(const QScriptValue &obj, Material &mat) {
    if (!obj.isObject()) {return;}
    mat.id = obj.property("id").toInt32();
    mat.name = obj.property("name").toString();
    mat.Q=obj.property("Q").toNumber();
    QScriptValue v = obj.property("keys");
    if (v.isArray()) {
        for(int i=0;i<v.property("length").toInt32();i++) {
            QString key = v.property(i).toString();
            mat.property[key] = obj.property("key").toString();
        }
    }
}

