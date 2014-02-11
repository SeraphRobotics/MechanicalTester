#ifndef COREINTERFACE_H
#define COREINTERFACE_H

#include <QThread>
#include "virtualmachines.h"
#include "xdflhandler.h"

class CoreInterface : public QObject
{
    Q_OBJECT
public:

    CoreInterface();

    enum SystemState {
        NotInitialized,
        Connected,
        FileLoaded,
        Printing
    };

signals:
    /**
     * emits when a call is made to the system when its in the wrong state
     */
    void outOfStateCall();

    /**
     * emits when a non-fatal error occurs
     */
    void error(QString);

    // Connection
    /**
     * emits when the system ditects comport for printers
     */
    void comPorts(QStringList l);

    /**
     * emits when a the system changes state
     */
    void stateChaged(int s);

    //GamePad
    /**
     * emits the current position of the system at regular intervals when the machine is not printing
     */
    void currentPosition(double x, double y, double z);

    /**
     * emits when the machine starts to move outside of a print
     */
    void moving();

    //Job

    /**
     * emits when an estimate of the time and volume of a config file is ready
     */
    void estimated(double time, double volume, int numberOfCommands);

    //Bay

    /**
     * emits when an XDFL file is parsed and materials are generated
     */
    void materialsAvailable(QMap<int,Material>);

    /**
     * emits when the material in a bay changes or when requested
     */
    void bayMaterial(int bayid, int materialid);

    //Print

    /**
     * emits durring a print when a new command is being begun
     */
    void currentCommand(int cmd);

    /**
     * emits when a material is needed in the print and not loaded into a bay
     */
    void needMaterialLoaded(int i);

    /**
     * emits when a print is done or canceled
     */
    void printsComplete();


public slots:

    // Connection

    /**
     * sets the config file and comport of the printer.
     * passing a comport of '' will use the comport of the config file
     */
    void setConfig(QString configFile,QString comport);

    // GamePad

    /**
     * Resets the current state of a connected and non printing machine to Zero
     */
    void resetPosition();
    /**
     * moves the machine to the position x,y,z at the speed given,
     * that the system is conected and not printing
     */
    void moveTo(double x, double y, double z, double speed);

    /**
     * moves the machine by the distances x,y,z at the speed given,
     * that the system is conected and not printing
     */
    void move(double x, double y, double z, double speed);

    /**
     * returns the current postion of the system
     */
    QVector<double> getCurrentPosition();

    // Job

    /**
     * sets the XDFL file for the print.
     */
    void setXDFL(QString xdfl); // this creates the thread but there should be a seperate estimating thread,
                                //or a seperate run for estimating

    // Bays
    /**
     * has the system emit the material id of the bayid given
     */
    void requestBayMaterial(int bayid);

    /**
     * sets the material in the bay given to the material specified
     */
    void setMaterial(int bayid,int materialid);

    /**
     * Moved the bay motor by the number of revolutions specified
     * this is a hack solution which needs to be rethought
     */
    void moveBayMotor(int bayid, double amount, double time);///Needs to be implemented better

    // Print

    /**
     * Starts the print if ready to print
     */
    void startPrint();

    /**
     * Pauses the print if printing. the system will stop moving at the end of the current command
     */
    void pausePrint();

    /**
     * resumes the print
     */
    void resumePrint();

    /**
     * Cancels the print. The current command will finish and the system will stop printing
     */
    void cancelPrint();

    /**
     * Kills all motion imediately and kills hte VM, resets the interface to a disconnected state
     */
    void forceStop();



private:
    /**
     * internal function used to update the state of the system
     */
    void setState(SystemState);

private slots:
    /**
     * used by the load config thread for processing. this should never be called by another object.
     */
    void configLoaded(); //ONLY FOR loadConfigThread

    // XDFL handling
    /**
     * internal slot used for handeling XDFL printing
     */
    void donePrinting();// ONLY FOR XDFLHANDLER

    /**
     * internal slot used for handeling XDFL printing
     * connects the XDFL Handler to the emit of the CoreInterface currentCommand signal
     */
    void processingCommand(int i);//ONLY FOR XDFLHANDLER

    /**
     * internal slot used for handeling XDFL printing
     * connects the XDFLHandler to the CoreInterface needMaterialLoaded signal
     */
    void needMaterial(int i);//ONLY FOR XDFLHANDLER

    void XDFLestimated(double t, double v, int cmd);

public:
//    VirtualPrinter *vm_;
    VMPrototype *vm_;
private:

    XDFLHandler *handler_;
    SystemState state_;
    QMap<int,Material> idMaterialMap_;
    QTimer positionTimer_;
    QString config_;
    QString comport_;
};

#endif // COREINTERFACE_H
