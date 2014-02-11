#include "xyzmotion.h"
#include "jsnpath.h"
#include <math.h>

XYZMotion::XYZMotion() :acceleration_(0),statesize_(0),frequency_(30.0) {}

XYZMotion::XYZMotion(const QDomNode& sourceDomNode):statesize_(0) {
    QDomNodeList bayChildren = sourceDomNode.childNodes();
    for(uint i=0; i<bayChildren.length();i++){
        QDomNode bchild = bayChildren.at(i);
        if (bchild.isComment()) {continue;}
        if ("acceleration" == bchild.nodeName().toLower()) {
            acceleration_ = bchild.toElement().text().toDouble();
        } else if ("axis" == bchild.nodeName().toLower()) {
            axis a;
            QDomNodeList bchildren = bchild.childNodes();
            for (int j=0; j < bchildren.size(); j++) {
                 QDomNode achild = bchildren.at(j);
                if (achild.isComment()) {continue;}
                if ("actuatorid"==achild.nodeName().toLower()) {
                    a.actuatorID = achild.toElement().text().toInt();
                } else if ("revolutionsperdistance"==achild.nodeName().toLower()) {
                    a.revPerDist = achild.toElement().text().toDouble();
                } else if ("range"==achild.nodeName().toLower()) {
                    a.range = achild.toElement().text().toDouble();
                } else if ("name"== achild.nodeName().toLower()) {
                    a.name = achild.toElement().text().toLower();
                }
            }
            axismap_[a.name] = a;
        }
    }
}

void XYZMotion::setAxes(QMap<QString,axis> axismap) {
    foreach(QString s,axismap.keys()) {
        axismap_[s.toLower()]=axismap[s];
    }
}

void XYZMotion::setAcceleration(double a) {
    if (0!=a) {acceleration_=a;}
}

double XYZMotion::getAcceleration()const{
    return acceleration_;
}

void XYZMotion::setFrequency(int frequency) {
    frequency_=frequency;
}

void XYZMotion::setIdMap(QMap<int,int> map) {
    idToStateIndex_ = map;
    statesize_ = map.keys().size()+1;
    defineDOFs();
}

void XYZMotion::accelerateAlong(NPath* np, QList<FabPoint> accel_list, double dist_a,bool decel) {
    /*
    Target points are the state locations in XYZ;
    waypoints are the points in the path list.
    The goal is to accelerate along the waypoints.
    */
    QListIterator<FabPoint> accelIt(accel_list);
    FabPoint lastpoint;
    FabPoint curpoint;
    FabPoint between_point;

    State tempState(statesize_,0.0);
    double coef= acceleration_/2.0/frequency_/frequency_;
    double v0=0;

    double dt=1.0/frequency_;
    double t1  = sqrt(2.0*dist_a/acceleration_);
    double timesteps = floor(t1*frequency_);

    double ri=0; //1/2*A*t_i^2  total distance from origin along the path to targetpoint
    double Rj=0; //sum(Dn,J)  total distance along waypoints to waypoint j
    double di=0; // r_i - Rj  Distance between current waypoint and target position
    double dj=0; // distance between current waypoint and the next

    if (decel) {
        v0 = acceleration_*t1/frequency_;
        coef = -coef;// Invert for decelleration
    }


    lastpoint = accelIt.next();
    curpoint = accelIt.next();
    dj = distance(lastpoint,curpoint);
    for (int i=1; i <= timesteps; i++) {
        ri = v0*i + coef*i*i;
        di = ri-Rj;

        if (di > dj && accelIt.hasNext()) {
            dj = distance(lastpoint,curpoint);
            Rj +=dj;
            lastpoint = curpoint;
            curpoint = accelIt.next();
            di = ri-Rj;
        }
        between_point = pointBetween(lastpoint,curpoint,di);
        tempState = positionToState(between_point.x,between_point.y,between_point.z);
        tempState[0] = dt;
        np->addState(State(tempState));
    }
}

