#-------------------------------------------------
#
#
#
#-------------------------------------------------

QT       += core gui sql quick qml webenginewidgets

CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SiteResponseTool
TEMPLATE = app

INCLUDEPATH += FEM \
            SiteResponse \




LIBS += -L"$$_PRO_FILE_PWD_/lib" -lFEM -lSRT \
-L/usr/local/Cellar/lapack/3.8.0_1/lib -lblas -llapack -llapacke
#-L/usr/lib -L/usr/local/lib -L/usr/local/Cellar/lapack/3.8.0_1/lib -lblas -llapack -llapacke -lm -ldl -lgfortran


SOURCES += main.cpp\
        UI/MainWindow.cpp \
    UI/DatabaseManager.cpp \
    UI/BonzaTableView.cpp \
    UI/InsertWindow.cpp \
    UI/BonzaTableModel.cpp \
    UI/SiteResponse.cpp


HEADERS  += UI/MainWindow.h \
    UI/DatabaseManager.h \
    GlobalConstances.h \
    UI/BonzaTableView.h \
    UI/InsertWindow.h \
    UI/BonzaTableModel.h \
    UI/SiteResponse.h

FORMS    += UI/MainWindow.ui \
    UI/InsertWindow.ui

RESOURCES += \
    resources.qrc

