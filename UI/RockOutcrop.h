#ifndef ROCK_OUTCROP_H
#define ROCK_OUTCROP_H

#include <QWidget>
#include <QWebEngineView>
#include <QQuickView>
#include "Mesher.h"
#include "ElementModel.h"
#include <QProcess>
#include "TabManager.h"
#include "ProfileManager.h"
#include "PostProcessor.h"
#include "SimCenterAppWidget.h"
#include "SiteResponse.h"
#include "SSSharkThread.h"
#include <QStandardPaths>
#include <QCheckBox>
#include <QDesktopServices>
#include "GoogleAnalytics.h"

using namespace std::placeholders;

#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace Ui {
class RockOutcrop;
}


class RockOutcrop : public SimCenterAppWidget
{
    Q_OBJECT

public:
    explicit RockOutcrop(QWidget *parent = nullptr);
    ~RockOutcrop();
    void updateCtrl();
    void loadFromJson();

    // SimCenterWidget interface
    bool outputToJSON(QJsonObject &rvObject);
    bool inputFromJSON(QJsonObject &rvObject);
    bool inputFromJSON_old(QJsonObject &rvObject);
    bool outputAppDataToJSON(QJsonObject &rvObject);
    bool inputAppDataFromJSON(QJsonObject &rvObject);
    bool copyFiles(QString &destDir);

    // callback setups
    bool refreshRun(double step);
    std::function<bool(double)> m_callbackptr = std::bind(&RockOutcrop::refreshRun,this, std::placeholders::_1);

    bool copyDir(const QDir& from, const QDir& to, bool cover) ;
    void cleanTable();

    void updateMesh(json &j);

    int checkDimension();

public slots:

    void onThicknessEdited();
    void onConfigTabUpdated();
    void onInternalFEAInvoked();
    void onInternalFEAUpdated(double step){refreshRun(step);}

private slots:

    void insert();
    void insertAbove();
    void insertBelow();
    void insertWithDialog();
    void remove();
    void gotoPage(int index);
    void nextPage();
    void prevPage();

    void totalHeightChanged();

    void on_delRowBtn_clicked();

    void on_addRowBtn_clicked();

    void on_meshBtn_clicked();

    void on_gwtEdit_textChanged(const QString &arg1);

    void on_gwtEdit_editingFinished();

    void on_rowRemoved(int row);

    void on_reBtn_clicked();

    void on_killBtn_clicked();

    void on_runBtn_clicked();

    json createMaterial(int tag, std::string matType, std::string parameters);

    void onTotalLayerEdited();

    void refresh();

    ElementModel* getElementModel()const;

    void onOpenSeesFinished();

    void hideShowTab();
    void showShowTab();

    void writeSurfaceMotion();

signals:
    void gwtChanged(const QString &newGWT);
    void tableMoved();
    void runBtnClicked();
    void signalProgress(int);
    void signalInvokeInternalFEA();
    void releaseShark();

private:
    Ui::RockOutcrop *ui;

    int layerViewWidth = 200;
    int meshViewWidth = 200;
    int layerTableWidth = 630;
    int layerTableHeight = 500;//320;
    int openseesErrCount = 0;
    int currentTabID;

private:// some of them were public
    QWidget *plotContainer;
    QWidget *matContainer;
    Mesher* mesher;
    QQuickView *meshView;
    //QQuickView *pgaView;
    ElementModel* elementModel;
    QProcess* openseesProcess;
    TabManager* theTabManager;
    QTabWidget* resultsTab;
    ProfileManager* profiler;
    PostProcessor* postProcessor;

    double maxPGA;

    QThread *workerThread;

    QString dimMsg = "";

    bool loadPreviousResults = true;

    SSSharkThread *shark;

 public:
    QString rootDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation); // qApp->applicationDirPath();//
    QString analysisDir = QDir(rootDir).filePath("analysis");
    QString tclName = QDir(analysisDir).filePath("model.tcl");
    QString outputDir = QDir(analysisDir).filePath("out_tcl");
    QString srtFileName = QDir(analysisDir).filePath("SRT.json");
    QString evtjFileName =  QDir(analysisDir).filePath("EVENT-SRT.json");
    QString femLog = QDir(analysisDir).filePath("fem.log");



};



#endif // ROCK_OUTCROP_H
