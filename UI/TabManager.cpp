#include "TabManager.h"
#include <QDebug>
#include <QAbstractItemModel>
#include <QLineEdit>


TabManager::TabManager(QWidget *parent) : QDialog(parent)
{

}

TabManager::TabManager(BonzaTableView *tableViewIn,QWidget *parent) : QDialog(parent)
{
    tableView = tableViewIn;

    tableModel = tableView->m_sqlModel;


}

void TabManager::init(QTabWidget* theTab){
    this->tab = theTab;
    tab->setTabsClosable(true);

    QUiLoader uiLoader;
    QString uiFileName = ":/UI/DefaultMatTab.ui";
    QFile uiFile(uiFileName);
    uiFile.open(QIODevice::ReadOnly);
    // setWorkingDirectory: if uiFile depended on other resources,
    // setWorkingDirectory needs to be set here
    //const QDir &workdir(uifileWorkPath);
    //uiLoader.setWorkingDirectory(workdir);
    defaultWidget = uiLoader.load(&uiFile,this);
    tab->addTab(defaultWidget,"FEM");


    QFile uiFilePM4Sand(":/UI/PM4Sand.ui");
    uiFilePM4Sand.open(QIODevice::ReadOnly);
    PM4SandWidget = uiLoader.load(&uiFilePM4Sand,this);
    QFile uiFileElasticIsotropic(":/UI/ElasticIsotropic.ui");
    uiFileElasticIsotropic.open(QIODevice::ReadOnly);
    ElasticIsotropicWidget = uiLoader.load(&uiFileElasticIsotropic,this);


}

void TabManager::onTableViewClicked(const QModelIndex &index){
    //qDebug() << index.row() << " " << index.column();

    QString thisMatType = tableView->m_sqlModel->record(index.row()).value("MATERIAL").toString();


    currentRow = index.row();
    currentCol = index.column();

    //qDebug() << "Num of Tabs: " << tab->count();

    tab->removeTab(0);
    if (thisMatType=="Elastic")
        tab->insertTab(0,ElasticIsotropicWidget,"FEM");
    else if (thisMatType=="PM4Sand")
        tab->insertTab(0,PM4SandWidget,"FEM");
    else
        tab->insertTab(0,defaultWidget,"FEM");
    tab->setCurrentIndex(0);

    QList<QVariant> infos = tableView->getRowInfo(currentRow);
    if (infos.size()>0)
    {   qDebug() << infos;
        qDebug() <<"Mat " << infos.at(MATERIAL-2).toString();
    }

    fillFEMTab(thisMatType, index);



}

void TabManager::onTableViewUpdated(const QModelIndex& index1,const QModelIndex& index2){
    onTableViewClicked(index1);
}

void TabManager::fillFEMTab(QString thisMatType,const QModelIndex &index){
    if(thisMatType == "Elastic")
    {
        QString FEMString = tableModel->record(index.row()).value("FEM").toString();
        QLineEdit* EEdt = ElasticIsotropicWidget->findChild<QLineEdit*>("EEdt");
        EEdt->setText(FEMString);

        //tableModel->setData(tableModel->index(index.row(), FEM), "100");
    }
}
