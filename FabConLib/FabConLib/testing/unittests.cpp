#include "unittests.h"
#include "util.h"
#include <QDebug>
void testNPaths(VMPrototype *vm) {
//     DIRECT PATHING


        NPath np(5,false);


        State o(5,-1.0);
        o[0]=0;
        np.setOrigin(o);
        State s1(5,-2.0);
        s1[0]=1;
        State s2(5,-1.0);
        s2[0]=1;
        State s3(5,0.0);
        s3[0]=1;
        np.addState(s1);
        np.addState(s2);
        np.addState(s3);
		
//        np.setOrigin(np.lastAbsolute());
        bool t = vm->executeNPath(np);
        qDebug(" printed %s",t ? "true" : "false");





}

void testScripting(VirtualPrinter *vm) {
    //SCRIPTING

        QString script_;
        { // load the source file into the DOM document
          QFile scriptFile("../../../media/TestScript.js");
          if (!scriptFile.open(QFile::ReadOnly |QIODevice::Text)) {
              qDebug("FAILED TO LOAD SCRIPT");
              return;
          }
          QTextStream in(&scriptFile);
          script_ = in.readAll();
          scriptFile.close();
        }

        QString errors = runScript(vm,script_);
        qDebug("Script: %s",script_.toStdString().c_str());
        qDebug("Script Engine Errors: %s",errors.toStdString().c_str());
}

void testVoxels(VMPrototype *vm) {
    //// XDFL VOXEL
        Material m;
        m.id =1;
        m.name="test-silicone";
        m.Q = 16.8;// near silicone mm^3/s
        m.property["pathspeed"] = "30";//mm/2
        m.property["compressionVolume"]="0";//mm^3
        vm->bays[0]->setMaterial(m);

        XDFLVoxel vox;
        vox.id = 1;
        vox.volume=101.95;
        vox.x=0;
        vox.y=-20;
        vox.z=0;

        NPath voxpath = vm->bays[0]->onVoxel(vox);
        vm->executeNPath(voxpath);
//        voxpath.setOrigin(voxpath.lastAbsolute());
//        vm->executeNPath(voxpath);
//        qDebug("");
//        voxpath.toAbsolute();
//        //voxpath.setOrigin(voxpath.lastAbsolute());
//        voxpath.toRelative();
//        voxpath.lastAbsolute();
}

void testXDFLPaths(VMPrototype *vm) {
//    XDFL PATH
        qDebug("testing XDFL paths directly");
        XDFLPath path1;
        NPath pathed1;
        FabPoint p1,p2,p3;
        p1.x=0;
        p1.y=0;
        p1.z=0.8;
        p2.y=0;
        p2.x=-6;
        p2.z=0.8;
        p3.y=0;
        p3.x=-70;
        p3.z=0.8;
//        path1.points.append(p0);
        path1.points.append(p1);
        path1.points.append(p2);
        path1.points.append(p3);
        path1.materialID=1;

        Material m;
        m.id =1;
        m.name="test-greentip-silicone";
        m.Q = 20.587;// silicone green tip mm^3/s
        m.property["pathspeed"] = "30";//mm/2
        m.property["compressionVolume"]="58.37";//mm^3
        vm->bays[0]->setMaterial(m);


        pathed1 = vm->bays[0]->onPath(path1);

//        pathed1 = vm->xyzmotion->pathAlong(path1,30);
        qDebug("paths calculated");
        vm->moveTo(p1.x,p1.y,p1.z,30);
//        vm->moveTo(0,0,-1,10);
        vm->executeRelativeNPath(pathed1);
        qDebug("executed paths");

    }

void testXDFLParsing(VMPrototype *vm) {
    QDomDocument xdflDom;
    QString xdflFilePath="../../../media/testxdfl-mid.xdfl";
    // load the XDFL file into the DOM document
    {
      QFile xdflFile(xdflFilePath);
      if (!xdflFile.open(QFile::ReadOnly)) {
          qDebug("FAILED TO OPEN XDFL FILE");
          return;
      }
      xdflDom.setContent(&xdflFile);
      xdflFile.close();
    }

    XDFLHandler *handler = new XDFLHandler(vm,xdflDom);
//    handler->setVM(vm);
//    handler->loadFromDom(xdflDom);

    qDebug("Handler made; DOM loaded.");

    QMap<int,Material> testing = handler->getMaterials();
    vm->bays[0]->setMaterial(testing[1]);

    QTimer::singleShot(1,handler,SLOT(start()));
    QTimer::singleShot(5000, handler, SLOT(pause()));

    QTimer::singleShot(12000, handler, SLOT(resume()));

}
