#-------------------------------------------------#
#                                                 #
#               Site Response Tool                #
#                                                 #
#-------------------------------------------------#

QT       += core gui sql quick qml webenginewidgets uitools

CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SiteResponseTool
TEMPLATE = app



# get system type
UNAME = $$system(uname -s)
contains( UNAME, [lL]inux  ):message( System type: $$UNAME )
contains( UNAME, [dD]arwin ):message( System type: $$UNAME )



INCLUDEPATH += FEM \
            SiteResponse \
            /usr/local/opt/lapack/include \
            /usr/local/include





LIBS += -L"$$_PRO_FILE_PWD_/lib" -lSRT -lFEM \
                /usr/local/opt/lapack/lib/libblas.3.8.0.dylib \
                /usr/local/opt/lapack/lib/liblapack.3.8.0.dylib \
                /usr/local/opt/lapack/lib/liblapacke.3.8.0.dylib \
#        -L/usr/local/opt/lapack/lib -lblas -llapack -llapacke





SOURCES += main.cpp\
        UI/MainWindow.cpp \
    UI/DatabaseManager.cpp \
    UI/BonzaTableView.cpp \
    UI/InsertWindow.cpp \
    UI/BonzaTableModel.cpp \
    UI/SiteResponse.cpp \
    UI/TabManager.cpp \
    SiteResponse/Mesher.cpp \
    UI/JsonManager.cpp \
    UI/ElementModel.cpp

HEADERS  += UI/MainWindow.h \
    UI/DatabaseManager.h \
    GlobalConstances.h \
    UI/BonzaTableView.h \
    UI/InsertWindow.h \
    UI/BonzaTableModel.h \
    UI/SiteResponse.h \
    UI/TabManager.h \
    SiteResponse/Mesher.h \
    UI/JsonManager.h \
    UI/ElementModel.h


FORMS    += UI/MainWindow.ui \
    UI/InsertWindow.ui \
    UI/PM4Sand.ui \
    UI/ElasticIsotropic.ui \
    UI/DefaultMatTab.ui \
    UI/GroundMotion.ui \
    UI/FEM.ui

RESOURCES += \
    resources.qrc




# build libs: FEM, SRT
message(building libs: SRT and FEM. This will take a while ...)
lib_compiled = $$system(make -j4)
contains(lib_compiled, FEMSRTCompiled){
message(SRT and FEM built successfully.)
}else{
message(==============================================>SRT and FEM fail to build.)
}
