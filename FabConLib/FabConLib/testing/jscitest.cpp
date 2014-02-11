#include "jscitest.h"
#include <QDebug>

JsCiTest::JsCiTest(){
    ci_ = new CoreInterface();
}

QString JsCiTest::loadXDFL(){
    QDomDocument xdflDom;
    QString xdflFilePath="../../../media/testxdfl-mid.xdfl";
    // load the XDFL file into the DOM document
    {
      QFile xdflFile(xdflFilePath);
      if (!xdflFile.open(QFile::ReadOnly)) {
          qDebug("FAILED TO OPEN XDFL FILE\n");
          return QString();
      }
      xdflDom.setContent(&xdflFile);
      xdflFile.close();
    }
    return xdflDom.toString();
}

void JsCiTest::test(){


    connect(ci_,SIGNAL(stateChaged(int)),this,SLOT(connected(int)));

    qDebug() << "attemping to make a connection...";
    QString configFilePath="../../../media/JrKerr-Single-deposition.config";
    QDomDocument document;
    {
      QFile configFile(configFilePath);
      if (!configFile.open(QFile::ReadOnly)) {
          qDebug() << "FAILED TO OPEN CONFIG FILE\n";
          return;
      }
      document.setContent(&configFile);
      configFile.close();
    }
    ci_->setConfig(document.toString(),"COM6");
}

void JsCiTest::connected(int i){
    qDebug("STATE IS %i",i);
    if(i==CoreInterface::Connected){
        //////////////Object is connected///////////

        QTextStream sstream(&script_,QIODevice::WriteOnly);
//        sstream<<"\n"<<"ci.move(-50,0,0,80);";
        sstream<<"\n"<<"ci.setXDFL(xdflstring);";
        sstream<<"\n"<<"ci.setMaterial(0,1);";
        sstream<<"\n"<<"";

        // Add CI object to engine
        QScriptValue scriptCI = engine_.newQObject(ci_);
        engine_.globalObject().setProperty("ci",scriptCI);

        // Add XDFLString to engine
        QString xdflstring = loadXDFL();
        QScriptValue scriptXDFL = QScriptValue(&engine_,xdflstring);
        engine_.globalObject().setProperty("xdflstring",scriptXDFL);

        runScript();


    }else if(i==CoreInterface::FileLoaded){
        /// Tells the printer to start the print job is has via Javascript
        qDebug("\nFile Ready");
        script_.clear();
        QTextStream sstream(&script_,QIODevice::WriteOnly);
        sstream<<"\n"<<"ci.startPrint()";
        sstream<<"\n"<<"";
        runScript();

    }else if(i==CoreInterface::Printing){
        qDebug("\nPrinting");
        QTimer::singleShot(2000,this,SLOT(pause()));
        QTimer::singleShot(12000,this,SLOT(resume()));




    }else{
        qDebug("\nUNKNOWN STATE is %i",i);
    }
}

void JsCiTest::pause(){
    qDebug("\nPAUSING");
    script_.clear();
    QTextStream sstream(&script_,QIODevice::WriteOnly);
    sstream<<"\n"<<"ci.pausePrint()";
    sstream<<"\n"<<"";
    runScript();
    qDebug("\nPAUSED");
}
void JsCiTest::resume(){
    qDebug("\nResuming");
    script_.clear();
    QTextStream sstream(&script_,QIODevice::WriteOnly);
    sstream<<"\n"<<"ci.resumePrint()";
    sstream<<"\n"<<"";
    runScript();
    qDebug("\nresumed");
}


void JsCiTest::runScript(){
    QString error="";
    QTextStream ss(&error,QIODevice::WriteOnly);
    // Check the syntax
    if (engine_.checkSyntax(script_).state()==QScriptSyntaxCheckResult::Error) {
       ss<<engine_.checkSyntax(script_).errorMessage();
    }

    // evaluate script
    engine_.evaluate(script_);

    // check for errors
    if (engine_.hasUncaughtException()) {
       ss<<engine_.uncaughtException().toString();
    }
    qDebug("\nERRORS:%s",error.toStdString().c_str());
}
