QT += core gui sql
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

TARGET = AirborneInventory
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    databasemanager.cpp \
    partsmodel.cpp \
    addeditpartdialog.cpp

HEADERS += \
    mainwindow.h \
    databasemanager.h \
    part.h \
    partsmodel.h \
    addeditpartdialog.h

DISTFILES += \
    ../../Downloads/style.qss

RESOURCES += \
    ../../Downloads/resources.qrc
