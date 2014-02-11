# event driven device enumeration on Posix requires the gui module
!win32: { 
    QT += gui
    DEPENDDIR += ./Posix/
    INCLUDEDIR += ./Posix/
    HEADERS += ./Posix/picstep.h \
        ./Posix/picservo.h \
        ./Posix/picio.h \
        ./Posix/path.h \
        ./Posix/nmccom.h
    SOURCES += ./Posix/picstep.cpp \
        ./Posix/picservo.cpp \
        ./Posix/picio.cpp \
        ./Posix/path.cpp \
        ./Posix/nmccom.cpp
}
