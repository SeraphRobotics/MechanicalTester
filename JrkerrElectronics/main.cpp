#include <stdio.h>
#include <QCoreApplication>
#include <QDomDocument>
#include <QFile>
#include <QTextStream>
#include "nmotion.h";
#include "npath.h";
#include "electronicsinterface.h"



int main(int argc, char *argv[]) {
    QCoreApplication app(argc,argv);
    QString configFilePath= "hobby-servo.config";   //"./testConfig.config";
    QString ComPort=""; ///dev/tty.usbserial-A400gNfH";   //"tty.usbserial-A9003UF2";
    ElectronicsInterface eInterface;
    QDomDocument document;
    // load the config file into the DOM document
    {
        QFile configFile(configFilePath);
        if (!configFile.open(QFile::ReadOnly)) {
            printf("\nFAILED TO OPEN CONFIG FILE\n");
            app.exec();
        }
        document.setContent(&configFile);
        configFile.close();
    }
    // Load the config to create the electronics
    QDomElement root = document.documentElement();
    QDomNode electronics = root.namedItem("electronics");
    //initialized the electronics
    QString result = eInterface.initialize(electronics,ComPort);
    printf("\ninitialization results %s\n",result.toStdString().c_str());
    if (eInterface.isInitialized()){
        printf("\nmoving motor!@!!!!");
        // eInterface.getCoordinatedMotion()->initializePathMode();
        // eInterface.getMotor(1)->moveAbsolute(1,1,1);
        // eInterface.getMotor(1)->waitMove();*/
        CoordinatedMotion* cmotion = eInterface.getCoordinatedMotion();
       int x = 1;
       int dx =- 1;
       int y = 0;
       int dy = 1;
       int z = 0;
       int dz = 0;
       int t = 4;

        State v1(4,0);
        v1[0]=t;
        v1[1]=1;
        v1[2]=y;
        v1[3]=z;

        State v2(4,0);
        v2[0]=t;
        v2[1]=-1;
        v2[2]=y;
        v2[3]=z;

        State v3(4,0);
        v3[0]=t;
        v3[1]=dx;
        v3[2]=dy;
        v3[3]=z+dz;

        State v4(4,0);
        v4[0]=t;
        v4[1]=x;
        v4[2]=dy;
        v4[3]=z;

        State v5(4,0);
        v5[0]=t;
        v5[1]=x;
        v5[2]=y;
        v5[3]=z;


     /*   State v3(5,0.0);
        v3[0]=6.0; */
        NPath p1(4);
        p1.addState(v1);
        p1.addState(v2);
       // p1.addState(v3);
        //  p1.addState(v4);
     //   p1.addState(v5);
       // p1.addState(&v3);
        //        State v4(5,0.0);
        //        v4[0]=0;
       /* State v5(5,0.0);
        v5[0]=6.0;
        v5[1]=0;
        v5[2]=0;
        v5[4]=1.5;
        v5[3]=1.5;
        State v6(5,0.0);
        v6[0]=6.0;
        NPath p2(5);
        //        p2.addState(&v4);
        p2.addState(&v5);
        p2.addState(&v6); */
        if (cmotion->initializePathMode()){
            printf("\ntrying coordinated motion");
            cmotion->moveAlongPath(p1);
           // cmotion->moveAlongPath(p2);
        }
    }
    eInterface.waitOnMove();
    printf("\nErrors: %s",eInterface.getErrors().toStdString().c_str());
    eInterface.reset();
    printf("\nDone\n");
    app.exit(true);
    return app.exec();//return 0;
}
