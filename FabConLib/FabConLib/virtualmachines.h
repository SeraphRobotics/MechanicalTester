#ifndef VIRTUALMACHINEENVIROMENT_H
#define VIRTUALMACHINEENVIROMENT_H

#include <QScriptEngine>
#include <QDomDocument>
#include "motor.h"
#include "electronicsinterface.h"
#include "bay.h"
#include "jscmotion.h"
#include "xyzmotion.h"
#include "jsxyz.h"

class VMPrototype:public QObject
    {
    Q_OBJECT
    public:
        VMPrototype();

        QScriptEngine* makeEngine();

    public slots:
        /**
         * Sets the override COM port for the VM which will be used in loadConfig.
         */
        void setComPort(QString comPort);
        /**
         * @return true if loadConfig() succeeded.
         */
        bool isInitialized();
        /**
         * Used to limit the reach of the upper level code.
         * @return the size of a valid NPath.
         */
        int nstatesize();

        /// These methods should be overriden by actual VM and test VM classes.

        /**
         * Builds a virtual machine based on the QDomDocument.
         * Allows the document to be build by a handler program rather than a file.
         */
        virtual void loadConfig(QDomDocument document);
        /**
         * @return a string of all errors.
         */
        virtual QString getErrors();
        /**
         * Used to limit the reach of the upper level code.
         * @return true if path is valid and queued.
         */
        virtual bool executeNPath(NPath path);

        virtual void dumpstates();

        virtual bool executeRelativeNPath(NPath path);

        virtual State currentState();

        QVector<double> currentPosition();

        bool moveTo(double x, double y, double z, double speed);
        bool move(double x, double y, double z, double speed);

        bool forceStop();

        virtual void stop();

        virtual void resetPosition();

        virtual void waitMove();

    public:

        ElectronicsInterface eInterface;// this should be private. Its only here to alow for loadconfig thread to work
        QList<Bay*> bays;
        XYZMotion* xyzmotion;

    protected:
        QString comPort_;
        QString error_string_;
        bool initialized_;
        int statesize_;
        JsXYZ jsxyz_;
        State laststate_;
        QMap<int,int> idtostatemap_;
        int frequency_;
    };



class VirtualPrinter:public VMPrototype
{
    Q_OBJECT
public:
    VirtualPrinter();

public slots:
    void loadConfig(QDomDocument document);
    QString getErrors();
    bool executeNPath(NPath path);

    void dumpstates();

    bool executeRelativeNPath(NPath path);
    State currentState();
    bool forceStop();
    void stop();
    void resetPosition();

    bool move(double x, double y, double z, double speed);
    void waitMove();
//public:
//    ElectronicsInterface eInterface; //TODO: this should be private. Nothing above VM should access it.
};



class TestPrinter:public VMPrototype
{
    Q_OBJECT
public:
    TestPrinter();


public slots:
    void loadConfig(QDomDocument document);
    bool executeNPath(NPath path);
    bool executeRelativeNPath(NPath path);
    State currentState();
    void dumpstates();
    void resetPosition();
    void waitMove();
private:
    QList<NPath> totalprintcommands_;
};


/**
 * Runs a script with access to motors and coordinated motion instances.
 */
QString runScript(VirtualPrinter *vm, QString script_);

#endif // VIRTUALMACHINEENVIROMENT_H
