#ifndef TABMANAGER_H
#define TABMANAGER_H

#include <QObject>
#include <QDialog>
#include <QTabWidget>

#include <QUiLoader>
#include <QFile>
#include <QModelIndex>
#include "BonzaTableView.h"

class TabManager : public QDialog
{
    Q_OBJECT
public:
    explicit TabManager(QWidget *parent = nullptr);
    TabManager(BonzaTableView *tableView,QWidget *parent = nullptr);
    void init(QTabWidget* theTab);
    void fillFEMTab(QString ,const QModelIndex &index);

signals:

public slots:
    void onTableViewClicked(const QModelIndex &index);
    void onTableViewUpdated(const QModelIndex&,const QModelIndex&);

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

};

#endif // TABMANAGER_H
