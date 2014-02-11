# event driven device enumeration on windows requires the gui module
# !win32:QT               -= gui
win32:{
DEPENDDIR  +=   JrkerrElectronics/Windows
INCLUDEDIR +=   JrkerrElectronics/Windows
HEADERS    +=   JrkerrElectronics/Windows/sio_util.h \
      JrkerrElectronics/Windows/picstep.h \
      JrkerrElectronics/Windows/picservo.h \
      JrkerrElectronics/Windows/picio.h \
      JrkerrElectronics/Windows/path.h \
      JrkerrElectronics/Windows/nmccom.h
SOURCES    +=   JrkerrElectronics/Windows/sio_util.cpp \
      JrkerrElectronics/Windows/picstep.cpp \
      JrkerrElectronics/Windows/picservo.cpp \
      JrkerrElectronics/Windows/picio.cpp \
      JrkerrElectronics/Windows/path.cpp \
      JrkerrElectronics/Windows/nmccom.cpp
}
