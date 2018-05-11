#-------------------------------------------------
#
# Project created by QtCreator 2018-05-03T23:16:03
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ProjectImage
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    checker.cpp \
    Libs/kcf/fhog.cpp \
    Libs/kcf/kcftracker.cpp

HEADERS += \
        mainwindow.h \
    checker.h \
    Libs/kcf/ffttools.hpp \
    Libs/kcf/fhog.hpp \
    Libs/kcf/kcftracker.hpp \
    Libs/kcf/labdata.hpp \
    Libs/kcf/recttools.hpp \
    Libs/kcf/tracker.h

FORMS += \
        mainwindow.ui

RESOURCES += \
    resources.qrc

RC_ICONS += favicon.ico

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/Libs/opencv/x64/vc15/lib/ -lopencv_world340
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/Libs/opencv/x64/vc15/lib/ -lopencv_world340d

INCLUDEPATH += $$PWD/Libs/opencv/include
DEPENDPATH += $$PWD/Libs/opencv/include