NPath XYZMotion::pathAlong(XDFLPath path,double speed) {
//    XDFLPath grPath = path.getAsGlobalRelative();
    FabPoint origin = path.start();
    if (0==acceleration_ || 0==frequency_) {return NPath(statesize_);}

    speed = fabs(speed);
    NPath np(statesize_,false);
    State tempState(statesize_,0.0);
    double dist=0;
    double dist_a = speed*speed/2.0/acceleration_;
    double dj=0;
    path.toRelative();
    QList<FabPoint> points = path.getGlobalRelativePoints();
    FabPoint lastpoint = points.first();
    FabPoint curpoint;
    QList<FabPoint> accel_list;
    QList<FabPoint> decel_list;
    QList<FabPoint> coast_list;

    // Ensure that the system can accelerate over the distance of the path.
    if (path.length()<2.0*dist_a) {
        dist_a = .5*path.length();
    }

    // Populate acceleration list and find endpoint of acceleration.
    int accelInt=0;
    for (accelInt=0;accelInt<points.length();accelInt++) {
        if (dist + distance(lastpoint,points[accelInt]) >= dist_a) {
            double d = dist_a-dist;
            FabPoint accelstop = pointBetween(lastpoint,points[accelInt],d);
            accel_list.append(accelstop);
            break;
        }
        dist += distance(lastpoint,points[accelInt]);
        accel_list.append(points[accelInt]);
        lastpoint = points[accelInt];
    }

    // Populate deceleration list and find startpoint of deceleration.
    int decelInt=0;
    dist=0;
    lastpoint=points.last();
    for (decelInt=points.length()-1;decelInt>-1;decelInt--) {
        if (dist + distance(lastpoint,points[decelInt]) >= dist_a) {
            double d = dist_a-dist;
            FabPoint accelstop = pointBetween(lastpoint,points[decelInt],d);
            decel_list.prepend(accelstop);
            break;
        }
        dist += distance(lastpoint,points[decelInt]);
        decel_list.prepend(points[decelInt]);
        lastpoint = points[decelInt];
    }

    // Populate coasting list
    coast_list.append(accel_list.last());

    for (int i=accelInt; i < decelInt+1; i++) {
        coast_list.append(points[i]);
    }
    coast_list.append(decel_list.first());


    accelerateAlong(&np,accel_list,dist_a,false);//acceleration

    // Coasting
    if (coast_list.size()>1) {
        QListIterator<FabPoint> coasting(coast_list);
        lastpoint = coast_list.first();
        curpoint = coasting.next();
        while (coasting.hasNext()) {
            lastpoint = curpoint;
            curpoint = coasting.next();
            tempState = positionToState(curpoint.x,curpoint.y,curpoint.z);
            dj = distance(lastpoint,curpoint);
            tempState[0] = dj/speed; // TODO: this could lead to problems it dt<1/30
            np.addState(State(tempState));
        }
    }
    //deceleration
    accelerateAlong(&np,decel_list,dist_a,true);

    State temp = positionToState(origin.x,origin.y,origin.z);
    np.setOrigin(temp);
    np.toRelative();
    return np;
}

bool XYZMotion::defineDOFs(){
    if ( ( !axismap_.empty() ) && ( !idToStateIndex_.empty() ) ){
        DOFs_.clear();
        foreach(QString s,axismap_.keys()){
            DOF dof;
            dof.index=idToStateIndex_[axismap_[s].actuatorID];
            dof.name = s;
            dof.range = axismap_[s].range;
            dof.scale = axismap_[s].revPerDist;
            DOFs_.append(dof);
        }
        return true;
    }else{
        return false;
    }
}

bool XYZMotion::defineDOFs(double x, double y, double z){
    if(!defineDOFs()){return false;}

    double dist = sqrt(x*x+y*y+z*z);
    QList<DOF>::const_iterator j;
    for (j = DOFs_.constBegin(); j != DOFs_.constEnd(); ++j){
        DOF dof = (*j);
        if(dof.name.toLower()=="x"){
            dof.norm= x/dist;
        }else if(dof.name.toLower()=="y"){
            dof.norm= y/dist;
        }else if(dof.name.toLower()=="z"){
            dof.norm=z/dist;
        }
    }
}

QList<DOF> XYZMotion::getDOFs(){
    return QList<DOF>(DOFs_);
}

QMap<QString,axis> XYZMotion::getAxes(){
    return QMap<QString,axis>(axismap_);
}

