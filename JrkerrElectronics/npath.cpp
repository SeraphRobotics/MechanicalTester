#include "npath.h"

NPath::NPath():state_size_(0),allStates_(false),states_(state_size_)
{
}

NPath::NPath(int n):state_size_(n),isRelative_(false),allStates_(false)
{
    origin_.fill(0.0,n);
    states_.append(origin_);
}

NPath::NPath(int n, bool isRelative):state_size_(n),isRelative_(isRelative),allStates_(false)
{
    origin_.fill(0.0,n);
    states_.append(origin_);
}

void NPath::setAllStates(bool all){
    allStates_=all;
}

void NPath::setStateSize(int n){
    state_size_= n;
    states_.clear();
    origin_.fill(0.0,n);
    states_.append(origin_);
}


int NPath::stateSize(){
    return state_size_;
}

bool NPath::addState(QVector<double> v){
    if(v.size()!=state_size_){
        return false;
    }
    if (isRelative_){
        State newstate = addStates(v,states_.last(),allStates_);
        states_.append(newstate);
    }else{
        states_.append(v);
    }
    return true;
}

bool NPath::setState(int i, State new_state){
    if(new_state.size()!=state_size_){return false;}
    if(((i>states_.size()-1)&&isRelative_)||(i>states_.size())){return false;}
    if(isRelative_){
        State setstate = addStates(new_state,states_[i],allStates_);
        states_[i+1]=setstate;
        return true;
    }else{
        states_[i]=State(new_state);
        return true;
    }
}

QVector<double> NPath::getState(int i){
    if(i>states_.size()){
//        throw 2;
        //return new QVector<double>(state_size_,0.0);
    }
    State newstate;
    if(isRelative_){
        newstate = subStates(states_[i+1],states_[i],allStates_);
    }else{
        newstate = states_[i];
    }
    return newstate;
}

int NPath::numberOfStates(){
    if(isRelative_){
        return states_.size()-1;
    }else{
        return states_.size();
    }
}

void NPath::clear(){
    states_.clear();
}

void NPath::toRelative(){
    isRelative_=true;
}

void NPath::toAbsolute(){
    isRelative_=false;
}

void NPath::shift(State shiftby){
    if(shiftby.size()!=state_size_){return;}
    State neworigin = addStates(origin_,shiftby,allStates_);
    setOrigin(neworigin);
}

void NPath::setOrigin(State new_origin){
    if(new_origin.size()!=state_size_){return;}


    //Shift the states over by the diffrence between the origin and the new origin
    State delta = subStates(new_origin,origin_,allStates_);
    for(int i=0;i<states_.size();i++){
        states_[i] = addStates(states_[i],delta,allStates_);
    }
    origin_=State(new_origin);
}

State NPath::origin(){
    return State(origin_);
}

State NPath::lastAbsolute(){

    if(states_.isEmpty()){return State();}
    State lastabsolute(states_.last());
    return lastabsolute;
}


State addStates(State s1, State s2, bool allStates){
    if (s1.size()!=s2.size()){return State();}

    double size = s1.size();

    State newstate(size,s1[0]);// This ensures that the dt is used for the s1 if time is not edited

    int k=1;
    if (allStates){k=0;}

    for(int i=k;i<size;i++){
        newstate[i] = s1.at(i)+s2.at(i);
    }
    return newstate;
}

State subStates(State s1, State s2, bool allStates){
    if (s1.size()!=s2.size()){return State();}

    double size = s1.size();

    State newstate(size,s1[0]);// This ensured DT is proper if doing not allstates

    int k=1;
    if (allStates){k=0;}

    for(int i=k;i<size;i++){
        newstate[i] = s1.at(i)-s2.at(i);
    }
    return newstate;
}
