#ifndef XDFLHANDLER_H
#define XDFLHANDLER_H

#include <QObject>
#include <QDomNode>
#include "virtualmachines.h"
#include "xdflpath.h"
#include "xdflvoxel.h"
#include "material.h"

// Create one XDFLHandler object per VM and XDFL file. The file must be set in the constructor.
class XDFLHandler : public QThread
{
    Q_OBJECT


public:
    // Accessing the handler's state must be threadsafe so it can be set during processing.
    // State transitions: Ready > Running <> Paused > Stopped > Ready
    enum HandlerState {
        Ready,
        Running,
        Paused,
        Stopped
    };
    /**
     * Default constructor of the XDFL handler,
     * this will process the XDFL file and run the print job
     * the VM and xdfldom must be set before the system will print
     */
    XDFLHandler();
    /**
      Note that since no default constructor is provided, an XDFLHandler
      object must be explicitly initialized to avoid undefined behavior.
      **/
    XDFLHandler(VMPrototype* vm, QDomDocument xdfl);


    void estimate();

    /**
     * returns the estimated time of a print job will take
     */
    double getEstimatedTime();

    /**
     * returns the estimated volume of material the system will consume
     */
    double getEstimatedVolume();

    /**
     * returns the number of commands in the currently set XDFL file
     */
    int getNumberOfCommands();

    /**
     * sets the Virtual Machine the program will use
     */
    void setVM(VMPrototype* vm);

    /**
     * Set the XDFL file DOM that will be used in the print
     */
    void loadFromDom(QDomDocument xdfl);



    int getState();

    void setState(HandlerState i);

public slots:
 // void start(); // Inherited from QThread. Calling it will start the thread and execute run().
    /**
     * Slot for pausing the print. It will stop moving at the end of the current command
     */
    void pause();

    /**
     * Slot for resuming a print form the next queued command
     */
    void resume();

    /**
     * Slot for canceling a print. the print will end and the end of the current command
     */
    void cancel();

    void forceStop();


signals:
    /**
     * emited when estimation is completed
     */
    void estimated(double t, double v, int ncmd);

    /**
     * emited when a material in the XDFL file is needed and not loaded in a bay
     * emits the id of the material
     */
    void needMaterialChange(int id); // Signals integer of the material ID that needs to be loaded


    /**
     * emits the number of commands in the XDFL file
     */
    void numberOfCommands(int cmds);

    /**
     * emits when a new command is started
     */
    void startingCommand(int cmd);


private:
    void updateState();

    void run(); // Reimplemented from QThread: called by start(). When it exits the thread is finished.
    void processCommand(); // This function will be called every time handling is started or resumed.
    NPath dwell(double time_in_ms);
    void runNPath(NPath n);
    bool setMaterial(int id);
    void updateInfo();

public:
    QMap<int,Material> getMaterials(); // When is this used?
    QMap<int,Material> mat_map;     // When is this used [as public]?

private:
    HandlerState handlerstate_;
    QWaitCondition resumed_;
    QMutex mutex_;
    VMPrototype *vm_;
    QDomNodeList commands_;
    bool estimationDone_;
    double estimatedTime_;
    double estimatedVolume_;
    //QDomNode xdfldom_;
    QMap<int,Bay*> material_bay_mapping_;
    int current_material_;
    unsigned int current_command_;
    FabPoint last_end_point_;
    State laststate_; // machine state (a vector; distinct from the handler state)
    bool needMaterial_;
};

#endif // XDFLHANDLER_H
