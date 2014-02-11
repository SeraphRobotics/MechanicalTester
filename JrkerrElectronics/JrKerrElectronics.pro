# -------------------------------------------------
# Project created by QtCreator 2011-06-10T00:48:58
# -------------------------------------------------
DEFINES += CONFIRM_ENABLED \
    FatHmBrd
QT += xml
QT -= gui
TARGET = TestMotors
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app
SOURCES += main.cpp \
    electronicsinterface.cpp \
    motor.cpp \
    coordinatedmotion.cpp \
    npath.cpp \
    nmotion.cpp 
HEADERS += electronicsinterface.h \
    motor.h \
    coordinatedmotion.h \
    npath.h \
    nmotion.h 


win32: { 
    include("win-src.pro")
    INCLUDEPATH += ./Windows
}
unix: { 
    include("posix-src.pro")
    INCLUDEPATH += ./Posix
    include("qextserial-src.pro")
}
OTHER_FILES += testConfig.config \
    JrKerr-m3.config \
    hobby-servo.config
