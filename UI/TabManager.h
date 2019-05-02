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
#include <QWebChannel>
#include "ElementModel.h"
#include "PostProcessor.h"
#include <math.h>
#include <QStandardPaths>




class TabManager : public QDialog
{
    Q_OBJECT
public:
    explicit TabManager(QWidget *parent = nullptr);
    TabManager(BonzaTableView *tableView,ElementModel *emodel,QWidget *parent = nullptr);
    void init(QTabWidget* theTab);
    void initGMTab();
    void fillMatTab(QString ,const QModelIndex &index);
    void cleanForm(QVector<QLineEdit*> currentEdts);
    void checkDefaultFEM(QString thisMatType,const QModelIndex &index);
    void setDefaultFEM(QString thisMatType,const QModelIndex &index);
    void fillFEMTab();
    void initFEMTab();
    void reFreshGMTab();
    void writeGM();
    void writeGMVintage();
    QString loadGMtoStringVintage();
    bool writeSurfaceMotion();
    QString loadGMtoString();
    QString loadMotions2String(QString);
    QString loadPWPResponse();
    QString loadruPWPResponse();
    QString loadNodeSa();
    QString loadEleResponse(QString);
    QString loadNodeResponse(QString);
    QTabWidget* getTab(){return tab;}
    void hideConfigure();
    QString openseespath(){return openseesPathStr;}
    QString rockmotionpath(){return GMPathStr;}
    void updateVelHtml();
    void updateDispHtml();
    void updateAccHtml();
    void updatePWPHtml();
    void updateRupwpHtml();
    void updateSaHtml();
    void updateStrainHtml();
    void updateStressHtml();
    void updateStressStrainHtml();
    void reFreshGMView(){GMView->show();}
    void setPM4SandToolTps();
    void updatePostProcessor(PostProcessor *postProcessort);
    void setGMViewLoaded(){GMViewLoaded = true;}
    QVector<QVector<double>> getElemResVec(QString);

signals:
    void configTabUpdated();


public:
    QString rootDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);//qApp->applicationDirPath();// QDir::currentPath();
public slots:
    void onTableViewClicked(const QModelIndex &index);
    void onTableViewUpdated(const QModelIndex&,const QModelIndex&);
    void onDataEdited();
    void onGMBtnClicked();
    //
    void onOpenseesBtnClicked();
    void onFEMTabEdited();
    double getGWTFromConfig();
    void onRunBtnClicked();
    void onOpenseesTextChanged(const QString&);
    void onGMTextChanged(const QString&);
    void onSecondaryBtnClicked(bool);
    void onElementDataChanged(QModelIndex,QModelIndex);
    void onTabBarClicked(int);
    void onGMLoadFinished(bool);
    void updateGMPath(QString);
    void updateOpenSeesPath(QString);
    void updateLayerTab(QJsonObject,QJsonObject);
    void onConfigTabEdtFinished();


