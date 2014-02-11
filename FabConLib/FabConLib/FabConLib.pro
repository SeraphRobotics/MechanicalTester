# -------------------------------------------------
# Project created by QtCreator 2011-06-10T00:48:58
# -------------------------------------------------
QT += network \
    sql \
    script \
    xml \
    xmlpatterns \
    dbus
QT -= gui
TARGET = FabConLib
TEMPLATE = lib
DEFINES += FABCONLIB_LIBRARY
INCLUDEPATH += ../../../../
SOURCES += fabconlib.cpp \
    virtualmachineenviroment.cpp \
    motor.cpp \
    electronicsinterface.cpp \
    coordinatedmotion.cpp\
    path.cpp \
    materialcalibration.cpp \
    materialsetting.cpp \
    tool.cpp \
    bay.cpp \
    main.cpp
HEADERS += fabconlib.h \
    FabConLib_global.h \
    virtualmachineenviroment.h \
    motor.h \
    electronicsinterface.h \
    coordinatedmotion.h\
    path.h \
    materialcalibration.h \
    materialsetting.h \
    tool.h \
    bay.h
win32: {
    include("win-src.pro")
    INCLUDEPATH += ../../Common/Windows/src
}
unix:{
 include("posix-src.pro")
 INCLUDEPATH += ../../Common/Posix/JrKerr
 include("qextserial-src.pro")
}

include("NPath.pro")
include("shared-math.pro")
OTHER_FILES += ../../../media/IdealConfig.config
