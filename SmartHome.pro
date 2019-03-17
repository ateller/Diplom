#-------------------------------------------------
#
# Project created by QtCreator 2018-05-25T13:44:27
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SmartHome
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
        widget.cpp \
    relation.cpp \
    sensor.cpp \
    effector.cpp \
    device.cpp \
    imitation.cpp \
    control.cpp \
    mape_loop.cpp \
    knowledge.cpp \
    rule_editing.cpp \
    rule_text.cpp

HEADERS += \
        widget.h \
    relation.h \
    sensor.h \
    effector.h \
    device.h \
    imitation.h \
    control.h \
    mape_loop.h \
    knowledge.h \
    rules.h \
    rule_editing.h \
    rule_text.h

FORMS += \
        widget.ui \
    control.ui \
    rule_editing.ui \
    rule_text.ui