NPath XYZMotion::pathTo(double x, double y, double z, double speed) {
    // http://people.mech.kuleuven.be/~bruyninc/blender/doc/interpolation-api.html#trapezoidal-velocity
    // dx_i = x_i/dist*a^2/(v^2*f^2)*(2i-1)

    NPath np(statesize_,false);
    double dist = sqrt(x*x+y*y+z*z);
    if (0==acceleration_ || 0==frequency_ || 0==speed || 0==dist) {return NPath(statesize_);}
    double dist_a = speed*speed/acceleration_/2.0;
    double dist_v =0;

    defineDOFs(x,y,z);
/*
    QList<DOF> DOFs;
    foreach(QString s,axismap_.keys()){
        DOF dof;
        dof.index=idToStateIndex_[axismap_[s].actuatorID];
        dof.name = s;
        dof.range = axismap_[s].range;
        dof.scale = axismap_[s].revPerDist;
        if(s.toLower()=="x"){
            dof.norm= x/dist;
        }else if(s.toLower()=="y"){
            dof.norm= y/dist;
        }else if(s.toLower()=="z"){
            dof.norm=z/dist;
        }
        DOFs.append(dof);
    }
    EAch axis has Name(QString,axis), index(int,idToSTateIndex_/axis, scale(double axis), norm (double, axes, x/y/z).
    int xindex = idToStateIndex_[axismap_["x"].actuatorID];
    int yindex = idToStateIndex_[axismap_["y"].actuatorID];
    int zindex = idToStateIndex_[axismap_["z"].actuatorID];
    double xscale = axismap_["x"].revPerDist;
    double yscale = axismap_["y"].revPerDist;
    double zscale = axismap_["z"].revPerDist;
    double norm_x = x/dist;
    double norm_y = y/dist;
    double norm_z = z/dist;
*/
    double coef=0;
    double timesteps=0;
    double dt=0;
    double v1 =0;
    double x1 = 0;
    //    double D=0;

    State tempState(statesize_,0.0);
    DOF tempDOF;
    coef = acceleration_/2.0/frequency_/frequency_;
    dt = 1.0/frequency_;

    if (dist<dist_a*2) {// Special case: acceleration distance is larger than distance
        dist_a = dist/2.0;
        double t1  = sqrt(2.0*dist_a/acceleration_);
        timesteps = floor(t1*frequency_);

        v1 = acceleration_*t1;
        x1 = acceleration_/2.0*t1*t1;
        for (int i=0; i <= timesteps; i++) {
            tempState[0] = 1.0/frequency_;
            QList<DOF>::const_iterator j;
            for (j = DOFs_.constBegin(); j != DOFs_.constEnd(); ++j){
                tempDOF = *j;//(*j);
                tempState[tempDOF.index] = tempDOF.norm*tempDOF.scale*coef*i*i;
            }
//            tempState[xindex] = xscale*norm_x*coef*i*i;
//            tempState[yindex] = yscale*norm_y*coef*i*i;
//            tempState[zindex] = zscale*norm_z*coef*i*i;
            np.addState(State(tempState));
        }

        // deceleration
        for (int i=0; i < timesteps+1; i++) {
            tempState[0] = 1.0/frequency_;
            QList<DOF>::const_iterator j;
            for (j = DOFs_.constBegin(); j != DOFs_.constEnd(); ++j){
                tempDOF = (*j);
                tempState[tempDOF.index] = tempDOF.norm*tempDOF.scale*(x1+v1*i*dt-coef*i*i);
            }
//            tempState[xindex] = norm_x*xscale*(x1+v1*i*dt-coef*i*i);//(2.0*i-1.0);
//            tempState[yindex] = norm_y*yscale*(x1+v1*i*dt-coef*i*i);//(2.0*i-1.0);
//            tempState[zindex] = norm_z*zscale*(x1+v1*i*dt-coef*i*i);//(2.0*i-1.0);
            np.addState(State(tempState));
        }

        // make sure to end on the target point
        tempState[0] = 1.0/frequency_;
        QList<DOF>::const_iterator j;
        for (j = DOFs_.constBegin(); j != DOFs_.constEnd(); ++j){
            tempDOF = (*j);
            tempState[tempDOF.index] = tempDOF.scale*tempDOF.norm*dist;
        }
//        tempState[xindex] = xscale*x;
//        tempState[yindex] = yscale*y;
//        tempState[zindex] = zscale*z;
        np.addState(State(tempState));

    } else {
        //make the ramp up .

        dist_v = dist-2.0*dist_a;
        double t1  = speed/acceleration_;
        timesteps = floor(t1*frequency_);

        v1 = acceleration_*t1;
        x1 = acceleration_/2.0*t1*t1;

        for (int i=0; i <= timesteps; i++) {
            tempState[0] = 1.0/frequency_;
            QList<DOF>::const_iterator j;
            for (j = DOFs_.constBegin(); j != DOFs_.constEnd(); ++j){
                tempDOF = (*j);
                tempState[tempDOF.index] = tempDOF.scale*tempDOF.norm*coef*i*i;
            }
//            tempState[xindex] = xscale*norm_x*coef*i*i;
//            tempState[yindex] = yscale*norm_y*coef*i*i;
//            tempState[zindex] = zscale*norm_z*coef*i*i;
            np.addState(State(tempState));
        }

        // coast state;
        double x2=x1+dist_v;
        tempState[0] = dist_v/speed;
        QList<DOF>::const_iterator k;
        for (k = DOFs_.constBegin(); k != DOFs_.constEnd(); ++k){
            tempDOF = (*k);
            tempState[tempDOF.index] = tempDOF.scale*tempDOF.norm*x2;
        }
//        tempState[xindex] = x2*norm_x*xscale;
//        tempState[yindex] = x2*norm_y*yscale;
//        tempState[zindex] = x2*norm_z*zscale;
        np.addState(State(tempState));


        //Ramp down
        for (int i=1; i < timesteps; i++) {
            tempState[0] = 1.0/frequency_;
            QList<DOF>::const_iterator j;
            for (j = DOFs_.constBegin(); j != DOFs_.constEnd(); ++j){
                tempDOF = (*j);
                tempState[tempDOF.index] = tempDOF.scale*tempDOF.norm*(x2+speed*i*dt-coef*i*i);
            }
//            tempState[xindex] = norm_x*xscale*(x2+speed*i*dt-coef*i*i);//(2.0*i-1.0);
//            tempState[yindex] = norm_y*yscale*(x2+speed*i*dt-coef*i*i);//(2.0*i-1.0);
//            tempState[zindex] = norm_z*zscale*(x2+speed*i*dt-coef*i*i);//(2.0*i-1.0);
            np.addState(State(tempState));

        }
        tempState[0] = 1.0/frequency_;
        QList<DOF>::const_iterator j;
        for (j = DOFs_.constBegin(); j != DOFs_.constEnd(); ++j){
            tempDOF = (*j);
            tempState[tempDOF.index] = tempDOF.scale*tempDOF.norm*dist;
        }
//        tempState[xindex] = xscale*x;
//        tempState[yindex] = yscale*y;
//        tempState[zindex] = zscale*z;
        np.addState(State(tempState));
        np.addState(State(tempState));

    }

    np.toRelative();
    return np;
}