private:
    QTabWidget* tab;
    int currentRow = -9;
    int currentCol = -9;
    QString currentMatType;
    BonzaTableView *tableView;
    BonzaTableModel *tableModel;
    ElementModel* elementModel;
    PostProcessor *postProcessor;

    //QFile uiFilePM4Sand;
    //QFile uiFileElasticIsotropic;

    QWidget* PM4SandWidget;
    QWidget* PM4SiltWidget;
    QWidget* PIMYWidget;
    QWidget* PDMYWidget;
    QWidget* PDMY02Widget;
    QWidget* ManzariDafaliasWidget;
    QWidget* ElasticIsotropicWidget;
    QWidget* defaultWidget;
    QWidget* GMWidget;
    QWidget* FEMWidget;

    QWebEngineView *GMView;
    QString GMPathStr;

    QWidget* currentWidget;
    QVector<QLineEdit*> currentEdts;

    QList<QString> listFEMtab = {"eSizeH", "eSizeV", "RockVs", "RockDen", "DashpotCoeff", "VisC", "GMPath", "openseesPath"  };
    int GMPathPos = 6;
    int OpenSeesPathPos = 7;


    QList<QString> listElasticIsotropicFEM = {"eSize", "EEdt", "vEdt", "rhoEdt", "DrEdt","voidEdt","hPermEdt","vPermEdt","rho_dEdt","rho_sEdt","uBulkEdt"};
    QList<QString> listPM4SandFEM = {"eSize", "Dr", "G0", "hpo", "Den",
                                      "P_atm", "h0", "emax", "emin", "nb", "nd", "Ado",
                                      "z_max", "cz", "ce", "phic", "nu", "cgd", "cdr",
                                      "ckaf", "Q", "R", "m", "Fsed_min", "p_sedo",
                                        "hPerm","vPerm","uBulk"};
    QList<QString> listPM4SiltFEM = {"eSize", "Dr", "S_u", "Su_Rat", "G_o", "h_po", "Den", "Su_factor", "P_atm",
                                      "nu", "nG", "h0", "eInit", "lambda", "phicv", "nb_wet", "nb_dry", "nd", "Ado", "ru_max", "z_max",
                                     "cz", "ce", "cgd", "ckaf", "m_m", "CG_consol",
                                        "hPerm","vPerm","uBulk"};
    QList<QString> listPIMYFEM = {"eSize", "Dr", "nd", "rho", "refShearModul", "refBulkModul", "cohesi", "peakShearStra",
                                      "frictionAng", "refPress", "pressDependCoe", "noYieldSurf",
                                        "hPerm","vPerm","uBulk"};
    QList<QString> listPDMYFEM = {"eSize", "Dr", "nd", "rho", "refShearModul", "refBulkModul", "frictionAng", "peakShearStra",
                                      "refPress", "pressDependCoe", "PTAng", "contrac", "dilat1", "dilat2", "liquefac1", "liquefac2", "liquefac3",
                                        "e", "cs1", "cs2", "cs3", "pa","c", "noYieldSurf",
                                        "hPerm","vPerm","uBulk"};
    QList<QString> listPDMY02FEM = {"eSize", "Dr", "nd", "rho", "refShearModul", "refBulkModul", "frictionAng", "peakShearStra",
         "refPress", "pressDependCoe", "PTAng", "contrac1","contrac3", "dilat1", "dilat3", "contrac2", "dilat2", "liquefac1", "liquefac2",
           "e", "cs1", "cs2", "cs3", "pa","c", "noYieldSurf",
           "hPerm","vPerm","uBulk"};
    QList<QString> listManzariDafaliasFEM = {"eSize", "Dr", "G0", "nu", "e_init", "Mc", "c", "lambda_c", "e0", "ksi", "P_atm", "m", "h0",
                                              "ch", "nb", "A0", "nd", "z_max", "cz", "Den",
                                        "hPerm","vPerm","uBulk"};
    QVector<QLineEdit*> edtsFEM;
    QVector<QLineEdit*> edtsElasticIsotropicFEM;
    QVector<QLineEdit*> edtsPM4SandFEM;
    QVector<QLineEdit*> edtsPM4SiltFEM;
    QVector<QLineEdit*> edtsPIMYFEM;
    QVector<QLineEdit*> edtsPDMYFEM;
    QVector<QLineEdit*> edtsPDMY02FEM;
    QVector<QLineEdit*> edtsManzariDafaliasFEM;

    QString thisMatType;
    QString GMTabHtmlName = QDir(rootDir).filePath("resources/ui/GroundMotion/index.html");
    QString accHtmlName = QDir(rootDir).filePath("resources/ui/GroundMotion/acc.html");
    QString dispHtmlName = QDir(rootDir).filePath("resources/ui/GroundMotion/disp.html");
    QString pwpHtmlName = QDir(rootDir).filePath("resources/ui/GroundMotion/pwp.html");
    QString rupwpHtmlName = QDir(rootDir).filePath("resources/ui/GroundMotion/rupwp.html");
    QString strainHtmlName = QDir(rootDir).filePath("resources/ui/GroundMotion/strain.html");
    QString stressHtmlName = QDir(rootDir).filePath("resources/ui/GroundMotion/stress.html");

    QString GMTabHtmlNameTmp = QDir(rootDir).filePath("resources/ui/GroundMotion/index-template.html");
    QString accHtmlNameTmp = QDir(rootDir).filePath("resources/ui/GroundMotion/acc-template.html");
    QString dispHtmlNameTmp = QDir(rootDir).filePath("resources/ui/GroundMotion/disp-template.html");
    QString pwpHtmlNameTmp = QDir(rootDir).filePath("resources/ui/GroundMotion/pwp-template.html");
    QString rupwpHtmlNameTmp = QDir(rootDir).filePath("resources/ui/GroundMotion/rupwp-template.html");
    QString strainHtmlNameTmp = QDir(rootDir).filePath("resources/ui/GroundMotion/strain-template.html");
    QString stressHtmlNameTmp = QDir(rootDir).filePath("resources/ui/GroundMotion/stress-template.html");


    QString openseesPathStr;
    //QString rockmotionpathStr;
    QString analysisName = "analysis";
    QString analysisDir = QDir(rootDir).filePath(analysisName);
    QString femFilename = QDir(analysisDir).filePath("configure.dat");
    QString srtFileName = QDir(analysisDir).filePath("SRT.json");

    bool GMViewLoaded = false;

    int maxStepToShow = 300;

    QVector<QVector<double>> m_vStress;


};

#endif // TABMANAGER_H
