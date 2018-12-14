#include "TabManager.h"
#include <QDebug>
#include <QAbstractItemModel>
#include <QLineEdit>
#include <QFileInfo>
#include <QMessageBox>
#include <QPushButton>


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


    QString uiFEMName = ":/UI/FEM.ui";
    QFile uiFEMFile(uiFEMName);
    uiFEMFile.open(QIODevice::ReadOnly);
    FEMWidget = uiLoader.load(&uiFEMFile,this);
    tab->addTab(FEMWidget,"FEM");
    initFEMTab();
    connect(FEMWidget->findChild<QPushButton*>("GMBtn"), SIGNAL(clicked()), this, SLOT(onGMBtnClicked()));


    QString uiFileName = ":/UI/DefaultMatTab.ui";
    QFile uiFile(uiFileName);
    uiFile.open(QIODevice::ReadOnly);
    defaultWidget = uiLoader.load(&uiFile,this);
    tab->addTab(defaultWidget,"Material");



    GMView = new QWebEngineView(this);
    //GMView->load(QUrl("file:////Users/simcenter/Codes/SimCenter/SiteResponseTool/resources/ui/GroundMotion/index.html"));
    GMView->load(QUrl::fromLocalFile(QFileInfo("resources/ui/GroundMotion/index.html").absoluteFilePath()));
    tab->addTab(GMView,"Ground motion");




    /*
    QFile uiFileGM(":/UI/GroundMotion.ui");
    uiFileGM.open(QIODevice::ReadOnly);
    GMWidget = uiLoader.load(&uiFileGM,this);
    tab->insertTab(1,GMWidget,"Ground motion");
    */



    QFile uiFilePM4Sand(":/UI/PM4Sand.ui");
    uiFilePM4Sand.open(QIODevice::ReadOnly);
    PM4SandWidget = uiLoader.load(&uiFilePM4Sand,this);
    for (int i = 0; i < listPM4SandFEM.size(); ++i) {
        QString edtName = listPM4SandFEM[i] ;
        edtsPM4SandFEM.push_back(PM4SandWidget->findChild<QLineEdit*>(edtName));
    }
    // connect edit signal with onDataEdited
    for (int i = 0; i < edtsPM4SandFEM.size(); ++i) {
        connect(edtsPM4SandFEM[i], SIGNAL(editingFinished()), this, SLOT(onDataEdited()));
    }






    QFile uiFileElasticIsotropic(":/UI/ElasticIsotropic.ui");
    uiFileElasticIsotropic.open(QIODevice::ReadOnly);
    ElasticIsotropicWidget = uiLoader.load(&uiFileElasticIsotropic,this);
    for (int i = 0; i < listElasticIsotropicFEM.size(); ++i) {
        QString edtName = listElasticIsotropicFEM[i] ;
        edtsElasticIsotropicFEM.push_back(ElasticIsotropicWidget->findChild<QLineEdit*>(edtName));
    }
    // connect edit signal with onDataEdited
    for (int i = 0; i < edtsElasticIsotropicFEM.size(); ++i) {
        connect(edtsElasticIsotropicFEM[i], SIGNAL(editingFinished()), this, SLOT(onDataEdited()));
    }



}

void TabManager::onGMBtnClicked()
{
    qDebug() << "GM btn clicked. ";
    QString file_name = QFileDialog::getOpenFileName(NULL,"Choose Ground Motion File",".","*");
    FEMWidget->findChild<QLineEdit*>("GMPath")->setText(file_name);

}

void TabManager::onFEMTabEdited()
{
    QString filename = "FEM.dat";
    QFile file(filename);
    if(file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text)) {
            QTextStream stream(&file);
            for (int i = 0; i < edtsFEM.size(); i++) {
                stream<< listFEMtab[i] << ","<<" "<<edtsFEM[i]->text() << endl;
            }
            stream<< "GWT" << ","<<" "<<tableView->getGWT() << endl;
            file.close();
    }
}

double TabManager::getGWTFromConfig()
{
    double GWT=0.0;
    QString filename = "FEM.dat";
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly)) {
        file.close();
        return GWT;
    }
    file.open(QIODevice::ReadOnly);
    QTextStream in(&file);

    while(!in.atEnd()) {
        QString line = in.readLine();
        QStringList thisLine = line.split(",");
        if(!thisLine[0].trimmed().compare("GWT"))
            GWT = thisLine[1].trimmed().toDouble();
    }
    file.close();
    return GWT;

}


