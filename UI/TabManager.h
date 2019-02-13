#ifndef TABMANAGER_H
#define TABMANAGER_H

#include <QObject>
#include <QDialog>
#include <QTabWidget>

#include <QUiLoader>
#include <QFile>
#include <QModelIndex>
#include "BonzaTableView.h"
#include <QWebEngineView>
#include <QFileDialog>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFileInfo>



class TabManager : public QDialog
{
    Q_OBJECT
public:
    explicit TabManager(QWidget *parent = nullptr);
    TabManager(BonzaTableView *tableView,QWidget *parent = nullptr);
    void init(QTabWidget* theTab);
    void fillMatTab(QString ,const QModelIndex &index);
    void cleanForm(QVector<QLineEdit*> currentEdts);
    void checkDefaultFEM(QString thisMatType,const QModelIndex &index);
    void setDefaultFEM(QString thisMatType,const QModelIndex &index);
    void fillFEMTab();
    void initFEMTab();
    void reFreshGMTab();
    void writeGM();
    bool writeSurfaceMotion();
    QString loadGMtoString();
    QTabWidget* getTab(){return tab;}
    void hideConfigure();
    QString openseespath(){return openseesPathStr;}
    void updateDispHtml();
    void updateAccHtml();

signals:

public slots:
    void onTableViewClicked(const QModelIndex &index);
    void onTableViewUpdated(const QModelIndex&,const QModelIndex&);
    void onDataEdited();
    void onGMBtnClicked();
    void onOpenseesBtnClicked();
    void onFEMTabEdited();
    double getGWTFromConfig();
    void onRunBtnClicked(QWebEngineView*);
    void onOpenseesTextChanged(const QString&);
    void onGMTextChanged(const QString&);
    void onSecondaryBtnClicked(bool);


private:
    QTabWidget* tab;
    int currentRow = -9;
    int currentCol = -9;
    QString currentMatType;
    BonzaTableView *tableView;
    BonzaTableModel *tableModel;

    QFile uiFilePM4Sand;
    QFile uiFileElasticIsotropic;

    QWidget* PM4SandWidget;
    QWidget* ElasticIsotropicWidget;
    QWidget* defaultWidget;
    QWidget* GMWidget;
    QWidget* FEMWidget;

    QWebEngineView *GMView;
    QString GMPathStr;

    QWidget* currentWidget;
    QVector<QLineEdit*> currentEdts;

    QList<QString> listFEMtab = {"eSizeH", "eSizeV", "RockVs", "RockDen", "DashpotCoeff", "VisC", "GMPath", "openseesPath"  };


    QList<QString> listElasticIsotropicFEM = {"eSize", "EEdt", "vEdt", "rhoEdt", "DrEdt","voidEdt","hPermEdt","vPermEdt","rho_dEdt","rho_sEdt","uBulkEdt"};
    QList<QString> listPM4SandFEM = {"eSize", "Dr", "G0", "hpo", "Den",
                                      "P_atm", "h0", "emax", "emin", "nb", "nd", "Ado",
                                      "z_max", "cz", "ce", "phic", "nu", "cgd", "cdr",
                                      "ckaf", "Q", "R", "m", "Fsed_min", "p_sedo",
                                        "hPerm","vPerm","uBulk"};

    QVector<QLineEdit*> edtsFEM;
    QVector<QLineEdit*> edtsElasticIsotropicFEM;
    QVector<QLineEdit*> edtsPM4SandFEM;

    QString thisMatType;
    QString GMTabHtmlName = "resources/ui/GroundMotion/index.html";
    QString accHtmlName = "resources/ui/GroundMotion/acc.html";
    QString dispHtmlName = "resources/ui/GroundMotion/disp.html";
    QString openseesPathStr;




};

#endif // TABMANAGER_H
