#-------------------------------------------------
#
# Project created by QtCreator 2016-07-29T09:55:37
#
#-------------------------------------------------

QT       += core
QT       += network
QT       -= gui

TARGET = BioAcqServer2016
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

INCLUDEPATH += /home/esaith/Libraries/OpenSSL_Compiled/include

LIBS += /home/esaith/Libraries/OpenSSL_Compiled/lib/libcrypto.so
LIBS += /home/esaith/Libraries/OpenSSL_Compiled/lib/libssl.so

SOURCES += main.cpp \
    server.cpp \
    client.cpp \
    task.cpp \
    shared.cpp \
    encrypto.cpp

HEADERS += \
    server.h \
    client.h \
    task.h \
    shared.h \
    encrypto.h

# -- Zip -- Library
# is this needed or libs folder will suffice
QUAZIPCODEDIR = "/home/esaith/Documents/MinorsProject/BiometricAcquistionServerApp/Zipper/Include"
ZLIBCODEDIR = "/home/esaith/Documents/MinorsProject/BiometricAcquistionServerApp/Zipper/Libraries"

unix{
  LIBS += -L$${ZLIBCODEDIR} -lz
}

win32{
  LIBS += -L$${ZLIBCODEDIR} -lzdll
}

INCLUDEPATH += $${QUAZIPCODEDIR}
HEADERS += $${QUAZIPCODEDIR}/*.h
SOURCES += $${QUAZIPCODEDIR}/*.cpp
SOURCES += $${QUAZIPCODEDIR}/*.c
