TARGET = VASTChallenge
TEMPLATE = app

CONFIG *= qt
CONFIG *= release #debug_and_release
CONFIG *= thread warn_off

QT *= xml opengl

## -------------------------------------------------
## depending upon the platform, set appropriate vals
## -------------------------------------------------
macx {
    OS = macx
    DEFINES *= MAC_OSX
    CONFIG -= app_bundle
}
else {
    OS = unix
    DEFINES *= LINUX
}

CONFIG *= staticlib


MOC_DIR = $$PWD/build/
OBJECTS_DIR = $$PWD/build/
UI_DIR = $$PWD/build/
RCC_DIR = $$PWD/build/
DESTDIR = $$PWD/build/

message($$DESTDIR)

## -------------------------------------------------
## specify external libraries and paths to be used
## -------------------------------------------------

# -- static linking
staticlib {

    # QGLViewer (as library)
    INCLUDEPATH *= $$(HOME)/usr/include
    LIBS *= $$(HOME)/usr/lib/libQGLViewer.a
}

# -- dynamic linking
else {

    # QGLViewer (as framework)
    #QMAKE_LFLAGS += -F$$(HOME)/Library/Frameworks
    #LIBS += -framework QGLViewer
    #INCLUDEPATH *= $$(HOME)/Library/Frameworks/QGLViewer.Framework/Headers

    # QGLViewer (as library)
    INCLUDEPATH *= $$(HOME)/usr/include
    LIBS *= -L$$(HOME)/usr/lib -lQGLViewer
}
# -----------------------------------------

INCLUDEPATH *= ./include

FORMS *= ./VASTChallenge.ui

HEADERS = ./include/VASTApp.h \
          ./include/Park.h

SOURCES = ./src/VASTViewer.cpp \
          ./src/Park.cpp \
          ./src/VASTApp.cpp
