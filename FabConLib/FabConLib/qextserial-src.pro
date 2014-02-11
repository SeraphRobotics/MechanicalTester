
# event driven device enumeration on windows requires the gui module
#!win32:QT               -= gui

DEPENDDIR              +=   JrkerrElectronics/Posix/qextserial/
INCLUDEDIR             +=   JrkerrElectronics/Posix/qextserial/ ../Common/Posix/JrKerr/tmp
HEADERS                 +=   JrkerrElectronics/Posix/qextserial/qextserialport.h \
                            JrkerrElectronics/Posix/qextserial/qextserialenumerator.h \
                            JrkerrElectronics/Posix/qextserial/qextserialport_global.h
SOURCES                 +=   JrkerrElectronics/Posix/qextserial/qextserialport.cpp

unix:SOURCES           +=   JrkerrElectronics/Posix/qextserial/posix_qextserialport.cpp
unix:!macx:SOURCES     +=   JrkerrElectronics/Posix/qextserial/qextserialenumerator_unix.cpp

macx {
  SOURCES          +=   JrkerrElectronics/Posix/qextserial/qextserialenumerator_osx.cpp
  LIBS             += -framework IOKit -framework CoreFoundation
}

