SOURCES  +=   JrkerrElectronics/Windows/qextserial/qextserialenumerator.cpp\
              JrkerrElectronics/Windows/qextserial/qextserialport_win.cpp \
              JrkerrElectronics/Windows/qextserial/qextserialenumerator_win.cpp\
              JrkerrElectronics/Windows/qextserial/qextserialport.cpp


HEADERS  +=   JrkerrElectronics/Windows/qextserial/qextserialport.h \
              JrkerrElectronics/Windows/qextserial/qextserialport_p.h \
              JrkerrElectronics/Windows/qextserial/qextserialenumerator.h \
              JrkerrElectronics/Windows/qextserial/qextserialenumerator_p.h \
              JrkerrElectronics/Windows/qextserial/qextserialport_global.h



# For Windows user who doesn't have Qt's Private files
#win32:!exists($$[QT_INSTALL_HEADERS]/QtCore/private/qwineventnotifier_p.h){
        DEFINES            += QESP_NO_QT_PRIVATE
        HEADERS            +=   JrkerrElectronics/Windows/qextserial/qextwineventnotifier_p.h
        SOURCES            +=   JrkerrElectronics/Windows/qextserial/qextwineventnotifier_p.cpp
#}
win32:LIBS             += -lsetupapi -ladvapi32 -luser32
