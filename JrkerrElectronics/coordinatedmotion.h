#ifndef COORDINATEDMOTION_H
#define COORDINATEDMOTION_H

#include <QObject>
#include "motor.h"
#include "npath.h"
#include "nmotion.h"
#include <QMap>



class CoordinatedMotion : public QObject
{
    Q_OBJECT
public:
    /**
     * Default constructor for the coordinated motion class
     */
    CoordinatedMotion();

    /**
     * Overloaded constructor for the coordinated motion class
     * it preloads the motors durring construction
     */
    CoordinatedMotion(QMap<int,Motor*> motors);

    /**
     * Sets the motors for the coordinated motion class, used for the default constructor
     */
    void setMotors(QMap<int,Motor*> motors);

    /**
     * Executes the entire NPath sequece in Absolute coordinates
     */
    bool moveAlongPath(NPath points);


    /**
     * Executes the NPath in Absolute coordinates starting with the indicated state index
     */
    bool moveAlongPath(NPath points, int startPointIndex);


    /**
     * sets the Group Address of the system.
     */
    void setGroupAddress(byte address); // this should not need to be visiblee

    /**
     * set the update rate of the system
     */
    void setFrequency(int frequency);

    /**
     * sets the size of the buffer on the electroncs in terms of states.
     * these states are executed at the rate of the frequency
     */
    void setBufferSize(int buffsize);

    /**
     * returns the update frequency of the system
     */
    int getFrequency();

    /**
     * makes the system ready for coorintated motion
     */
    bool initializePathMode();


    /**
     * Resets the position of the system;
     */
    bool resetPosition();

    /**
     * @returns true if the NPath is valid for the motor configuration
     * Uses the NPath in relative coordinates
     **/
    bool validateNPath(NPath path);

    /**
     * returns the mapping between the motor id and the state index number
     * This is needed since not all motors need be in the coordinate motion
     */
    QMap<int,int> getIdToStateIndexMap();

public slots:// THis is uneeded

    /**
     * returns the number of motors which are being coordinated
     */
    int getNumberOfAxes();

    /**
     * returns the string of errors which may have occured while being used.
     */
    QString getErrors();

    void stop();


private:
    QMap<int,Motor*> motors_; // the integer id of the state variable and the corresponding motor instance
    bool initialized_;
    byte X_Y_Z_GROUP_ADDRESS;
    int BUFF_SIZE;
    QString error_string;
    bool pathbegan_;
    QMap<int,double> scales;
    QMap<int,int> addressMap;
    int frequency_;

};

#endif // COORDINATEDMOTION_H
