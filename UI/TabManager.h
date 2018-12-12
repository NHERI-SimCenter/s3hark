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

class TabManager : public QDialog
{
    Q_OBJECT
public:
    explicit TabManager(QWidget *parent = nullptr);
    TabManager(BonzaTableView *tableView,QWidget *parent = nullptr);
    void init(QTabWidget* theTab);
    void fillFEMTab(QString ,const QModelIndex &index);
    void cleanForm(QVector<QLineEdit*> currentEdts);
    void checkDefaultFEM(QString thisMatType,const QModelIndex &index);
    void setDefaultFEM(QString thisMatType,const QModelIndex &index);

signals:

public slots:
    void onTableViewClicked(const QModelIndex &index);
    void onTableViewUpdated(const QModelIndex&,const QModelIndex&);
    void onDataEdited();

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

    QWebEngineView *GMView;

    QWidget* currentWidget;
    QVector<QLineEdit*> currentEdts;

    QList<QString> listElasticIsotropicFEM = { "EEdt", "vEdt", "rhoEdt" };
    QList<QString> listPM4SandFEM = { "Dr", "G0", "hpo", "Den",
                                      "P_atm", "h0", "emax", "emin", "nb", "nd", "Ado",
                                      "z_max", "cz", "ce", "phic", "nu", "cgd", "cdr",
                                      "ckaf", "Q", "R", "m", "Fsed_min", "p_sedo"};

    QVector<QLineEdit*> edtsElasticIsotropicFEM;
    QVector<QLineEdit*> edtsPM4SandFEM;

    QString thisMatType;


};

#endif // TABMANAGER_H