void TabManager::initFEMTab(){


    for (int i = 0; i < listFEMtab.size(); ++i) {
        QString edtName = listFEMtab[i] ;
        edtsFEM.push_back(FEMWidget->findChild<QLineEdit*>(edtName));
    }
    // connect edit signal with onDataEdited
    for (int i = 0; i < edtsFEM.size(); ++i) {
        connect(edtsFEM[i], SIGNAL(editingFinished()), this, SLOT(onFEMTabEdited()));
    }


    fillFEMTab();
}

void TabManager::fillFEMTab(){

    QString filename = "FEM.dat";
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly)) {
        //QMessageBox::information(nullptr, "error", file.errorString());

        QFile fileNew(filename);
        if (fileNew.open(QIODevice::ReadWrite)) {
            QTextStream stream(&fileNew);
            double eleThickness = 1.0;
            double eSizeH = 0.25;
            double eSizeV = 0.25;
            double RockVs = 180.0;
            double RockDen = 2.0;
            double DashpotCoeff = RockVs * RockDen;
            double VisC = eSizeH * eleThickness * DashpotCoeff;
            stream << "eSizeH,"<<" "<<eSizeH << endl;
            stream << "eSizeV,"<<" "<<eSizeV << endl;
            stream << "RockVs,"<<" "<<RockVs << endl;
            stream << "RockDen,"<<" "<<RockDen << endl;
            stream << "DashpotCoeff,"<<" "<<DashpotCoeff << endl;
            stream << "VisC,"<<" "<<VisC << endl;
            stream << "GMPath,"<<" "<<"Input the path of a ground motion file. " << endl;
            stream << "GWT,"<<" "<<"0.0" << endl;
            fileNew.close();
            file.open(QIODevice::ReadOnly);
        }
    }

    QTextStream in(&file);

    QStringList savedPars;

    while(!in.atEnd()) {
        QString line = in.readLine();
        QStringList thisLine = line.split(",");
        savedPars.append(thisLine[1].trimmed());
    }

    file.close();

    for (int i = 0; i < edtsFEM.size(); i++) {
        edtsFEM[i]->setText(savedPars.at(i));
    }
}

void TabManager::onTableViewClicked(const QModelIndex &index){
    //qDebug() << index.row() << " " << index.column();

    thisMatType = tableView->m_sqlModel->record(index.row()).value("MATERIAL").toString();

    currentRow = index.row();
    currentCol = index.column();




    if (thisMatType=="Elastic")
    {
        currentEdts = edtsElasticIsotropicFEM;
        currentWidget = ElasticIsotropicWidget;
    }
    else if (thisMatType=="PM4Sand")
    {
        currentEdts = edtsPM4SandFEM;
        currentWidget = PM4SandWidget;
    }
    else
        currentWidget = defaultWidget;


    for (int j=tab->count();j>0;j--)
        tab->removeTab(j-1);
    tab->insertTab(0,FEMWidget,"FEM");
    tab->insertTab(1,currentWidget,"Material");
    tab->insertTab(2,GMView,"Ground motion");
    tab->setCurrentIndex(1);

    QList<QVariant> infos = tableView->getRowInfo(currentRow);
    if (infos.size()>0)
    {   qDebug() << infos;
        qDebug() <<"Mat " << infos.at(MATERIAL-2).toString();
    }

    fillMatTab(thisMatType, index);

}

void TabManager::onTableViewUpdated(const QModelIndex& index1,const QModelIndex& index2){
    onTableViewClicked(index1);
}

