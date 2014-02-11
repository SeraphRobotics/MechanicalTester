
# event driven device enumeration on windows requires the gui module
#!win32:QT               -= gui

DEPENDDIR              += ./Posix/qextserial/
INCLUDEDIR             += ./Posix/qextserial/ ../Common/Posix/JrKerr/tmp
HEADERS                 += ./Posix/qextserial/qextserialport.h \
                          ./Posix/qextserial/qextserialenumerator.h \
                          ./Posix/qextserial/qextserialport_global.h
SOURCES                 += ./Posix/qextserial/qextserialport.cpp

unix:SOURCES           += ./Posix/qextserial/posix_qextserialport.cpp
unix:!macx:SOURCES     += ./Posix/qextserial/qextserialenumerator_unix.cpp

macx {
  SOURCES          += ./Posix/qextserial/qextserialenumerator_osx.cpp
  LIBS             += -framework IOKit -framework CoreFoundation
}

