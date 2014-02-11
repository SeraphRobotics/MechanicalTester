#ifndef ELECTRONICSINTERFACE_H
#define ELECTRONICSINTERFACE_H

#include <QObject>
#include <QDomNode>

#include "nmccom.h"
#include "motor.h"
#include "nmotion.h"
#include "coordinatedmotion.h"


class ElectronicsInterface : public QObject
{
    Q_OBJECT

public:

    /**
     * This class is what allows other classes to interface with the Fab@Home hardware.
     * it contructs the motors and CoordinatedMotion classes in its constructor.
     */
    ElectronicsInterface();

    /**
     * Builds the motors and coordinateMotion objects using the QDomNode information
     */
    QString initialize(QDomNode node, QString portLocation);

    /**
     * returns true if the system is initialized
     */
    bool isInitialized();

    /**
     * resets the electronics
     */
    void reset();

    /**
     * resets position of all of the motors
     */
    void resetPosition();

    /**
     * returns a pointer to the coordinatedmotion object
     * use isInitialized to ensure the object is built.
     */
    CoordinatedMotion* getCoordinatedMotion();

    /**
     * returns a list of the points to the motors.
     * use isInitialized to ensure the objects have been built
     */
    QList<Motor*> getMotors();

    /**
     * returns the motor wiht the id specified.
     */
    Motor* getMotor(int id);

    /**
     * waits the program until all motors finish moving.
     */
    void waitOnMove();

    /**
     * returns the error strings of the motors and CoordinatedMotion classes
     */
    QString getErrors();


    /**
     * emergency stops all motors, causes the system to no longer be initialized
     */
    void forceStop();

private:

    /**
     * creates the einterface coordinated motion class
     */
    void createCoodinatedMotion();

    /**
     * This should be moved to the motor class.
     */
    void createMotor(QDomNode node);

    // this should be a contstructor
    QString loadSettings(QDomNode node);

    /**
     * Cleans up the classes spawned by the ElectronicsInterface when its destroyed;
     */
    bool cleanUp();

public:
    QList<Motor*> motors_;
private:
    CoordinatedMotion cmotion_;
    bool initialized_;

    unsigned int NUM_MODULES, BAUD_RATE;
    byte X_Y_Z_GROUP_ADDRESS;//JRKERR SPECIFIC
    int BUFF_SIZE; //buffer size
    QString COM_PORT;
    QString error_string_;
    int frequency_;
};

#endif // ELECTRONICSINTERFACE_H
