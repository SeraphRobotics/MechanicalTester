#ifndef BAY_H
#define BAY_H

#include <QObject>
#include "motor.h"
//#include "materialcalibration.h"
#include <QDomNode>
#include "xdflpath.h"
#include "xdflvoxel.h"
#include "material.h"
#include <QScriptEngine>
#include "jsnpath.h"

class Bay : public QObject
{
    Q_OBJECT
public:

    /**
     * default constructor for bays
     */
    Bay();

    /**
     * Builds the bays based on a config file node
     */
    Bay(const QDomNode& sourceDomNode);

    /**
     * sets the mapping between the motor id and the state index
     */
    void setIdMap(QMap<int,int> map);

    /**
     * sets the size of a state to be used in the npaths returned by the bay
     */
    void setStateSize(int statesize);

    /**
     * set the bay's id
     */
    void setId(int id);

    /**
     * returns the ID of the bay
     */
    int getId();

    /**
     * set the maximum reservoir volume of the bay
     */
    void setMaxVolume(double vol);

    /**
     * returns the maximum reservoir volume (in mm^3) of the bay
     */
    double getMaxVolume();

    /**
     * sets the location of the bay interface location
     * relative to the absolute workspace refrence frame
     */
    void setLocation(QVector<double> location);

    /**
     * returns the location of the bay interface
     * relative to the absolute workspace refrence frame
     */
    QVector<double> getLocation();

    /**
     * sets the engine of the bay and prepares its envrioment
     * for the onPath and onVoxel functions.
     */
    void setEngine(QScriptEngine* engine);


    /**
     * returns the bay's errors as a string
     */
    QString getErrors();

public slots:
    /**
     * resets the amount of volume extruded to zero
     */
    void clearVolume();

    //Materials
    /**
     * returns true if a valid material is loaded.
     */
    bool hasMaterial()const ;

    /**
     * returns the material of the bay. It could be null,
     * check if the bay has a material using hasMaterial() first.
     */
    const Material getMaterial() const;

    /**
     * set the material of the bay
     */
    void setMaterial(Material material);

    //Actuation
    /**
     * This returns a relative NPath from a relative path.
     * It uses a script function from the config file
     */
    NPath onPath(XDFLPath path);

    /**
     * This returns a relative NPath from a relative voxel.
     * It uses a script function from the config file
     */
    NPath onVoxel(XDFLVoxel voxel);

    /**
     * moves the bays actautors by a given rotation, in a given time
     * this is a hack to work with the current system and should be improved
     */
    NPath jogActuators(double amount,double time);

private:
    QScriptEngine* engine_;

    double  max_volume_;

    double volume_;

    int id_;

    QVector<double> location_; //The bay systems point of contact
    Material material_;
    QMap<QString,QString> scriptSettings_;
    QList<int> actuatorIDs_;
    QMap<int,int> idToStateIndex_;
    QString script_;
    QString error_;
    int statesize_;
};

#endif // BAY_H
