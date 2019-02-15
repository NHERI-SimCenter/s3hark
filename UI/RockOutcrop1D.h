#ifndef ROCK_OUTCROP_1D_H
#define ROCK_OUTCROP_1D_H

#include <QWidget>
#include <QWebEngineView>
#include <QQuickView>
#include "Mesher.h"
#include "ElementModel.h"
#include <QProcess>
#include "TabManager.h"
#include "ProfileManager.h"
#include "PostProcessor.h"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace Ui {
class RockOutcrop1D;
}



class RockOutcrop1D : public QWidget
{
    Q_OBJECT

public:
    explicit RockOutcrop1D(QWidget *parent = nullptr);
    ~RockOutcrop1D();
    void updateCtrl();


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
    void on_meshBtn_clicked(bool checked);

    void on_thickness_edited();

    void on_gwtEdit_textChanged(const QString &arg1);

    void on_rowRemoved(int row);





    void on_reBtn_clicked();
    void on_runBtn_clicked();

    json createMaterial(int tag, std::string matType, std::string parameters);

    void onTotalLayerEdited();

    void refresh();

    ElementModel* getElementModel()const;
    void onOpenSeesFinished();

    void hideShowTab();

    void writeSurfaceMotion();

signals:
    void gwtChanged(const QString &newGWT);
    void tableMoved();
    void runBtnClicked(QWebEngineView*);

private:
    Ui::RockOutcrop1D *ui;

    int layerViewWidth = 200;
    int meshViewWidth = 200;
    int layerTableWidth = 630;
    int layerTableHeight = 500;//320;
    int openseesErrCount = 0;

private:// some of them were public
    QWidget *plotContainer;
    QWidget *matContainer;
    QWebEngineView *dinoView;
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


};



#endif // ROCK_OUTCROP_1D_H
