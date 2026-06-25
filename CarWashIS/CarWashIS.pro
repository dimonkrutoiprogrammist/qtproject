# CarWashIS.pro
QT += core widgets sql printsupport

CONFIG += c++17

TARGET = CarWashIS
TEMPLATE = app

SOURCES += \
    main.cpp \
    databasemanager.cpp \
    loginwindow.cpp \
    mainwindow.cpp \
    clientform.cpp \
    orderform.cpp \
    serviceform.cpp \
    employeeform.cpp \
    reportform.cpp

HEADERS += \
    databasemanager.h \
    loginwindow.h \
    mainwindow.h \
    clientform.h \
    orderform.h \
    serviceform.h \
    employeeform.h \
    reportform.h


# RESOURCES += \
#     resources.qrc

# win32:RC_ICONS = app.ico
