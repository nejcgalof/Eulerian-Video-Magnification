#-------------------------------------------------
#
# Project created by QtCreator 2016-05-27T20:31:23
#
#-------------------------------------------------

QT       += core gui

QT_CONFIG -= no-pkg-config
CONFIG += link_pkgconfig

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = excutable
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    VideoProcessor.cpp \
    SpatialFilter.cpp \
    MagnifyDialog.cpp

HEADERS  += mainwindow.h \
    VideoProcessor.h \
    SpatialFilter.h \
    MagnifyDialog.h

FORMS    += mainwindow.ui \
    MagnifyDialog.ui

RESOURCES += \
    myResources.qrc

win32 {
    message("* Using settings for Windows.")

    INCLUDEPATH += C://opencv//release//install//include//opencv2
    INCLUDEPATH += C://opencv//release//install//include//opencv
    INCLUDEPATH += C://opencv//release//install//include

    LIBS += C://opencv//release//lib//*.a
}
