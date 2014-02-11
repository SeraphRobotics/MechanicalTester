# -------------------------------------------------
# Project created by QtCreator 2011-06-10T00:48:58
# -------------------------------------------------
SOURCES += \
    JrkerrElectronics/electronicsinterface.cpp \
    JrkerrElectronics/motor.cpp \
    JrkerrElectronics/coordinatedmotion.cpp \
    JrkerrElectronics/npath.cpp \
    JrkerrElectronics/nmotion.cpp \

HEADERS += \
    JrkerrElectronics/electronicsinterface.h \
    JrkerrElectronics/motor.h \
    JrkerrElectronics/coordinatedmotion.h \
    JrkerrElectronics/npath.h \
    JrkerrElectronics/nmotion.h \

win32: {
    include("win-src.pro")
    INCLUDEPATH += JrkerrElectronics/Windows/
    include("qextserial-win.pro")
    INCLUDEPATH += JrkerrElectronics/Windows/qextserial/
}
unix: {
    include("posix-src.pro")
    INCLUDEPATH += JrkerrElectronics/Posix/
    include("qextserial-src.pro")
    INCLUDEPATH += JrkerrElectronics/Posix/qextserial/
}
