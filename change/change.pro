#-------------------------------------------------
#
# Project created by QtCreator 2017-01-06T14:36:25
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = change
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
LIBS+=-I usr/include/libxml2/libxml -lxml2
LIBS+=-I /usr/local/lib/libpcap -lpcap
LIBS+=-I /lib/libpci9820 -lpci9820

SOURCES += main.cpp \
    CRCCheckSum.cpp \
    CSoftBus.cpp \
    HardwareAdapter.cpp \
    HardwareCanbus.cpp \
    HardwareEthernet.cpp \
    HardwareSerial.cpp \
    ProtocolAnalysis.cpp \
    ReceiveWindow.cpp \
    SoftBusMsg.cpp \
    StrategyPriority.cpp \
    StrategyRepeat.cpp \
    TransmitNode.cpp \
    TransRecModel.cpp \
    XMLAnalyse.cpp

HEADERS += \
    conditioner_com.h \
    CRCCheckSum.h \
    CSoftBus.h \
    FrameHead.h \
    HardwareAdapter.h \
    HardwareCANbus.h \
    HardwareEthernet.h \
    HardwareSerial.h \
    ProtocolAnalysis.h \
    ReceiveWindow.h \
    SoftBusMsg.h \
    softBusSystem.h \
    StrategyPriority.h \
    StrategyRepeat.h \
    TransmitNode.h \
    TransRecModel.h \
    XMLAnalyse.h
