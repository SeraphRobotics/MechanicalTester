#include <QtGui/QApplication>
#include "mainwindow.h"
//#include "Interface.h"
#include <stdio.h>
#include <iostream>
#include <QDomDocument>
#include "scancontroller.h"
#include "mastercontrolunit.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // QSettings setup. These values will carry over in the rest of the application.
    QCoreApplication::setOrganizationName("Seraph");
    QCoreApplication::setOrganizationDomain("seraphrobotics.com");
    QCoreApplication::setApplicationName("SeraphScan");
    QSettings::setDefaultFormat(QSettings::IniFormat);
    // Default scope is QSettings::UserScope
    // This is the list of values the application is storing:
    //   config_dir

    MainWindow mw;

    mw.show();
    return a.exec();
}