void TabManager::fillMatTab(QString thisMatType,const QModelIndex &index){

    checkDefaultFEM(thisMatType, index);

    QString FEMString = tableModel->record(index.row()).value("FEM").toString();
    QStringList FEMStringList = FEMString.split(" ", QString::SkipEmptyParts);

    cleanForm(currentEdts);


    // set values for the form
    QString thisFEmString;
    if (FEMStringList.size() == currentEdts.size())
    {
        for (int i = 0; i < FEMStringList.size(); ++i) {
            currentEdts[i]->setText(FEMStringList.at(i));
            qDebug() << FEMStringList.at(i);
        }

        // for Elastic, check the density are the same as shown in the soil layer table
        if (thisMatType == "Elastic")
        {
            QString densityFromTable = tableModel->record(index.row()).value("DENSITY").toString();

            QLineEdit* DenEdt = ElasticIsotropicWidget->findChild<QLineEdit*>("rhoEdt");
            QString densityFromForm = DenEdt->text();

            if(densityFromTable != densityFromForm)
            {
                qDebug() << "Den here is different from the above table. ";
                if (densityFromTable == "")
                {
                    tableModel->setData(tableModel->index(index.row(), DENSITY), densityFromForm);
                    DenEdt->setText(densityFromForm);
                }
                else
                {
                    DenEdt->setText(densityFromTable);
                }
            }
        }

        // for PM4Sand, check the density are the same as shown in the soil layer table
        if (thisMatType == "PM4Sand")
        {
            QString densityFromTable = tableModel->record(index.row()).value("DENSITY").toString();

            QLineEdit* DenEdt = PM4SandWidget->findChild<QLineEdit*>("Den");
            QString densityFromForm = DenEdt->text();

            if(densityFromTable != densityFromForm)
            {
                qDebug() << "Den here is different from the above table. ";
                if (densityFromTable == "")
                {
                    tableModel->setData(tableModel->index(index.row(), DENSITY), densityFromForm);
                    DenEdt->setText(densityFromForm);
                }
                else
                {
                    DenEdt->setText(densityFromTable);
                }
            }
        }

    } else
    {
        qDebug() << "FEMStringList.size() not == edts.size() !";
    }



}

void TabManager::onDataEdited()
{
    qDebug() << "edited. I feel that.";

    if (thisMatType=="Elastic")
        currentEdts = edtsElasticIsotropicFEM;
    else if (thisMatType=="PM4Sand")
        currentEdts = edtsPM4SandFEM;

    // collect data in the form
    QString thisFEmString;
    for (int i = 0; i < currentEdts.size(); ++i) {
        thisFEmString += currentEdts[i]->text()+" ";
    }
    // update the model
    tableModel->setData(tableModel->index(currentRow, FEM), thisFEmString);

    // update the Density in table
    if (thisMatType=="Elastic")
    {
        QLineEdit* DenEdt = ElasticIsotropicWidget->findChild<QLineEdit*>("rhoEdt");
        tableModel->setData(tableModel->index(currentRow, DENSITY), DenEdt->text());

    }
    if (thisMatType=="PM4Sand")
    {
        QLineEdit* DenEdt = PM4SandWidget->findChild<QLineEdit*>("Den");
        tableModel->setData(tableModel->index(currentRow, DENSITY), DenEdt->text());
    }


}

void TabManager::cleanForm(QVector<QLineEdit*> currentEdts)
{
    for (int i = 0; i < currentEdts.size(); ++i) {
        currentEdts[i]->clear();
    }
}


void TabManager::checkDefaultFEM(QString thisMatType,const QModelIndex &index)
{
    QString FEMString = tableModel->record(index.row()).value("FEM").toString();
    QStringList FEMStringList = FEMString.split(" ", QString::SkipEmptyParts);

    int numPars;
    if (thisMatType == "Elastic")
        numPars = 4;
    else if (thisMatType == "PM4Sand")
        numPars = 25;
    else
        numPars =0;

    if (FEMStringList.size() != numPars)
        setDefaultFEM(thisMatType, index);

}

void TabManager::setDefaultFEM(QString thisMatType,const QModelIndex &index)
{// TODO: set values based on basic soil parameters

    if (thisMatType == "Elastic")
    {
        double rho = tableModel->record(index.row()).value("DENSITY").toDouble();
        if (rho<1.0e-11 & rho>-1.0e-11) //
        {
            rho = 2.0; // set default Vs
            tableModel->setData(tableModel->index(index.row(), DENSITY), QString::number(rho));
        }

        double vs = tableModel->record(index.row()).value("VS").toDouble();
        if (vs<1.0e-11 & vs>-1.0e-11) //
        {
            vs = 182.0; // set default Vs
            tableModel->setData(tableModel->index(index.row(), VS), QString::number(vs));
        }
        double E = 2.0 * rho * vs * vs * (1. + .3);
        tableModel->setData(tableModel->index(currentRow, FEM), " 2.0 " + QString::number(E)+" 0.3 "+QString::number(rho));
    }
    else if (thisMatType == "PM4Sand")
    {
        QString density = tableModel->record(index.row()).value("DENSITY").toString();
        if (density=="")
            density = "2.0";
        tableModel->setData(tableModel->index(currentRow, FEM), "2.0 0.47 500.0 0.45 "+ density +" 101.3 -1. 0.8 0.5 0.5 0.1 -1. -1. 250 -1. 33.0 0.3 2.0 -1. -1. 10. 1.5 0.01 -1. -1.");
        //  "1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24"
    }

}

