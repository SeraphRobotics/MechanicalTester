
# event driven device enumeration on windows requires the gui module
!win32:QT               -= gui

OBJECTS_DIR             = ../Common/JrKerr/src/tmp
MOC_DIR                 = ../Common/JrKerr/src/tmp
DEPENDDIR               = ../Common/JrKerr/src/
INCLUDEDIR              = ../Common/JrKerr/src/
HEADERS                 = ../Common/JrKerr/src/qextserialport.h \
                          ../Common/JrKerr/src/qextserialenumerator.h \
                          ../Common/JrKerr/src/qextserialport_global.h
SOURCES                 = ../Common/JrKerr/src/qextserialport.cpp

unix:SOURCES           += ../Common/JrKerr/src/posix_qextserialport.cpp
unix:!macx:SOURCES     += ../Common/JrKerr/src/qextserialenumerator_unix.cpp
macx {
  SOURCES          += ../Common/JrKerr/src/qextserialenumerator_osx.cpp
  LIBS             += -framework IOKit -framework CoreFoundation
}

win32 {
  SOURCES          += ../Common/JrKerr/src/win_qextserialport.cpp ../Common/JrKerr/src/qextserialenumerator_win.cpp
  DEFINES          += WINVER=0x0501 # needed for mingw to pull in appropriate dbt business...probably a better way to do this
  LIBS             += -lsetupapi
}
