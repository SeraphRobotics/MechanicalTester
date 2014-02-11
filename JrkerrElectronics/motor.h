#ifndef MOTOR_H
#define MOTOR_H

#include <QObject>

//PIC-SERVO path header files

#include "picservo.h"
#include "nmccom.h"





class Motor : public QObject
{
    Q_OBJECT
public:


    // These classes are for the interface only
    Motor();

    /**
     *A constructor for a JRKErr servo motor which uses explicit value declaration
     */
    Motor(int id,const byte& address,
          const double countsPerRev,
          const double ticksPerSecond,
          short kp, short kd, short ki, short il, byte ol, byte cl, short int el, byte sr, byte db,
          double minComPos, double maxComPos, double minComVel, double maxComVel, double minComAcc, double maxComAcc);
    /**
     *A constructor for a JRKErr servo motor which uses explicit value declaration
     */
    Motor(int id,const byte& address,
          const double countsPerRev,
          const double ticksPerSecond,
          double newVelocity, double newAcceleration,
          short kp, short kd, short ki, short il, byte ol, byte cl, short int el, byte sr, byte db,
          double minComPos, double maxComPos, double minComVel, double maxComVel, double minComAcc, double maxComAcc);



public:
    /**
      Sets the address of the motor on the controller board, see JKerr documentation for more details
      @param groupAddress The group address
      @param isLeader A boolean that indicates whether this motor is the leader of the motors under groupAddress
      @return true iff the command was sent successfully
     */
    bool setGroup(byte groupAddress, bool isLeader);

    /**
      gets the motor address from the controller board
      @return the address of the motor
     */
    byte getAddress();

    /**
      @return the ratio of counts of motor movement per real unit of measurement
     */
    double getCountsPerRev();

    /**
      Perform cleanup when done using motor.
      @return True iff the command was sent to the motor successfully
     */
    bool cleanUp();


    bool resetPosition();

    /**
      Initialize the motor.
      @return "" if successful or an error message if failed
     */
    QString initialize();

    /**
      Set whether this motor is reversed.
      If a motor is reversed, movement commands will cause the motor to move in the opposite direction.
      @param A bool that reverses the direction of the motor if true
     */
    void setReversed(bool);

public slots: // these are slots for when the item is added to javascipt interface.

    /**
     * returns the id of the motor
     */
    int getID();

    /**
     * returns the current position of the motor
     */
    double getPosition();
    /**
      Sets the velocity to given number
      @param The new velocity
     */
    void setVelocity(double v);

    /**
      Sets the acceleration to given number
      @param The new acceleration
     */
    void setAcceleration(double a);

    /**
      @return The velocity of the given motor
     */
    double getVelocity();

    /**
      @return The acceleration of the given motor
     */
    double getAcceleration();


    /**
      Gets the minimum position for input
      @return the smallest position the motor can be moved to
     */
    double getMinPosition();

    /**
      Gets the maximum position for input
      @return the largest position the motor can be moved to
     */
    double getMaxPosition();

    /**
      @return The most recent position that the motor was commanded to move to by moveAbsolute,
      moveRelative, and resetPosition functions. The unit of position is U.
     */
    double getCommandedPosition();

    /**
      @return the smallest velocity the motor can have;
     */
    double getMinVelocity();

    /**
      @return the largest velocity the motor can have;
     */
    double getMaxVelocity();

    /**
      @return the smallest velocity the motor can have;
     */
    double getMinAcceleration();

    /**
      @return the largest velocity the motor can have;
     */
    double getMaxAcceleration();

    /**
      Move to a position at a specified velocity and acceleration.
      The unit of position is Revolutions, the unit of velocity is rev/second, and the unit of acceleration is rev/(second^2).
      Requires MIN_COMMANDED_POSITION <= position <= MAX_COMMANDED_POSITION,
      MIN_COMMANDED_VELOCITY <= velocity <= MAX_COMMANDED_VELOCITY,
      MIN_COMMANDED_ACCELERATION <= acceleration <= MAX_COMMANDED_ACCELERATION
      @return true iff command was issued successfully.
     */
    bool moveAbsolute(double position, double velocity, double acceleration);

    /**
      Move relative to the current position at a specified velocity and acceleration.
      @return true iff command was issued successfully
     */
    bool moveRelative(double positionDelta, double velocity, double acceleration);
    /**
      This function returns as soon as possible after the motor has finished moving.
     */
    void waitMove();

    /**
      Checks to see if the motor whose address is contained by ADDRESS is moving
      @return true iff motor is currently moving.
     */
    bool moving();


    /**
      Stops the motor
      @return true iff the motor command to stop was sent in successfully
     */
    bool foreceStop();

    void stop();

    QString getErrors();



private:

    void updateData(void);//Tell hardware to send updated data for this motor to the DLL.

    double commandedPosition_; //desired position of the motor
    double velocity_, acceleration_; //The velocity and acceleration of the motor
    bool initialized_; //true is motor is initialized and ready for us, false otherwise
    bool isReversed_; //true if motor direction is to be reversed from default values on controller board

    QString error_;

    int id_;
    byte ADDRESS; //The number of the motor port that this motor is plugged in.
    double COUNTS_PER_REV; //Number of counts for 1 U of motion.
    double TICKS_PER_SECOND; //Number of ticks that a motor makes per second.

    double MIN_COMMANDED_POSITION; //The minimum position that the motor can go to
    double MAX_COMMANDED_POSITION; //The maxiumum position that the motor can go to
    double MIN_COMMANDED_VELOCITY; //The minumum velocity the motor can travel at
    double MAX_COMMANDED_VELOCITY; //The maximum velocity the motor can travel at
    double MIN_COMMANDED_ACCELERATION; //The minimum acceleration of the motor
    double MAX_COMMANDED_ACCELERATION; //The maxiumum acceleration of the motor

    short KP, KD, KI, IL, EL; //Damping contraints (look up procedure for critically damping motors for more details, also JKerr has documentation)
    byte OL, CL, SR, DB; //See JKerr's Document for more details on his motor bytes


};

#endif // MOTOR_H
