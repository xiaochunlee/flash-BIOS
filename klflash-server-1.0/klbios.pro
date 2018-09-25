#-------------------------------------------------
#
# Project created by QtCreator 2012-02-09T15:14:56
#
#-------------------------------------------------

QT       += core gui\
            sql

TARGET   = klflash
TEMPLATE = app
LIBS    +=  -lflash


SOURCES += main.cpp\
    logindlg.cpp \
    mainwindow.cpp

HEADERS  += \
    logindlg.h \
    connection.h \
    mainwindow.h\
    basefunc.h \
    flashopt.h \
    smbios.h \
    data.h \
    bios_app.h

FORMS    += \
    logindlg.ui \
    mainwindow.ui

RESOURCES += \
    menu.qrc

