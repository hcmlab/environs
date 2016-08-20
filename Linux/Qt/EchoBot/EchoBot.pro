TEMPLATE = app
CONFIG += console
QMAKE_CXXFLAGS += -std=gnu++11
CONFIG -= app_bundle
CONFIG -= qt
INCLUDEPATH += ../../../Common
LIBS += -ldl -lpthread
QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter

SOURCES += \
    ../../../Windows/Echo.Bot.CPP/Echo.Bot.CPP.cpp \
    ../../../Common/Environs.Loader.cpp \
    ../../../Common/Interop/Threads.cpp

HEADERS += \
    ../../../Windows/Echo.Bot.CPP/Echo.Bot.CPP.h \
    ../../../Common/Environs.Loader.h \
    ../../../Common/Environs.Native.h \
    ../../../Common/Interop/Threads.h \
    ../../../Common/Interop/Threads.Int.h
