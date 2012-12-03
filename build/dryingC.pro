TARGET = dryingC

HEADERS = ../src/DryingControl.h

SOURCES = ../src/DryingControl.cpp \
          ../src/main.cpp

INCLUDEPATH += ../../emHC/src
INCLUDEPATH += ../../emHC/src/alert
INCLUDEPATH += ../../emHC/src/ports

LIBS += -L../../emHC/build -lemHC

QT -= gui
QT += xml

QMAKE_POST_LINK += $$quote(cp $$TARGET ..)
