SOURCES += \
    FabConLib/FabConLib/bay.cpp \
    FabConLib/FabConLib/xdflpath.cpp \
    FabConLib/FabConLib/xdflvoxel.cpp \
    FabConLib/FabConLib/material.cpp \
    FabConLib/FabConLib/xyzmotion.cpp \
    FabConLib/FabConLib/xdflhandler.cpp \
    FabConLib/FabConLib/virtualmachines.cpp \
    FabConLib/FabConLib/coreinterface.cpp \
    FabConLib/FabConLib/loadconfigthread.cpp \
    FabConLib/FabConLib/jsnpath.cpp \
    FabConLib/FabConLib/jscmotion.cpp \
    FabConLib/FabConLib/jsxyz.cpp \
    FabConLib/FabConLib/testing/util.cpp \
    FabConLib/FabConLib/testing/unittests.cpp \
    FabConLib/FabConLib/testing/testerforci.cpp \
    FabConLib/FabConLib/testing/jscitest.cpp \
    FabConLib/FabConLib/comportdetector.cpp

HEADERS += \
    FabConLib/FabConLib/bay.h \
    FabConLib/FabConLib/xdflpath.h \
    FabConLib/FabConLib/xdflvoxel.h \
    FabConLib/FabConLib/material.h \
    FabConLib/FabConLib/xyzmotion.h \
    FabConLib/FabConLib/xdflhandler.h \
    FabConLib/FabConLib/virtualmachines.h \
    FabConLib/FabConLib/coreinterface.h \
    FabConLib/FabConLib/loadconfigthread.h \
    FabConLib/FabConLib/jsnpath.h \
    FabConLib/FabConLib/jscmotion.h \
    FabConLib/FabConLib/jsxyz.h \
    FabConLib/FabConLib/testing/util.h \
    FabConLib/FabConLib/testing/unittests.h \
    FabConLib/FabConLib/testing/testerforci.h \
    FabConLib/FabConLib/testing/jscitest.h \
    FabConLib/FabConLib/comportdetector.h

INCLUDEPATH += JrkerrElectronics/
include("JrKerr_Electronics.pro")

OTHER_FILES += \
    media/testxdfl.xdfl \
    media/TestScript.js \
    media/testConfig.config \
    media/testxdfl-test.xdfl \
    media/testxdfl-mid.xdfl \
    media/JrKerr-Valve.config \
    media/JrKerr-Duel-deposition.config

