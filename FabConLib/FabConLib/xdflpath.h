#ifndef XDFLPATH_H
#define XDFLPATH_H

#include <QList>
#include <QtScript>
#include <QDomNode>

/**
 * the struct for a Point in 3-space
 */
struct FabPoint { // this will need to be redone to handel uvw later
    FabPoint();
    double x;
    double y;
    double z;
};

/**
 * Calculates the distances between two points
 */
double distance(FabPoint p1, FabPoint p2);

/**
 * calulated a point at a distance dist away form point 1 on the line which intersects point one and two
 */
FabPoint pointBetween(FabPoint p1, FabPoint p2, double dist);

/**
 * Returns if the two points are withing the rounding distance from each other
 */
bool pointsEqual(FabPoint p1, FabPoint p2, double rounding = 0.01);

/**
 * returns a point from a QVector. the QVector must be of size 3
 */
FabPoint pointFromQVector(QVector<double> v);

/**
 * A container class for the XDFL paths.
 */
class XDFLPath
{
public:

    /**
     * default constructor of an XDFL Path.
     */
    //There are three states, absolute, local relative (euclidian) and global relative. toRelative makes it local relative.
    explicit XDFLPath(int material_id =0, bool relative = false);

    /**
     * Copy contstructor of XDFLpaths
     */
    XDFLPath(const XDFLPath &path);

    /**
     * calculates the length of the path
     */
    double length();

    /**
     * returns the beginning point of the path
     */
    FabPoint start();

    /**
     * returns the endpoint of the path
     */
    FabPoint end();

    /**
     * Converts the path to coordinates relative to each other
     */
    void toRelative();

    /**
     * Converts the path to absolute coordinates
     */
    void toAbsolute();

    /**
     * returns if the path contains points
     */
    bool isNull();

    /**
     * returns the points (relative or absolute based on setting)
     */
    QList<FabPoint> getPoints();

    /**
     * returns the points in a coordinate system with 0,0,0 being the position of the first point
     */
    QList<FabPoint> getGlobalRelativePoints();

public:
    QList<FabPoint> points;
    double speed;
    int materialID;
    bool relative_;

};

/**
 * Contrsucts an XDFLPath from a QDomNode from an XDFL file
 */
XDFLPath pathFromQDom(QDomNode node);

Q_DECLARE_METATYPE(XDFLPath)

/**
 * Creates a Javascript object from an XDFLPath
 */
QScriptValue objFromPath(QScriptEngine *engine, const XDFLPath &path);

/**
 * Creates a path from a javascipt object
 */
void pathFromObj(const QScriptValue &obj, XDFLPath &path);


/**
 * returns a point which is the diffrence between the two points
 */
FabPoint subtractpoints(FabPoint p1, FabPoint p2);

/**
 * returns a point which si the addition of two points
 */
FabPoint addpoints(FabPoint p1, FabPoint p2);


#endif // XDFLPATH_H
