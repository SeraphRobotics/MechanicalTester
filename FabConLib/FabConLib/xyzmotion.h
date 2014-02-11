#ifndef XYZMOTION_H
#define XYZMOTION_H

#include <QObject>
#include <QMap>
#include "motor.h"
#include "npath.h"
#include "nmotion.h"
#include "xdflpath.h"
#include <QDomNode>

/**
 * A struct for representing the properties of a machine axis
 */
struct axis{
    QString name;
    double range;
    double revPerDist; //
    int actuatorID;
};


/**
 * A Struct for the XYZmotion to use that is used for path planning.
 * EAch axis has Name(QString,axis), index(int,idToSTateIndex_/axis,
 * scale(double axis), norm (double, axes, x/y/z).
 **/
struct DOF{
    QString name;
    int index;
    double scale;
    double norm;
    double range;
};


class XYZMotion: public QObject
{
    Q_OBJECT
public:
    /**
     * Default constructor of the XYZMotion object.
     * this object proforms the calulations needed for coordinated motion of 3 axes
     */
    XYZMotion();

    /**
     * Constructor of the XYZMotion object from a config file DomNode
     */
    XYZMotion(const QDomNode& sourceDomNode);

    //backup contructor settings

    /**
     * sets the axes of the system, used with the default constructor
     */
    void setAxes(QMap<QString,axis> axismap);

    /**
     * Sets the acceleration used in calculations
     */
    void setAcceleration(double a);

    /**
     * returns the acceleration used in calculations
     */
    double getAcceleration()const;

    //for vm

    /**
     * sets the state change frequency. This is used to calculate the states for accelerations
     */
    void setFrequency(int frequency);

    /**
     * sets the mapping between the id of motors and their state index
     */
    void setIdMap(QMap<int,int> map);

    // general usage

    /**
     * Calculates a relative path to a point(x,y,z) from the origin
     * at the given speed using the preset acceleration
     */
    NPath pathTo(double x, double y, double z, double speed);

    /**
     * Caclulated the relative states for movment along a XDFLPath's relative positions
     * It accelerates ad the preset rate upto the speed and then maintains the speed till it
     * decelerates at the end of the path.
     */
    NPath pathAlong(XDFLPath path,double speed);


    /**
     * Calculates the relative state of a relative position x,y,z
     */
    State positionToState(double x,double y,double z);

    /**
     * calculates the absolute position from the state
     */
    QVector<double> positionFromState(State* state);

    /**
     * Calculates the distance in 3 space this state would cover in time State[0]
     */
    double distanceFromState(State* state);


    /**
     * returns the build space of the system
     */
    QVector<double> buildSpace();

    QList<DOF> getDOFs();

    QMap<QString,axis> getAxes();

private:
    bool defineDOFs();
    bool defineDOFs(double x,double y,double z);

private:
    void accelerateAlong(NPath* np, QList<FabPoint> accel_list, double dist_a, bool deccel);
    double acceleration_;
    QMap<QString,axis> axismap_;
    QList<DOF> DOFs_;
    QMap<int,int> idToStateIndex_;
    int statesize_;
    int frequency_;

};

#endif // XYZMOTION_H
