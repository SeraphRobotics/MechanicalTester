#include "motor.h"
#include "QTextStream"
#include <math.h>
//#include <QDebug>

Motor::Motor(){}

Motor::Motor(int id,const byte& address,
             const double countsPerRev,
             const double ticksPerSecond,
             short kp, short kd, short ki, short il,
             byte ol, byte cl, short int el, byte sr, byte db,
             double minComPos, double maxComPos, double minComVel,
             double maxComVel, double minComAcc, double maxComAcc):
initialized_(false),
commandedPosition_(0.0),
isReversed_(false),
id_(id),
ADDRESS(address),
COUNTS_PER_REV(countsPerRev),
TICKS_PER_SECOND(ticksPerSecond),
KP(kp),
KD(kd),
KI(ki),
IL(il),
OL(ol),
CL(cl),
EL(el),
SR(sr),
DB(db),
MIN_COMMANDED_POSITION(minComPos/countsPerRev),// see page 23 of picsrvsc.pdf
MAX_COMMANDED_POSITION(maxComPos/countsPerRev),
MIN_COMMANDED_VELOCITY(minComVel),
MAX_COMMANDED_VELOCITY(maxComVel/countsPerRev),//see page 15 of picsrvsc.pdf
MIN_COMMANDED_ACCELERATION(minComAcc),
MAX_COMMANDED_ACCELERATION(maxComAcc/countsPerRev),
velocity_(0.0),
acceleration_(0.0)
{}


Motor::Motor(int id, const byte &address,
             double countsPerRev,
             double ticksPerSecond,
             double newVelocity, double newAcceleration,
             short kp, short kd, short ki, short il,
             byte ol, byte cl, short el, byte sr, byte db,
             double minComPos, double maxComPos, double minComVel,
             double maxComVel, double minComAcc, double maxComAcc):
initialized_(false),
commandedPosition_(0.0),
isReversed_(false),
id_(id),
ADDRESS(address),
COUNTS_PER_REV(countsPerRev),
TICKS_PER_SECOND(ticksPerSecond),
velocity_(newVelocity),
acceleration_(newAcceleration),
KP(kp),
KD(kd),
KI(ki),
IL(il),
OL(ol),
CL(cl),
EL(el),
SR(sr),
DB(db),
MIN_COMMANDED_POSITION(minComPos/countsPerRev),
MAX_COMMANDED_POSITION(maxComPos/countsPerRev),
MIN_COMMANDED_VELOCITY(minComVel),
MAX_COMMANDED_VELOCITY(maxComVel/countsPerRev),
MIN_COMMANDED_ACCELERATION(minComAcc),
MAX_COMMANDED_ACCELERATION(maxComAcc/countsPerRev)
{}



void Motor::updateData()
{
    NmcNoOp(ADDRESS,&error_);
}

void Motor::setReversed(bool isReversed)
{
    this->isReversed_ = isReversed;
}

double Motor::getPosition()
{
    if(!initialized_){return 0.0;}
    updateData();
    return (isReversed_ ? -1 : 1) * ServoGetPos(ADDRESS) / COUNTS_PER_REV; //Translate count to U.
}

double Motor::getCommandedPosition()
{
    return commandedPosition_;
}

int Motor::getID()
{
    return id_;
}

bool Motor::moveAbsolute(double position, double velocity, double acceleration)
{
//    qDebug()<<"Motor move called";
    if(!initialized_){
//        qDebug()<<"not initialized";
        return false;
    }
    //if reversed then switches position
    if(isReversed_)
    {
        position *= -1;
    }

    //Checks command againsts given constraints
    if (!(position >= MIN_COMMANDED_POSITION
        && position <= MAX_COMMANDED_POSITION
        && velocity >= MIN_COMMANDED_VELOCITY
        && velocity <= MAX_COMMANDED_VELOCITY
        && acceleration >= MIN_COMMANDED_ACCELERATION
        && acceleration <= MAX_COMMANDED_ACCELERATION)){
        error_="Tried to issue a movement command with arguments that are out of bounds.";
//        qDebug()<<error_;
//        qDebug()<<"accel: "<<acceleration<<"Bounds ("<<MIN_COMMANDED_ACCELERATION<<","<<MAX_COMMANDED_ACCELERATION<<")";
//        qDebug()<<"vel: "<<velocity<<"Bounds ("<<MIN_COMMANDED_VELOCITY<<","<<MAX_COMMANDED_VELOCITY<<")";
//        qDebug()<<"pos: "<<position<<"Bounds ("<<MIN_COMMANDED_POSITION<<","<<MAX_COMMANDED_POSITION<<")";
        return false;
    }

   //sets the commanded position to the given position
    commandedPosition_ = position;
    bool status=ServoLoadTraj
            (
                    ADDRESS,
                    LOAD_POS | LOAD_VEL | LOAD_ACC | ENABLE_SERVO | START_NOW,
                    (int)(position * COUNTS_PER_REV), //Translate U to count.
                    (int)(velocity * COUNTS_PER_REV / TICKS_PER_SECOND * 65536), //Translate U/second to count/tick * 2^16.
                    (int)(acceleration * COUNTS_PER_REV / (TICKS_PER_SECOND * TICKS_PER_SECOND) * 65536), //Translate U/(second^2) to count/(ticks^2) * 2^16.
                    0, //pwm (no effect if not in pwm mode)
                    &error_) == 1;

//    qDebug()<<"Servo load traj: "<<status;
    return status;
}

