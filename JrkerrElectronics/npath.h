#ifndef NPATH_H
#define NPATH_H

#include <QList>
#include <QVector>

typedef QVector<double> State;

class NPath
{
public:
    /**
     * Constructor for an NPath: the machines' state space will be of size n.
     * 0 will be the time coordinate, all other coordinates will be for states 1->N
     */
    NPath();
    /**
     * Constructor for an NPath: the machines' state space will be of size n.
     * 0 will be the time coordinate, all other coordinates will be for states 1->N
     */
    NPath(int n);

    /**
     * Constructor for an NPath: the machines' state space will be of size n.
     * 0 will be the time coordinate, all other coordinates will be for states 1->N
     * the relative bool set if the NPath works in realtive states mode
     */
    NPath(int n, bool isRelative);

    /**
     * if set to true, the system acts a a hzpath, and there is no time index.
     * this is used for holding states which will be executed at a predetermined rate
     */
    void setAllStates(bool all);


    /**
     * Sets the size requirement for all states to bte added to the system
     * a state that is sized!=n will be rejected and not added.
     */
    void setStateSize(int n);

    /**
     * Returns the machines state space size.
     */
    int stateSize();
    /**
     * Addes a vector to the list of machine states.
     * if the added vector is not of the correct size
     * it will return false and not add the vector
     */
    bool addState(QVector<double>);
    /**
     * Returns the stateVector at index i if i<=states_.size()
     * Returns a null vector if i>states_.size()
     */
    QVector<double> getState(int i);

    /**
     * inserts a state at position n
     */
    bool setState(int i, State new_state);

    /**
     * returns the number of system states in the npath
     */
    int numberOfStates();

    /**
     * removes all states from the Npath
     */
    void clear();

    /**
     * Makes the Npath work in relative mode. All states are still stored internally in absolute coordinates
     * any state added is assumbed to tbe relative to the previous state.
     * any attempt to access a state will return a relative state
     */
    void toRelative();

    /**
     * Makes the Npath work in absolute mode. All states are still stored internally in absolute coordinates
     * any state added is assumbed to be in absolute coordinates.
     * any attempt to acces a state will return a state in absolute coordinates
     */
    void toAbsolute();

    /**
     * This will shift all states of the npath by the specified state provided it is of the correct size
     */
    void shift(State shiftby);

    /**
     * Sets the origin of hte npath. The states will shift by the diffrence between the new origin and the old origin
     */
    void setOrigin(State origin);

    /**
     * returns the current origin of hte npath
     */
    State origin();

    /**
     * returns the last absolute state of the machine the npath would drive the machine too.
     */
    State lastAbsolute();

private:
    QVector<QVector<double> > states_;
    int state_size_;
    bool isRelative_;
    State origin_;
    bool allStates_;

};

State subStates(State s1, State s2, bool allStates);
State addStates(State s1, State s2, bool allStates);

#endif // NPATH_H
