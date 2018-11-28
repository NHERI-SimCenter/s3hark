#-------------------------------------------------
#
# Project created by QtCreator 2016-02-17T19:23:56
#
#-------------------------------------------------

## common compile issues
#
# -- Windows 10
#
#   * missing lapack.lib and/or blas.lib:
#
#     download and install Intel MKL (math kernel library).
#     it is free from intel's web site, easy to install, and efficient.
#
#   * LNK1104: cannot open file 'qwt.lib'
#
#     - in Qt Creator, open existing project PileGroupTool\qwt-6.2\qwt.pro
#     - configure the kit to compile in "[your_path]\build-qwt-[....]" (the full path is important)
#     - adjust the path in the LIBS line below to reflect your build directory
#     - in Qt Creator, PileGroupTool, rerun QMake and Build
#
# -- MacOS
#
#   * cannot open file 'qwt.lib'
#
#     first time compile only:
#     - download qwt source
#     - qmake qwt.pro
#     - make BUILD=debug
#     - make install
#     - make BUILD=release
#     - make install
#
#     now, and with future updates, work as usual:
#     - return to Qt Creator and Build the PileGroupTool.
#
# -- Linux
#

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport network

TARGET   = SiteResponseTool
TEMPLATE = app
VERSION  = 0.0.1

PRODUCT_NAME = 'SiteResponseTool'

DEFINES += APP_VERSION=\\\"$$VERSION\\\"

unix:  QMAKE_CXXFLAGS_WARN_ON = -Wall -Wno-unused-variable -std=c++11
win32: QMAKE_CXXFLAGS += /Y-

unix: DEPENDPATH += /usr/local/qwt-6.2.0-svn
win32: DEPENDPATH += C:\Qwt-6.1.3

win32: include(C:\qwt-6.1.3\features\qwt.prf)
unix: include(/usr/local/qwt-6.2.0-svn/features/qwt.prf)

include(OPS_includes.pro)

INCLUDEPATH += includes
INCLUDEPATH += mainWindow
INCLUDEPATH += dialogs
INCLUDEPATH += plots
INCLUDEPATH += FEA

unix: QT += svg

SOURCES += main.cpp\
        mainWindow/mainwindow.cpp \
        qcp/qcustomplot.cpp \
        FEA/getPyParam.cpp \
        FEA/getQzParam.cpp \
        FEA/getTZParam.cpp \
        FEA/soilmat.cpp \
        FEA/pilefeamodeler.cpp \
        dialogs/materialdbinterface.cpp \
        utilWindows/copyrightdialog.cpp \
        utilWindows/dialogabout.cpp \
        utilWindows/dialogpreferences.cpp \
        utilWindows/dialogfuturefeature.cpp \
        plots/systemplotsuper.cpp \
        plots/systemplotqcp.cpp \
        plots/systemplotqwt.cpp \
        plots/resultplotsuper.cpp \
        plots/resultplotqcp.cpp \
        plots/resultplotqwt.cpp \
        ../SimCenterCommon/Common/FooterWidget.cpp \
        ../SimCenterCommon/Common/HeaderWidget.cpp
        
HEADERS  += \
        mainWindow/mainwindow.h \
        includes/pilegrouptool_parameters.h \
        qcp/qcustomplot.h \
        FEA/soilmat.h \
        FEA/pilefeamodeler.h \
        dialogs/materialdbinterface.h \
        utilWindows/copyrightdialog.h \
        utilWindows/dialogabout.h \
        utilWindows/dialogpreferences.h \
        utilWindows/dialogfuturefeature.h \
        plots/systemplotsuper.h \
        plots/systemplotqcp.h \
        plots/systemplotqwt.h \
        plots/resultplotsuper.h \
        plots/resultplotqcp.h \
        plots/resultplotqwt.h \
        ../SimCenterCommon/Common/FooterWidget.h \
        ../SimCenterCommon/Common/HeaderWidget.h

FORMS    += mainWindow/mainwindow.ui \
        dialogs/materialdbinterface.ui \
        utilWindows/copyrightdialog.ui \
        utilWindows/dialogabout.ui \
        utilWindows/dialogpreferences.ui \
        utilWindows/dialogfuturefeature.ui

RESOURCES += \
	qtpileresources.qrc

DISTFILES +=
