# event driven device enumeration on windows requires the gui module
# !win32:QT               -= gui
win32:{
DEPENDDIR  += ./Windows
INCLUDEDIR += ./Windows
HEADERS    += ./Windows/sio_util.h \
    ./Windows/picstep.h \
    ./Windows/picservo.h \
    ./Windows/picio.h \
    ./Windows/path.h \
    ./Windows/nmccom.h
SOURCES    += ./Windows/sio_util.cpp \
    ./Windows/picstep.cpp \
    ./Windows/picservo.cpp \
    ./Windows/picio.cpp \
    ./Windows/path.cpp \
    ./Windows/nmccom.cpp
}