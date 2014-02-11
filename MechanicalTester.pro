#-------------------------------------------------
#
# Project created by QtCreator 2011-10-24T13:56:22
#
#-------------------------------------------------

QT += \
    core \
    gui \
    xml \
    script

TARGET = MechanicalTester
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    scancontroller.cpp \
    mastercontrolunit.cpp 

HEADERS += \
    mainwindow.h \
    scancontroller.h \
    mastercontrolunit.h 


FORMS += \
    mainwindow.ui


INCLUDEPATH += FabConLib
include(FabConLib/FabConLib/include.pro)


#RESOURCES += fabprint.qrc
OTHER_FILES += SeraphScanner.config


