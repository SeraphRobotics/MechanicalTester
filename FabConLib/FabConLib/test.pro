# -------------------------------------------------
# Project created by QtCreator 2011-06-10T00:48:58
# -------------------------------------------------
DEFINES += CONFIRM_ENABLED
QT += core \
    script \
    xml

TARGET = TestMotors
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app
SOURCES += main.cpp \
    bay.cpp \
    xdflpath.cpp \
    xdflvoxel.cpp \
    material.cpp \
    xyzmotion.cpp \
    xdflhandler.cpp \
    virtualmachines.cpp \
    coreinterface.cpp \
    loadconfigthread.cpp \
    jsnpath.cpp \
    jscmotion.cpp \
    jsxyz.cpp \
    testing/util.cpp \
    testing/unittests.cpp \
    testing/testerforci.cpp \
    testing/jscitest.cpp

HEADERS += bay.h \
    xdflpath.h \
    xdflvoxel.h \
    material.h \
    xyzmotion.h \
    xdflhandler.h \
    virtualmachines.h \
    coreinterface.h \
    loadconfigthread.h \
    jsnpath.h \
    jscmotion.h \
    jsxyz.h \
    testing/util.h \
    testing/unittests.h \
    testing/testerforci.h \
    testing/jscitest.h

INCLUDEPATH += JrkerrElectronics/
include("JrKerr_Electronics.pro")

OTHER_FILES += \
    media/testxdfl.xdfl \
    media/TestScript.js \
    media/testConfig.config \
    media/testxdfl-test.xdfl \
    media/testxdfl-mid.xdfl \
    media/JrKerr-Valve.config \
    media/JrKerr-Single-deposition.config \
    media/JrKerr-Duel-deposition.config













