bool Motor::moveRelative(double positionDelta, double velocity, double acceleration)
{
    return moveAbsolute(getPosition()+positionDelta, fabs(velocity), acceleration);
}

bool Motor::foreceStop()
{
    return cleanUp();
}

void Motor::stop()
{
    if(initialized_)
    {
        ServoStopMotor(ADDRESS, AMP_ENABLE | STOP_SMOOTH | ADV_FEATURE,&error_);
//        qDebug()<<QString("Result: ")<<
    }
}


bool Motor::setGroup(byte groupAddress, bool isLeader)
{
    return NmcSetGroupAddr(ADDRESS, groupAddress, isLeader, &error_) == true;
}

bool Motor::resetPosition()
{
    bool working = ServoResetPos(ADDRESS,&error_);
    commandedPosition_ = 0.0;
    return working;
}

bool Motor::moving()
{
    if(!initialized_){return false;}
    updateData();//updates the motor info, does this work? MOVE_DONE | PATH_MODE
    bool statAuxRead = NmcReadStatus(ADDRESS, SEND_AUX, &error_);
    //MOVE_DONE
    byte auxByte = ServoGetAux(ADDRESS); //does not work?
    bool pathMoving = ((auxByte & PATH_MODE) != 0);
    byte statbyte = NmcGetStat(ADDRESS);
    bool nonPathMoving = !(statbyte & MOVE_DONE);
    return pathMoving || nonPathMoving;
}

void Motor::waitMove()
{
    while(moving())
    {
    }
}

QString Motor::initialize()
{
    QString str;
    QTextStream ss(&str,QIODevice::WriteOnly);

    if(!initialized_)
    {

        if(NmcGetModType(ADDRESS) != SERVOMODTYPE)
        {
            ss << "Motor with address " << ADDRESS << " is not a PIC-SERVO motor.";
            return str;
        }
        if(ServoSetGain(ADDRESS,KP,KD,KI,IL,OL,CL,EL,SR,DB,&error_) != 1) //Probably want to use ServoSetGain2() here instead
        {
            ss << "Failed to set gains for motor with address " << ADDRESS << ".";
            return str;
        }

        ServoStopMotor(ADDRESS, AMP_ENABLE | STOP_ABRUPT | ADV_FEATURE,&error_);
        NmcDefineStatus(ADDRESS, SEND_POS | SEND_NPOINTS | SEND_PERROR | SEND_AUX,&error_);

        if(!resetPosition())
        {
            ss << "Could not reset position for motor with address " << ADDRESS << ".";
            return str;
        }
        initialized_ = true;
    }
    return "";
}

bool Motor::cleanUp()
{
    bool result;
    if(initialized_)
    {
        result = ServoStopMotor(ADDRESS, STOP_ABRUPT | MOTOR_OFF,&error_) == 1;// && NmcHardReset(ADDRESS,&error_) == 1;// HAHAH dont reset!!
        if(result)
        {
            initialized_ = false;
        }
    }
    else
    {
        result = true;
    }
    return result;
}

byte Motor::getAddress()
{
    return ADDRESS;
}

double Motor::getCountsPerRev()
{
    return (isReversed_ ? -1 : 1) * COUNTS_PER_REV;
}

double Motor::getMinPosition()
{
    return MIN_COMMANDED_POSITION;
}
double Motor::getMaxPosition()
{
    return MAX_COMMANDED_POSITION;
}
double Motor::getMinVelocity(){
    return MIN_COMMANDED_VELOCITY;
}

double Motor::getMaxVelocity(){
    return MAX_COMMANDED_VELOCITY;
}

double Motor::getMinAcceleration(){
    return MIN_COMMANDED_ACCELERATION;
}

double Motor::getMaxAcceleration(){
    return MAX_COMMANDED_ACCELERATION;
}

void Motor::setVelocity(double newVelocity)
{
    velocity_ = newVelocity;
}

void Motor::setAcceleration(double newAcceleration)
{
    acceleration_ = newAcceleration;
}

double Motor::getVelocity()
{
    return velocity_;
}

double Motor::getAcceleration()
{
    return acceleration_;
}

QString Motor::getErrors(){
    return error_;
}