State XYZMotion::positionToState(double x,double y,double z) {
/*
    int xindex = idToStateIndex_[axismap_["x"].actuatorID];
    double xscale = axismap_["x"].revPerDist;
    int yindex = idToStateIndex_[axismap_["y"].actuatorID];
    double yscale = axismap_["y"].revPerDist;
    int zindex = idToStateIndex_[axismap_["z"].actuatorID];
    double zscale = axismap_["z"].revPerDist;
    temp[xindex] = x*xscale;
    temp[yindex] = y*yscale;
    temp[zindex] = z*zscale;
*/
    State tempState(statesize_,0.0);
    DOF tempDOF;
    QList<DOF>::const_iterator j;
    for (j = DOFs_.constBegin(); j != DOFs_.constEnd(); ++j){
        tempDOF = (*j);
        if (tempDOF.name.toLower()=="x"){
            tempState[tempDOF.index] = tempDOF.scale*x;
        }else if (tempDOF.name.toLower()=="y"){
            tempState[tempDOF.index] = tempDOF.scale*y;
        }else if (tempDOF.name.toLower()=="z"){
            tempState[tempDOF.index] = tempDOF.scale*z;
        }

    }
    return tempState;
}

QVector<double> XYZMotion::positionFromState(State* state) {

    QVector<double> v(3,0);
    DOF tempDOF;
    QList<DOF>::const_iterator j;

    for (j = DOFs_.constBegin(); j != DOFs_.constEnd(); ++j){
        tempDOF = (*j);
        if (tempDOF.name.toLower()=="x"){
            v[0] = state->at(tempDOF.index)/tempDOF.scale;
        }else if (tempDOF.name.toLower()=="y"){
            v[1] = state->at(tempDOF.index)/tempDOF.scale;
        }else if (tempDOF.name.toLower()=="z"){
            v[2] = state->at(tempDOF.index)/tempDOF.scale;
        }
    }
    return v;
}

double XYZMotion::distanceFromState(State* state) {
    QVector<double> v = positionFromState(state);
    return sqrt(v.at(0)*v.at(0)+v.at(1)*v.at(1)+v.at(2)*v.at(2));
}

QVector<double> XYZMotion::buildSpace(){
    QVector<double> v(3,0);
    DOF tempDOF;
    QList<DOF>::const_iterator j;
    for (j = DOFs_.constBegin(); j != DOFs_.constEnd(); ++j){
        tempDOF = (*j);
        if (tempDOF.name.toLower()=="x"){
            v[0] = tempDOF.range;
        }else if (tempDOF.name.toLower()=="y"){
            v[1] = tempDOF.range;
        }else if (tempDOF.name.toLower()=="z"){
            v[2] = tempDOF.range;
        }
    }
    return v;
}
