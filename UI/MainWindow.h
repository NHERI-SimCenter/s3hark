#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWebEngineView>
#include "Mesher.h"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
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

    json createMaterial(int tag, std::string matType, std::string parameters);

    void onTotalLayerEdited();

    void refresh();

signals:
    void gwtChanged(const QString &newGWT);
    void tableMoved();

private:
    Ui::MainWindow *ui;

    int layerViewWidth = 200;
    int meshViewWidth = 200;
    int layerTableWidth = 630;
    int layerTableHeight = 500;//320;
public:
    QWidget *plotContainer;
    QWidget *matContainer;
    QWebEngineView *dinoView;
    Mesher* mesher;
};

#endif // MAINWINDOW_H
