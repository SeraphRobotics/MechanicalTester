#ifndef LOADCONFIGTHREAD_H
#define LOADCONFIGTHREAD_H

#include <QThread>
#include "virtualmachines.h"

class LoadConfigThread : public QThread
{
    Q_OBJECT
public:

    /**
     * Default constructer for a thread which will ofload the loading of hte vm fmr the main thread
     */
   LoadConfigThread();

   /**
    * Overide constructor which will set the vm and the config DomDocument at construction
    */
   LoadConfigThread(VMPrototype* vm,QDomDocument config);//VirtualPrinter* vm,QDomDocument config);

   /**
    * sets the VM of the system
    */
   void setVM(VMPrototype* vm);

   /**
    * sets the config file Dom to be loaded
    */
   void setConfig(QDomDocument config);

signals:

   /**
    * emits when the thread is ready to tbe started
    */
   void ready();

   /**
    * emits when the vm is initialized and loaded
    */
   void loaded();

public slots:
   /**
    * the function which is called by Start() to do the processing
    */
   void run();

private:
   /**
    * function to check if the system is ready to be run
    */
   void checkReady();

private:
   VMPrototype* vm_;
   QDomDocument config_;
   bool ready_;
   QThread* vmThread_;
};

#endif // LOADCONFIGTHREAD_H
