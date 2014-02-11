#include "nmotion.h"

#include <math.h>
#include "nmccom.h"
//#include "sio_util.h"
#include "picservo.h"


#include <QMapIterator>
#include <QTextStream>

#include <QFile>
#include <QDebug>

namespace NP {
//---------------------------------------------------------------------------
//Globals:
int pathsize_;				//local path size
int curseg_;                 //current segment number
int curppoint;				//current pathpoint

int pathfreq_ = P_30HZ;		//selected path frequency
int bufsize =0;   				//max num points to store in the PIC-SERVO buffer
byte group_ = 0xFF;			//group address for coordinated controllers
byte leader_ = 0x00;			//group leader address for coordinated controllers




int stateSize=0;
QMap<int,double> scales_;
QMap<int,int> motors_;
HzPath states_;

int maxattempts_ = 30;      //the max num attempts for dl path


void resetStates(){
    states_=HzPath(stateSize);
    curseg_=0;
}

void makeTest(){
    HzPath testPath(stateSize,true);
    for(int i=0;i<states_.numberOfStates();i++){
        State temp = states_.getState(i);
        State newstate(states_.stateSize());
        for (int j=0; j<states_.stateSize();j++){
            newstate[j] = temp.at(j);
        }
        testPath.addState(newstate);
    }
    int j= curseg_;
    int k;


}


//---------------------------------------------------------------------------
//Initialize various parameters for this path generation module
//Returns -1 if Status items are not set properly,
//        -2 if scale factors are zero
int SetPathParams(int statesize, int freq, int nbuf,
                  QMap<int,int> motorMap, int groupaddr, int leaderaddr,
                  QMap<int,double> scales, QString* error_string ){

    byte statitems;
    byte ioctrl;
    byte tempId;

    stateSize=statesize;
    pathfreq_ = freq;                //set to 30 or 60 hz
    bufsize = nbuf;   	  	//max num points to store in the PIC-SERVO buffer
    group_ = (byte)groupaddr;
    leader_ = (byte)leaderaddr;


    //returns -2 if a scale factor is less than 1 count per U
    QMapIterator<int,double> k(scales);
    while(k.hasNext()){
        k.next();
        double temp = fabs(k.value());
        if (temp<1.0){return -2;}
    }
    scales_ = scales;



    QMapIterator<int,int> i(motorMap);
    while(i.hasNext()){
        i.next();
        tempId = (byte)i.value();
        if (freq == P_120HZ){    //set fast path mode if using 120 Hz path
            ioctrl = ServoGetIoCtrl(tempId);
            ServoSetIoCtrl(tempId, (byte)(ioctrl | FAST_PATH),error_string);
        }else{    //clear fast path bit if using slower modes
            ioctrl = ServoGetIoCtrl(tempId);
            ServoSetIoCtrl(tempId, (byte)(ioctrl & ~((byte)(FAST_PATH)) ),error_string);
        }
    }



    //Check that the required status data will be returned with each command:
    statitems = SEND_POS | SEND_NPOINTS | SEND_PERROR | SEND_AUX;
    QMapIterator<int,int> j(motorMap);
    while(j.hasNext()){
        j.next();
        tempId = (byte)j.value();
        if ( (NmcGetStatItems(tempId)& statitems) != statitems ){
            QTextStream ss(error_string,QIODevice::WriteOnly);
            ss<<"Required status items have not been set";
            return(-1);
        }
    }
    motors_=motorMap;
    return(0);
}




//---------------------------------------------------------------------------
//Adds points to path buffer -
//
//Returns: -1 if path download is done
//         curseg if in middle  of the path
//		   -2 if communication error
int DlPathPoints(QString* error_string){
    long int pointBuffer[7];  //set of up to 7 pathpoints
    int pointcount=0;
    byte tempId = 1;
    int testingBuffer = bufsize;
    bool testingReadStat;
    bool inpathmode;



    //retrieve status from each of the motors, return -2 if failure
    //this is basically a nice, final check that the motors are online, but it's superfluous, since each attempt to send points
    //that will be done below basically does the same thing.
    QMapIterator<int,int> j(motors_);
    while(j.hasNext()){
        j.next();
        tempId = (byte)j.value();
        if (!NmcNoOp(tempId,error_string)) {
            QTextStream ss(error_string, QIODevice::WriteOnly);
            ss<<" error checking status at address "<<tempId;
            return(-2);//make sure data is updated even if points are not added
        }
    }


    //if not in path mode
    testingReadStat = NmcReadStatus(leader_, SEND_NPOINTS,error_string);
    testingBuffer = ServoGetNPoints(leader_);
//    qDebug()<<"number of points in buffer"<<ServoGetNPoints(leader_);


    if ( !(ServoGetAux(leader_) & PATH_MODE) || (ServoGetNPoints(leader_)==0) ){
        //normally, we should not be in pathmode and should enter this part once

        //qDebug("NOT in pathmode");

        if (ServoGetNPoints(leader_)>bufsize) { //if there are more points in the buffer than that buffer size, stop
            QTextStream ss(error_string, QIODevice::WriteOnly);
            ss<<"Buffer is overflowing with joy... I mean points";
            return(-2); //something is wrong, stop executing
        }

        //if total number of points is less than bufsize, download all points
        NmcReadStatus(leader_, SEND_NPOINTS,error_string);
        if (states_.numberOfStates() <= bufsize){
//            qDebug("total number of points is less than bufsize, download all points");
            while(curseg_ < states_.numberOfStates()){
                 DlSev(error_string, pointcount, leader_, pointBuffer);
            }
        }
        //else download points to fill half of the buffer
        else {
            //qDebug("download points to fill half of the buffer");
            while(curseg_ < bufsize/2){
                  //qDebug("downloading 7 points");
                  DlSev(error_string, pointcount, leader_, pointBuffer);
            }
            //qDebug("done downloading");
        }

        //Start pathmode, If fail return
        if (StartPathMode(error_string)==-2){return -2;}

    }

    if ( (ServoGetAux(leader_) & PATH_MODE)){
//        qDebug("in pathmode, Downloading the rest of the points in the path");
        NmcReadStatus(leader_, SEND_NPOINTS,error_string);
//        qDebug()<<"Number of points in buffer:"<<ServoGetNPoints(leader_);

        int numberOfAttempts=0;
        while((curseg_ < states_.numberOfStates())&&(numberOfAttempts<100)){
            //if there is no space in the buffer, nap for 50 ms
            NmcNoOp(leader_,error_string);   //force the leader to return the current status so that we can read the number of points in the buffer

            //            NmcReadStatus(leader_, SEND_NPOINTS,error_string);
            //            qDebug()<<"Number of points in buffer:"<<ServoGetNPoints(leader_);
            inpathmode = ServoGetAux(leader_) & PATH_MODE;
            //            qDebug()<< inpathmode;
            if (!inpathmode){
                if (StartPathMode(error_string)==-2){return -2;}
                numberOfAttempts=0;
            }

            if(ServoGetNPoints(leader_)>=(bufsize-7)){
//                qDebug("not enough space in buffer");
                nap(50);
                numberOfAttempts++;
            }
            //else add 7 points
            else{
                numberOfAttempts =0;
//                qDebug("downloading 7 points");
                DlSev(error_string, pointcount, leader_, pointBuffer);
            }
        }
        return (-1); //done downloading all points
    }
    else{
        qDebug("ERROR!!!!");
        return(-2);
    }
}
//Downloads 7 points to all motors. Returns the number of points added, and updates curseg_
int DlSev(QString* error_string, int pointcount, byte tempId, long int pointBuffer[]){
//    qDebug()<<">>>>>>>>>>>>>Point<<<<<<<<<<<<";
    QMapIterator<int,int> i(motors_);
    while(i.hasNext()){
        i.next();
        tempId = (byte)i.value();
        //Load the point buffer for the motor at index i.key() from the statelist
        pointcount = GetNextPoints(i.key(),pointBuffer,7);  //NOTE! why does this work!! pointBuffer is passed by value, and therefore shouldn't return data
        //Adds points to the servos buffer
        ServoAddPathpoints(tempId, pointcount, pointBuffer, pathfreq_,error_string);//TODO! Need to handle errors here

    }
    curseg_+=pointcount;
    return pointcount;
}

int StartPathMode(QString* error_string){
    //start path mode
//    qDebug("START PATH");
    bool check = true;
    int numattempts = 0;
//    qDebug("starting pathmode");
    while (check){
        if (!ServoStartPathMode(group_, leader_,error_string)){
            numattempts++;
            if (numattempts>=maxattempts_) {
                QTextStream ss(error_string, QIODevice::WriteOnly);
                ss<<maxattempts_<<" number of communication attempts failed.";
                check=false;
                return(-2);
            }
        }
        else { check = false;}
    }
    return -1;
}


int GetNextPoints(int stateVariableIndex,long int pointBuffer[],int bufferLength){

    QVector<double> v;
    int pointcount=bufferLength;

    if (bufferLength>(states_.numberOfStates()-curseg_)) //handle the end-case where there are fewer states to be fetched than we've asked for in "bufferLength"
    {
        pointcount = (states_.numberOfStates()-curseg_);
    }
//    qDebug()<<"axis "<<stateVariableIndex;
    for (int i=0;i<pointcount;++i){
        v= states_.getState(curseg_+i);
        double distanceinrev = v.at(stateVariableIndex-1);//do account for HzPath not having a dt as the first state variable
        double distanceincounts = distanceinrev*scales_[stateVariableIndex];
        pointBuffer[i]= (long int)distanceincounts;
//        qDebug()<<" "<<i<<":"<<distanceincounts;

    }
    return pointcount;
}
//---------------------------------------------------------------------------
//Adds points to the states_ list. These states are spaced at a set time interval apart and will be uploaded to the system
//
//
//Returns: 0 if it fails
//         1 of it works
int AddStates(HzPath statelist){
    if (states_.numberOfStates()<2){//==0){
        states_=statelist;
        return 1;
    }else if(states_.stateSize()!=statelist.stateSize()){
        return 0;
    }
    // make a copy of the state that is disconected from the original state. This is needed for a global value.
    for(int i=0;i<statelist.numberOfStates();i++){
        State temp = statelist.getState(i);
        State newstate(statelist.stateSize());
        for (int j=0; j<statelist.stateSize();j++){
            newstate[j] = temp.at(j);
        }
        states_.addState(newstate);
    }
    return 1;

}

void ClearStates(){
    states_.clear();
}

//---------------------------------------------------------------------------
//Initializes the coordinated path after all of the segments have been added.
//This function should be called just before the application starts calling
//the function AddPathPoints().
//Returns 1 if it works.
//Returns 0.0 on communications error
float InitPath(QString* error_string){


    curseg_ = 0;

    byte tempId;

    QMapIterator<int,int> i(motors_);
    while(i.hasNext()){
        i.next();
        tempId = (byte)i.value();
        //make sure we exit path mode first
        if (!ServoStopMotor(tempId, ServoGetStopCtrl(tempId) & (byte)AMP_ENABLE,error_string)) return(0.0);
    }

    QMapIterator<int,int> j(motors_);
    while(j.hasNext()){
        j.next();
        tempId = (byte)j.value();
        //set the beginning of the path to the current position
        ServoInitPath(tempId,error_string);
    }


    return(1);
}

//---------------------------------------------------------------------------
//
//Converts the Npath to the HzPath, IE a list of states seperated by a time of 1/pathFreq.
//returns the list of states which the system will attempt to reach
//

HzPath toHzPath(NPath* npath,int startIndex){
    //ToHzPath takes in an ABSOLUTE position npath, and returns and absolute hzPath


    // When HzPaths are used, they are used in absolute coordinates
    HzPath hzpath(npath->stateSize()-1,true);
    hzpath.setAllStates(true);
    hzpath.toRelative();
    QVector<double> h_origin = QVector<double>(npath->stateSize()-1,0.0);
    QVector<double> origin = QVector<double>(npath->stateSize(),0.0);
    QVector<double> statebuffer = QVector<double>(npath->stateSize(),0.0);
    QVector<double> joinedState = QVector<double>(npath->stateSize(),0.0);
//    QVector<double> laststate = QVector<double>(npath->stateSize()-1,0.0);
    QVector<double> currentState;
    double dStateTime=0;
    double times = 0;
    double timestep = 1.0;

    switch (pathfreq_)       //calculate acceleration in units per tick^2
            {
        case P_30HZ: timestep = timestep/30.0;
                             break;
        case P_60HZ: timestep = timestep/60.0;
                             break;
        case P_120HZ: timestep = timestep/120.0;
                             break;
        default: timestep = timestep/30.0;
        }






////make sure you end at zero
    npath->toRelative();
    State temp(npath->stateSize(),0.0);
    temp[0]=timestep;
//    npath->addState(temp);// this ensures that the system ends with a zero delta on one time step
    // This patches a bug somewhere in this code, that causes a jerkyness if your  motor profile is /\ for a channel


    for(int i=startIndex;i<npath->numberOfStates();i++){// check to see if its in side of states


        currentState = npath->getState(i);

        dStateTime = fabs(currentState.at(0)); // time aloted to drive the system to the state specified

        if ((dStateTime < timestep)){// if the transition time is less than or equal frequency step, append the point to the buffer
            for(int k=0;k<npath->stateSize();k++){
                statebuffer[k] = currentState.at(k) + statebuffer.at(k);
            }
            //TODO: nearly identical code is repeated bellow, there must be a better way to format this code


            // if the buffer becomes larger than the timestep, append to hzpath
            if (statebuffer.at(0)> timestep){
                dStateTime = statebuffer.at(0);
                times = floor(fabs(dStateTime/timestep));
                for (int j=0;j<times;j++){
                    QVector<double> delta = QVector<double>(npath->stateSize()-1,0.0);
                    for(int k=1;k<npath->stateSize();k++){// Add a
                        // X       = ( Dx             /  Dt)     * dt
                        delta[k-1] = statebuffer.at(k)/dStateTime*timestep;
                    }
                    hzpath.addState(delta);
                }
                // subtract the added amount from the buffer
                statebuffer[0] = statebuffer.at(0) - times*timestep;
                for(int s=1;s<npath->stateSize();s++){
                    statebuffer[s] = statebuffer.at(s)*(1.0-1.0/dStateTime*(timestep*times));
                }


            }

        }else{//if dStateTime != timestep, then process it this way

            // Add the state buffer to the state, this acts as a dt and dx offset for points that couldnt be added

            for (int l=0; l<currentState.size();l++){
                joinedState[l] = currentState.at(l)+statebuffer.at(l);
            }

            statebuffer.fill(0);

            dStateTime = fabs(joinedState.at(0));
            if(dStateTime == timestep){
                times = 1.0;
            }else{
                times = floor(dStateTime/timestep);
            }
            for (int j=0;j<times;j++){
                QVector<double> n = QVector<double>(npath->stateSize()-1,0.0);
                for(int k=0;k<npath->stateSize()-1;k++){
                    //(X = (Dx                / Dt)      *dt)
                    n[k] = joinedState.at(k+1)/dStateTime*timestep;
                }
                hzpath.addState(n);
            }


            //  add the leftover to the buffer
            statebuffer[0] = joinedState.at(0) - floor(times)*timestep;
            for(int s=1;s<npath->stateSize();s++){
                statebuffer[s] = joinedState.at(s)*(1.0-1.0/dStateTime*(timestep*times));
            }

        }
    }



    // Get the origin of the current npath
    origin = npath->origin();
    for(int j=1;j<npath->stateSize();j++){
        h_origin[j-1] = origin.at(j);//currentState->at(j);
    }

    hzpath.setOrigin(h_origin);

    //Add the last point latter on to ensure it makes it accurately
    State endstate = npath->lastAbsolute();
    State h_endstate(npath->stateSize()-1,0.0);


    for(int d=0;d<npath->stateSize()-1;d++){
       h_endstate[d] = endstate.at(d+1);
    }



    hzpath.toAbsolute();
    hzpath.addState(h_endstate);
    return hzpath;

}


}
