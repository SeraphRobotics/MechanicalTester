# event driven device enumeration on Posix requires the gui module
!win32: { 
    QT += gui
    DEPENDDIR +=   JrkerrElectronics/Posix/
    INCLUDEDIR +=   JrkerrElectronics/Posix/
    HEADERS +=   JrkerrElectronics/Posix/picstep.h \
          JrkerrElectronics/Posix/picservo.h \
          JrkerrElectronics/Posix/picio.h \
          JrkerrElectronics/Posix/path.h \
          JrkerrElectronics/Posix/nmccom.h
    SOURCES +=   JrkerrElectronics/Posix/picstep.cpp \
          JrkerrElectronics/Posix/picservo.cpp \
          JrkerrElectronics/Posix/picio.cpp \
          JrkerrElectronics/Posix/path.cpp \
          JrkerrElectronics/Posix/nmccom.cpp
}
