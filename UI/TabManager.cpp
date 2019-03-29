#include "TabManager.h"
#include <QDebug>
#include <QAbstractItemModel>
#include <QLineEdit>
#include <QFileInfo>
#include <QMessageBox>
#include <QPushButton>
#include <QFileInfo>
#include <QFile>
#include <QLabel>



TabManager::TabManager(QWidget *parent) : QDialog(parent)
{

}

TabManager::TabManager(BonzaTableView *tableViewIn, ElementModel *emodel,QWidget *parent) : QDialog(parent),elementModel(emodel)
{
    tableView = tableViewIn;

    tableModel = tableView->m_sqlModel;

    if(!QDir(analysisDir).exists())
        QDir().mkdir(analysisDir);

}

void TabManager::onTabBarClicked(int tabinx)
{

    if(tabinx==2 && !GMViewLoaded)
    {
        GMView->reload();
        GMViewLoaded = true;
    }
    //emit onTabBarClickedFinished();
    //elementModel->reSetActive();
    //emit GMView->loadFinished(true);
}
void TabManager::onGMLoadFinished(bool ok)
{
    //elementModel->reSetActive();
}

void TabManager::onElementDataChanged(QModelIndex,QModelIndex)
{
    //tab->setCurrentIndex(2);
    //GMView->reload();
    //emit elementModel->activeIDChanged(1);
}

void TabManager::init(QTabWidget* theTab){
    this->tab = theTab;
    //tab->setTabsClosable(true);


    QUiLoader uiLoader;


    QString uiFEMName = ":/UI/FEM.ui";
    QFile uiFEMFile(uiFEMName);
    uiFEMFile.open(QIODevice::ReadOnly);
    FEMWidget = uiLoader.load(&uiFEMFile,this);
    hideConfigure();
    tab->addTab(FEMWidget,"Configure");

    initFEMTab();
    connect(FEMWidget->findChild<QPushButton*>("GMBtn"), SIGNAL(clicked()), this, SLOT(onGMBtnClicked()));
    connect(FEMWidget->findChild<QPushButton*>("openseesBtn"), SIGNAL(clicked()), this, SLOT(onOpenseesBtnClicked()));
    connect(FEMWidget->findChild<QLineEdit*>("openseesPath"), SIGNAL(textChanged(const QString&)), this, SLOT(onOpenseesTextChanged(const QString&)));
    connect(FEMWidget->findChild<QLineEdit*>("GMPath"), SIGNAL(textChanged(const QString&)), this, SLOT(onGMTextChanged(const QString&)));
    connect(FEMWidget->findChild<QLineEdit*>("openseesPath"), SIGNAL(editingFinished()), this, SLOT(onConfigTabEdtFinished()));
    connect(FEMWidget->findChild<QLineEdit*>("GMPath"), SIGNAL(editingFinished()), this, SLOT(onConfigTabEdtFinished()));



    QString uiFileName = ":/UI/DefaultMatTab.ui";
    QFile uiFile(uiFileName);
    uiFile.open(QIODevice::ReadOnly);
    defaultWidget = uiLoader.load(&uiFile,this);
    tab->addTab(defaultWidget,"Layer properties");


    // load ground motion view from html
    GMView = new QWebEngineView(this);
    //GMView->load(QUrl("file:////Users/simcenter/Codes/SimCenter/SiteResponseTool/resources/ui/GroundMotion/index.html"));
    QString GMTabHtmlName_true = QDir(rootDir).filePath(GMTabHtmlName);
    //GMView->load(QUrl::fromLocalFile(QFileInfo(GMTabHtmlName_true).absoluteFilePath()));
    QWebChannel *pWebChannel   = new QWebChannel(GMView->page());
    //TInteractObj *pInteractObj = new TInteractObj(this);
    pWebChannel->registerObject(QStringLiteral("elementModel"), elementModel);
    GMView->page()->setWebChannel(pWebChannel);
    GMView->page()->load(QUrl::fromLocalFile(QFileInfo(GMTabHtmlName_true).absoluteFilePath()));
    //GMView->show();

    //GMView->reload();

    tab->addTab(GMView,"Response");


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
    QLineEdit *eSizeEdtTmp= PM4SandWidget->findChild<QLineEdit*>("eSize");
    eSizeEdtTmp->hide();
    QLabel *eSizeLabelTmp= PM4SandWidget->findChild<QLabel*>("eSizeLabel");
    eSizeLabelTmp->hide();
    QPushButton *secondaryBtn= PM4SandWidget->findChild<QPushButton*>("secondaryBtn");
    secondaryBtn->setCheckable(true);
    secondaryBtn->hide();
    connect(secondaryBtn, SIGNAL(clicked(bool)), this, SLOT(onSecondaryBtnClicked(bool)));




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
    eSizeEdtTmp= ElasticIsotropicWidget->findChild<QLineEdit*>("eSize");
    eSizeEdtTmp->hide();
    eSizeLabelTmp= ElasticIsotropicWidget->findChild<QLabel*>("eSizeLabel");
    eSizeLabelTmp->hide();
    QLineEdit *rhoEdtTmp= ElasticIsotropicWidget->findChild<QLineEdit*>("rhoEdt");
    rhoEdtTmp->hide();
    QLabel *rhoLabelTmp= ElasticIsotropicWidget->findChild<QLabel*>("rhoLabel");
    rhoLabelTmp->hide();

    QLineEdit *voidEdtTmp= ElasticIsotropicWidget->findChild<QLineEdit*>("voidEdt");
    voidEdtTmp->hide();
    QLabel *voidLabelTmp= ElasticIsotropicWidget->findChild<QLabel*>("voidLabel");
    voidLabelTmp->hide();

    QLineEdit *DrEdtTmp= ElasticIsotropicWidget->findChild<QLineEdit*>("DrEdt");
    DrEdtTmp->hide();
    QLabel *DrLabelTmp= ElasticIsotropicWidget->findChild<QLabel*>("DrLabel");
    DrLabelTmp->hide();

    QLineEdit *rho_sEdtTmp= ElasticIsotropicWidget->findChild<QLineEdit*>("rho_sEdt");
    rho_sEdtTmp->hide();
    QLabel *rho_sLabelTmp= ElasticIsotropicWidget->findChild<QLabel*>("rho_sLabel");
    rho_sLabelTmp->hide();

    QLineEdit *rho_dEdtTmp= ElasticIsotropicWidget->findChild<QLineEdit*>("rho_dEdt");
    rho_dEdtTmp->hide();
    QLabel *rho_dLabelTmp= ElasticIsotropicWidget->findChild<QLabel*>("rho_dLabel");
    rho_dLabelTmp->hide();

    // adding tooltips
    QLabel *ELabel= ElasticIsotropicWidget->findChild<QLabel*>("ELabel");
    ELabel->setToolTip("kPa");
    QLabel *vLabel= ElasticIsotropicWidget->findChild<QLabel*>("vLabel");
    vLabel->setToolTip("Poisson's ratio");
    QLabel *hPermLabel= ElasticIsotropicWidget->findChild<QLabel*>("hPermLabel");
    hPermLabel->setToolTip("");
    QLabel *vPermLabel= ElasticIsotropicWidget->findChild<QLabel*>("vPermLabel");
    vPermLabel->setToolTip("");
    QLabel *uBulkLabel= ElasticIsotropicWidget->findChild<QLabel*>("uBulkLabel");
    uBulkLabel->setToolTip("kPa");



    //reFreshGMTab();
    initGMTab();


    //connect(elementModel,SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(onElementDataChanged(QModelIndex,QModelIndex)));
    connect(tab,SIGNAL(tabBarClicked(int)), this, SLOT(onTabBarClicked(int)));
    connect(GMView,SIGNAL(loadFinished(bool)), this, SLOT(onGMLoadFinished(bool)));

    setPM4SandToolTps();


}

void TabManager::setPM4SandToolTps()
{

    // adding tooltips

    QLabel *G0Label= PM4SandWidget->findChild<QLabel*>("G0_2");
    G0Label->setToolTip("kPa");
    QLabel *DenLabel= PM4SandWidget->findChild<QLabel*>("Den_2");
    DenLabel->setToolTip("Mg/m^3");

    QLabel *patmLabel= PM4SandWidget->findChild<QLabel*>("P_atm_2");
    patmLabel->setToolTip("kPa");
    QLabel *phicLabel= PM4SandWidget->findChild<QLabel*>("phic_2");
    phicLabel->setToolTip("deg");

    QLabel *hPermLabel= PM4SandWidget->findChild<QLabel*>("hPerm_2");
    hPermLabel->setToolTip("");
    QLabel *vPermLabel= PM4SandWidget->findChild<QLabel*>("vPerm_2");
    vPermLabel->setToolTip("");
    QLabel *uBulkLabel= PM4SandWidget->findChild<QLabel*>("uBulk_2");
    uBulkLabel->setToolTip("kPa");

}

void TabManager::onSecondaryBtnClicked(bool checked)
{
    qDebug()<<"secondary btn clicked: "<<checked;
    QList<QString> secondaryEdtNameList = {"P_atm", "h0", "emax", "emin", "nb", "nd", "Ado",
                                      "z_max", "cz", "ce", "phic", "nu", "cgd", "cdr",
                                      "ckaf", "Q", "R", "m", "Fsed_min", "p_sedo",
                                        "hPerm","vPerm","uBulk"};
    QList<QString> secondaryLabelNameList = {"P_atm_2", "h0_2", "emax_2", "emin_2", "nb_2", "nd_2", "Ado_2",
                                      "z_max_2", "cz_2", "ce_2", "phic_2", "nu_2", "cgd_2", "cdr_2",
                                      "ckaf_2", "Q_2", "R_2", "m_2", "Fsed_min_2", "p_sedo_2",
                                        "hPerm_2","vPerm_2","uBulk_2"};
    QList<QString> hydEdtNameList = {"hPerm","vPerm","uBulk"};
    QList<QString> hydLabelNameList = {"hPerm_2","vPerm_2","uBulk_2","line2"};

    QVector<QLineEdit*> secondaryEtds;
    for (int i = 0; i < secondaryEdtNameList.size(); ++i) {
        QString edtName = secondaryEdtNameList[i] ;
        secondaryEtds.push_back(ElasticIsotropicWidget->findChild<QLineEdit*>(edtName));
    }

    QVector<QLabel*> secondaryLabels;
    for (int i = 0; i < secondaryEdtNameList.size(); ++i) {
        QString edtName = secondaryEdtNameList[i] ;
        secondaryLabels.push_back(ElasticIsotropicWidget->findChild<QLabel*>(edtName));
    }

    if (checked)
    {
        for (int i = 0; i < secondaryEtds.size(); ++i) {
            secondaryEtds[i]->hide();
        }
        /*
        for (int i = 0; i < secondaryLabels.size(); ++i) {
            secondaryLabels[i]->setVisible(true);
        }
        */
    }else{
        /*
        for (int i = 0; i < secondaryEtds.size(); ++i) {
            secondaryEtds[i]->setVisible(false);
        }
        */
        /*
        for (int i = 0; i < secondaryLabels.size(); ++i) {
            secondaryLabels[i]->setVisible(false);
        }
        */

    }



}

void TabManager::hideConfigure()
{
    QLineEdit *eSizeH= FEMWidget->findChild<QLineEdit*>("eSizeH");
    eSizeH->hide();
    QLineEdit *eSizeV= FEMWidget->findChild<QLineEdit*>("eSizeV");
    eSizeV->hide();
    QLabel *eSizeHLabel= FEMWidget->findChild<QLabel*>("eSizeHLabel");
    eSizeHLabel->hide();
    QLabel *eSizeVLabel= FEMWidget->findChild<QLabel*>("eSizeVLabel");
    eSizeVLabel->hide();

    QLabel *pBaseLabel= FEMWidget->findChild<QLabel*>("pBaseLabel");
    pBaseLabel->hide();

    QLabel *RockVsLabel= FEMWidget->findChild<QLabel*>("RockVsLabel");
    RockVsLabel->hide();
    QLabel *RockDenLabel= FEMWidget->findChild<QLabel*>("RockDenLabel");
    RockDenLabel->hide();
    QLabel *coeffLabel= FEMWidget->findChild<QLabel*>("coeffLabel");
    coeffLabel->hide();
    QLabel *dampLabel= FEMWidget->findChild<QLabel*>("dampLabel");
    dampLabel->hide();

    QLineEdit *RockVs= FEMWidget->findChild<QLineEdit*>("RockVs");
    RockVs->hide();
    QLineEdit *RockDen= FEMWidget->findChild<QLineEdit*>("RockDen");
    RockDen->hide();
    QLineEdit *DashpotCoeff= FEMWidget->findChild<QLineEdit*>("DashpotCoeff");
    DashpotCoeff->hide();
    QLineEdit *VisC= FEMWidget->findChild<QLineEdit*>("VisC");
    VisC->hide();


}

void TabManager::onGMBtnClicked()
{
    qDebug() << "GM btn clicked. ";
    QString file_name = QFileDialog::getOpenFileName(this,"Choose Ground Motion File",".","*");
    FEMWidget->findChild<QLineEdit*>("GMPath")->setText(file_name);
    //writeGM();

    onFEMTabEdited();

}
void TabManager::onOpenseesBtnClicked()
{
    qDebug() << "OpenSees btn clicked. ";
    QString file_name = QFileDialog::getOpenFileName(this,"Choose opensees excutable",".","*");
    FEMWidget->findChild<QLineEdit*>("openseesPath")->setText(file_name);

    onFEMTabEdited();

}


void TabManager::onOpenseesTextChanged(const QString& text)
{
    openseesPathStr = text;
    onFEMTabEdited();
}
void TabManager::onGMTextChanged(const QString& text)
{
    GMPathStr = text;
    onFEMTabEdited();
}

void TabManager::onConfigTabEdtFinished()
{
    emit configTabUpdated();
}



void TabManager::writeGM()
{



    /*
     * Get rock motion from file
     */
    /*
    QString newGmPathStr = FEMWidget->findChild<QLineEdit*>("GMPath")->text();
    QFile file(newGmPathStr);
    QStringList xd, yd;
    if(file.open(QIODevice::ReadOnly)) {
        QTextStream in(&file);
        while(!in.atEnd()) {
            QString line = in.readLine();
            QStringList thisLine = line.split(" ");
            if (thisLine.size()<2)
                break;
            xd.append(thisLine[0].trimmed());
            yd.append(thisLine[1].trimmed());
        }
        file.close();
    }
    */



    QString newGmPathStr = FEMWidget->findChild<QLineEdit*>("GMPath")->text();
    QString in;
    QFile inputFile(newGmPathStr);
    if(inputFile.open(QFile::ReadOnly)) {
    //inputFile.open(QIODevice::ReadOnly | QIODevice::Text);
    in = inputFile.readAll();
    inputFile.close();
    }

    QJsonDocument indoc = QJsonDocument::fromJson(in.toUtf8());
    //qWarning() << indoc.isNull();
    QJsonObject inobj = indoc.object();

    QJsonArray events = inobj["Events"].toArray();
    if (events.size()>0)
    {
        QJsonArray patterns = events[0].toObject()["pattern"].toArray();
        QJsonArray timeseries = events[0].toObject()["timeSeries"].toArray();
        QString type = patterns[0].toObject()["type"].toString();
        QString tsname = patterns[0].toObject()["timeSeries"].toString();

        QJsonObject units = events[0].toObject()["units"].toObject();
        double g = 9.81;
        double lengthUnit = 1.;
        double timeUnit = 1.; // I guess everybody use sec for time.
        if(!units.isEmpty())
        {
            QString lengthType = units["length"].toString();
            if (lengthType=="in")
                lengthUnit = 0.0254;
            else if (lengthType=="m")
                lengthUnit = 1.0;
            else
                qDebug() << "length unit not recognized.";// TODO: more handling in future
            g = lengthUnit / timeUnit / timeUnit;
        }


        if(type=="Time-Velocity")
        {
            for(int i=0;i<timeseries.size();i++)
            {
                QJsonObject tstmp = timeseries[i].toObject();
                if(tstmp["name"]==tsname)
                {
                    QFile outFile(analysisDir+"/Rock.vel");
                    outFile.open(QIODevice::WriteOnly | QIODevice::Text);
                    QTextStream stream(&outFile);

                    QFile outFileTime(analysisDir+"/Rock.time");
                    outFileTime.open(QIODevice::WriteOnly | QIODevice::Text);
                    QTextStream streamTime(&outFileTime);

                    QJsonArray tV = tstmp["time"].toArray();
                    QJsonArray data = tstmp["data"].toArray();
                    for(int j=0;j<tV.size();j++)
                        streamTime << QString::number(tV[j].toDouble())+"\n";
                    for(int j=0;j<data.size();j++)
                        stream << QString::number(data[j].toDouble(),'g',16)+"\n";

                    outFile.close();
                    outFileTime.close();

                    break;
                }
            }
        }else if(type=="Time-Acceleration") // unit in g
        {
            for(int i=0;i<timeseries.size();i++)
            {
                QJsonObject tstmp = timeseries[i].toObject();
                if(tstmp["name"]==tsname)
                {
                    QFile outFile(analysisDir+"/Rock.acc");
                    outFile.open(QIODevice::WriteOnly | QIODevice::Text);
                    QTextStream stream(&outFile);

                    QFile outFileTime(analysisDir+"/Rock.time");
                    outFileTime.open(QIODevice::WriteOnly | QIODevice::Text);
                    QTextStream streamTime(&outFileTime);

                    QJsonArray tV = tstmp["time"].toArray();
                    QJsonArray data = tstmp["data"].toArray();
                    for(int j=0;j<tV.size();j++)
                        streamTime << QString::number(tV[j].toDouble())+"\n";
                    for(int j=0;j<data.size();j++)
                        stream << QString::number(data[j].toDouble())+"\n";

                    outFile.close();
                    outFileTime.close();

                    break;
                }
            }
        }else if(type=="Time-Displacement")
        {
            for(int i=0;i<timeseries.size();i++)
            {
                QJsonObject tstmp = timeseries[i].toObject();
                if(tstmp["name"]==tsname)
                {
                    QFile outFile(analysisDir+"/Rock.disp");
                    outFile.open(QIODevice::WriteOnly | QIODevice::Text);
                    QTextStream stream(&outFile);

                    QFile outFileTime(analysisDir+"/Rock.time");
                    outFileTime.open(QIODevice::WriteOnly | QIODevice::Text);
                    QTextStream streamTime(&outFileTime);

                    QJsonArray tV = tstmp["time"].toArray();
                    QJsonArray data = tstmp["data"].toArray();
                    for(int j=0;j<tV.size();j++)
                        streamTime << QString::number(tV[j].toDouble())+"\n";
                    for(int j=0;j<data.size();j++)
                        stream << QString::number(data[j].toDouble())+"\n";

                    outFile.close();
                    outFileTime.close();

                    break;
                }
            }
        } else if(type=="UniformVelocity")
        {
            for(int i=0;i<timeseries.size();i++)
            {
                QJsonObject tstmp = timeseries[i].toObject();
                if(tstmp["name"]==tsname)
                {
                    QFile outFile(analysisDir+"/Rock.vel");
                    outFile.open(QIODevice::WriteOnly | QIODevice::Text);
                    QTextStream stream(&outFile);

                    QFile outFileTime(analysisDir+"/Rock.time");
                    outFileTime.open(QIODevice::WriteOnly | QIODevice::Text);
                    QTextStream streamTime(&outFileTime);

                    double dTtmp = tstmp["dT"].toDouble();
                    QJsonArray data = tstmp["data"].toArray();
                    for(int j=0;j<data.size();j++)
                        streamTime << QString::number(dTtmp*j)+"\n";
                    for(int j=0;j<data.size();j++)
                        stream << QString::number(data[j].toDouble(),'g',16)+"\n";

                    outFile.close();
                    outFileTime.close();

                    break;
                }
            }
        } else if(type=="UniformAcceleration")
        {

            for(int i=0;i<timeseries.size();i++)
            {
                QJsonObject tstmp = timeseries[i].toObject();
                if(tstmp["name"]==tsname)
                {

                    QFile outFile(analysisDir+"/Rock.vel");
                    outFile.open(QIODevice::WriteOnly | QIODevice::Text);
                    QTextStream stream(&outFile);

                    QFile outFileTime(analysisDir+"/Rock.time");
                    outFileTime.open(QIODevice::WriteOnly | QIODevice::Text);
                    QTextStream streamTime(&outFileTime);

                    double dTtmp = tstmp["dT"].toDouble();
                    QJsonArray data = tstmp["data"].toArray();
                    for(int j=0;j<data.size();j++)
                        streamTime << QString::number(dTtmp*j)+"\n";
                    double vprevious = 0.0;
                    for(int j=0;j<data.size();j++)
                    {
                        double vnow = vprevious + data[j].toDouble()*g*dTtmp;
                        vprevious = vnow;
                        stream << QString::number(vnow,'g',16)+"\n";
                    }
                    outFile.close();
                    outFileTime.close();

                    break;
                }
            }
        }


    }else{
        qWarning() << "No events found in the input file.";
    }








}

void TabManager::writeGMVintage()
{

    QFile outFile(analysisDir+"/Rock.vel");
    outFile.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream stream(&outFile);

    QFile outFileTime(analysisDir+"/Rock.time");
    outFileTime.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream streamTime(&outFileTime);

    /*
     * Get rock motion from file
     */
    QString newGmPathStr = FEMWidget->findChild<QLineEdit*>("GMPath")->text();
    QFile file(newGmPathStr);
    QStringList xd, yd;
    if(file.open(QIODevice::ReadOnly)) {
        QTextStream in(&file);
        while(!in.atEnd()) {
            QString line = in.readLine();
            QStringList thisLine = line.split(" ");
            if (thisLine.size()<2)
                break;
            xd.append(thisLine[0].trimmed());
            yd.append(thisLine[1].trimmed());
        }
        file.close();
    }


    for (int i=0; i<(yd.size()); i++)
    {
        streamTime<<xd.at(i) << "\n";
        stream<<yd.at(i) << "\n";
    }

    outFile.close();
    outFileTime.close();


}

void TabManager::onFEMTabEdited()
{
    writeGM();

    // writing FEM.dat
    QString filename = femFilename;//"FEM.dat";
    QFile file(filename);
    if(file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text)) {
            QTextStream stream(&file);
            for (int i = 0; i < edtsFEM.size(); i++) {
                stream<< listFEMtab[i] << ","<<" "<<edtsFEM[i]->text() << endl;
            }
            stream<< "GWT" << ","<<" "<<tableView->getGWT() << endl;
            //QString openseesPath = FEMWidget->findChild<QLineEdit*>("openseesPath")->text();
            //stream<< "OpenSeesPath" << ","<<" "<<openseesPath << endl;
            file.close();
    }

    QString newGmPathStr = FEMWidget->findChild<QLineEdit*>("GMPath")->text();
    if(newGmPathStr != GMPathStr){
        GMPathStr = newGmPathStr;
        reFreshGMTab();
    }
}


void TabManager::reFreshGMTab()
{


    updateVelHtml();
    updateAccHtml();
    updateDispHtml();
    updatePWPHtml();
    updateSaHtml();
    updateStrainHtml();
    updateStressHtml();
    updateStressStrainHtml();
    updateRupwpHtml();

    GMView->reload();
    //GMView->show();

}

void TabManager::initGMTab()
{
    QString GMTabHtmlNameTmp = QDir(rootDir).filePath("resources/ui/GroundMotion/index-template.html");
    QString accHtmlNameTmp = QDir(rootDir).filePath("resources/ui/GroundMotion/acc-template.html");
    QString dispHtmlNameTmp = QDir(rootDir).filePath("resources/ui/GroundMotion/disp-template.html");
    QString pwpHtmlNameTmp = QDir(rootDir).filePath("resources/ui/GroundMotion/pwp-template.html");
    QString rupwpHtmlNameTmp = QDir(rootDir).filePath("resources/ui/GroundMotion/rupwp-template.html");
    QString strainHtmlNameTmp = QDir(rootDir).filePath("resources/ui/GroundMotion/strain-template.html");
    QString stressHtmlNameTmp = QDir(rootDir).filePath("resources/ui/GroundMotion/stress-template.html");
    QFile file1(GMTabHtmlName);
    if(!file1.exists())
        QFile::copy(GMTabHtmlNameTmp, GMTabHtmlName);
    QFile file2(accHtmlName);
    if(!file2.exists())
        QFile::copy(accHtmlNameTmp, accHtmlName);
    QFile file3(dispHtmlName);
    if(!file3.exists())
        QFile::copy(dispHtmlNameTmp, dispHtmlName);
    QFile file4(pwpHtmlName);
    if(!file4.exists())
        QFile::copy(pwpHtmlNameTmp, pwpHtmlName);
    QFile file5(rupwpHtmlName);
    if(!file5.exists())
        QFile::copy(rupwpHtmlNameTmp, rupwpHtmlName);
    QFile file6(strainHtmlName);
    if(!file6.exists())
        QFile::copy(strainHtmlNameTmp, strainHtmlName);
    QFile file7(stressHtmlName);
    if(!file7.exists())
        QFile::copy(stressHtmlNameTmp, stressHtmlName);
    GMView->reload();
}

void TabManager::updateVelHtml()
{
    // get file paths
    QFileInfo indexHtmlInfo(GMTabHtmlName);
    QString tmpPath = QDir(rootDir).filePath("resources/ui/GroundMotion/index-template.html");
    QString newPath = QDir(rootDir).filePath("resources/ui/GroundMotion/index.html");
    QFile::remove(newPath);
    //QFile::copy(tmpPath, newPath);

    // read template file into string
    QFile file(tmpPath);
    QString text;
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QByteArray t = file.readAll();
        text = QString(t);
        file.close();
    }


    QString insertedString = loadGMtoString();
    text.replace(QString("//UPDATEPOINT"), insertedString);

    // write to index.html
    QFile newfile(newPath);
    if(newfile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        newfile.write(text.toUtf8());
        newfile.close();
    }
}


void TabManager::updateAccHtml()
{
    // get file paths
    QFileInfo htmlInfo(accHtmlName);
    //QString dir = htmlInfo.path();
    QString tmpPath = QDir(rootDir).filePath("resources/ui/GroundMotion/acc-template.html");
    QString newPath = QDir(rootDir).filePath("resources/ui/GroundMotion/acc.html");
    QFile::remove(newPath);

    // read template file into string
    QString text;
    QFile file(tmpPath);
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QByteArray t = file.readAll();
        text = QString(t);
        file.close();
    }



    QString insertedString = loadMotions2String("acc");
    text.replace(QString("//UPDATEPOINT"), insertedString);


    // write to index.html
    QFile newfile(newPath);
    if(newfile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        newfile.write(text.toUtf8());
        newfile.close();
    }
}

void TabManager::updateDispHtml()
{
    // get file paths
    QFileInfo htmlInfo(dispHtmlName);
    //QString dir = htmlInfo.path();
    QString tmpPath = QDir(rootDir).filePath("resources/ui/GroundMotion/disp-template.html");
    QString newPath = QDir(rootDir).filePath("resources/ui/GroundMotion/disp.html");
    QFile::remove(newPath);

    // read template file into string
    QString text;
    QFile file(tmpPath);
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QByteArray t = file.readAll();
        text = QString(t);
        file.close();
    }


    QString insertedString = loadMotions2String("disp");
    text.replace(QString("//UPDATEPOINT"), insertedString);


    // write to index.html
    QFile newfile(newPath);
    if(newfile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        newfile.write(text.toUtf8());
        newfile.close();
    }
}

void TabManager::updateStrainHtml()
{
    // get file paths
    QFileInfo htmlInfo(strainHtmlName);
    //QString dir = htmlInfo.path();
    QString tmpPath = QDir(rootDir).filePath("resources/ui/GroundMotion/strain-template.html");
    QString newPath = QDir(rootDir).filePath("resources/ui/GroundMotion/strain.html");
    QFile::remove(newPath);

    // read template file into string
    QFile file(tmpPath);
    QString text;
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QByteArray t = file.readAll();
        text = QString(t);
        file.close();
    }


    QString insertedString = loadEleResponse("strain");
    text.replace(QString("//UPDATEPOINT"), insertedString);


    // write to index.html
    QFile newfile(newPath);
    if(newfile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        newfile.write(text.toUtf8());
        newfile.close();
    }
}


void TabManager::updateStressHtml()
{
    // get file paths
    QFileInfo htmlInfo(strainHtmlName);
    //QString dir = htmlInfo.path();
    QString tmpPath = QDir(rootDir).filePath("resources/ui/GroundMotion/stress-template.html");
    QString newPath = QDir(rootDir).filePath("resources/ui/GroundMotion/stress.html");
    QFile::remove(newPath);

    // read template file into string
    QFile file(tmpPath);
    QString text;
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QByteArray t = file.readAll();
        text = QString(t);
        file.close();
    }


    QString insertedString = loadEleResponse("stress");
    text.replace(QString("//UPDATEPOINT"), insertedString);


    // write to index.html
    QFile newfile(newPath);
    if(newfile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        newfile.write(text.toUtf8());
        newfile.close();
    }
}

void TabManager::updateStressStrainHtml()
{
    // get file paths
    QFileInfo htmlInfo(strainHtmlName);
    //QString dir = htmlInfo.path();
    QString tmpPath = QDir(rootDir).filePath("resources/ui/GroundMotion/stressstrain-template.html");
    QString newPath = QDir(rootDir).filePath("resources/ui/GroundMotion/stressstrain.html");
    QFile::remove(newPath);

    // read template file into string
    QFile file(tmpPath);
    QString text;
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QByteArray t = file.readAll();
        text = QString(t);
        file.close();
    }


    QString insertedString = loadEleResponse("stressstrain");
    text.replace(QString("//UPDATEPOINT"), insertedString);


    // write to index.html
    QFile newfile(newPath);
    if(newfile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        newfile.write(text.toUtf8());
        newfile.close();
    }
}

void TabManager::updatePWPHtml()
{
    // get file paths
    QFileInfo htmlInfo(pwpHtmlName);
    //QString dir = htmlInfo.path();
    QString tmpPath = QDir(rootDir).filePath("resources/ui/GroundMotion/pwp-template.html");
    QString newPath = QDir(rootDir).filePath("resources/ui/GroundMotion/pwp.html");
    QFile::remove(newPath);

    // read template file into string
    QFile file(tmpPath);
    QString text;
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QByteArray t = file.readAll();
        text = QString(t);
        file.close();
    }


    QString insertedString = loadPWPResponse();
    text.replace(QString("//UPDATEPOINT"), insertedString);


    // write to index.html
    QFile newfile(newPath);
    if(newfile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        newfile.write(text.toUtf8());
        newfile.close();
    }
}


void TabManager::updateRupwpHtml()
{
    // get file paths
    QFileInfo htmlInfo(rupwpHtmlName);
    //QString dir = htmlInfo.path();
    QString tmpPath = QDir(rootDir).filePath("resources/ui/GroundMotion/rupwp-template.html");
    QString newPath = QDir(rootDir).filePath("resources/ui/GroundMotion/rupwp.html");
    QFile::remove(newPath);

    // read template file into string
    QFile file(tmpPath);
    QString text;
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QByteArray t = file.readAll();
        text = QString(t);
        file.close();
    }


    QString insertedString = loadruPWPResponse();
    text.replace(QString("//UPDATEPOINT"), insertedString);


    // write to index.html
    QFile newfile(newPath);
    if(newfile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        newfile.write(text.toUtf8());
        newfile.close();
    }
}


void TabManager::updateSaHtml()
{
    // get file paths
    QFileInfo htmlInfo(pwpHtmlName);
    //QString dir = htmlInfo.path();
    QString tmpPath = QDir(rootDir).filePath("resources/ui/GroundMotion/sa-template.html");
    QString newPath = QDir(rootDir).filePath("resources/ui/GroundMotion/sa.html");
    QFile::remove(newPath);

    // read template file into string
    QFile file(tmpPath);
    QString text;
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QByteArray t = file.readAll();
        text = QString(t);
        file.close();
    }


    QString insertedString = loadNodeSa();
    text.replace(QString("//UPDATEPOINT"), insertedString);


    // write to index.html
    QFile newfile(newPath);
    if(newfile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        newfile.write(text.toUtf8());
        newfile.close();
    }
}




QString TabManager::loadGMtoString()
{

    QString text;
    QTextStream stream(&text);

    /*
     * Get rock motion from file
     */
    writeGM();

    QStringList *xd = postProcessor->getxdBaseVel();
    QStringList *yd = postProcessor->getydBaseVel();

    int overStep = int(floor(xd->size()/maxStepToShow));

    stream << "xnew = ['x'";
    for (int i=0; i<xd->size(); i+=overStep)
        stream << ", "<<xd->at(i);
    stream <<"];" <<endl;

    stream << "ynew = ['Rock motion'";
    for (int i=0; i<yd->size(); i+=overStep)
        stream << ", "<<yd->at(i);
    stream <<"];" <<endl;

    /*
     * Get surface motion from file
     */
    //QString surfaceVelFileName = "/Users/simcenter/Codes/SimCenter/SiteResponseTool/bin/out_tcl/vel_surface.txt";

    QStringList *xdSurfaceVel = postProcessor->getxdSurfaceVel();
    QStringList *ydSurfaceVel = postProcessor->getydSurfaceVel();

    overStep = int(floor(xdSurfaceVel->size()/maxStepToShow));
    stream << "xSurfaceVel = ['x'";
    for (int i=0; i<xdSurfaceVel->size(); i+=overStep)
        stream << ", "<<xdSurfaceVel->at(i);
    stream <<"];" <<endl;

    stream << "ySurfaceVel = ['Surface motion'";
    for (int i=0; i<ydSurfaceVel->size(); i+=overStep)
        stream << ", "<<ydSurfaceVel->at(i).toDouble();
    stream <<"];" <<endl;

    QString nodeResponseStr = loadNodeResponse("vel");
    stream << nodeResponseStr;

    writeSurfaceMotion();




    //stream << "       xnew = ['x', 1, 2, 3, 4, 5, 6];" <<endl;
    //stream << "       ynew = ['Ground motion', 70, 180, 190, 180, 80, 250];"<<endl;
    //stream << "       chart.unload();"<<endl;
    stream << "       setTimeout(function () {"<<endl;
    stream << "       chart.load({"<<endl;
    stream << "           columns: ["<<endl;
    stream << "           xnew,"<<endl;
    stream <<"           ynew"<<endl;
    stream <<"           ]"<<endl;
    stream <<"       });"<<endl;

    stream << "       chart.load({"<<endl;
    stream << "           columns: ["<<endl;
    stream << "           xSurfaceVel,"<<endl;
    stream <<"           ySurfaceVel"<<endl;
    stream <<"           ]"<<endl;
    stream <<"       });"<<endl;

    stream <<"       chart.unload({"<<endl;
    stream <<"           ids: 'Demo motion 1'"<<endl;
    stream <<"       });"<<endl;
    stream <<"       chart.unload({"<<endl;
    stream <<"           ids: 'Demo motion 2'"<<endl;
    stream <<"       });"<<endl;
    stream <<"       }, 1000);"<<endl;
    return text;

}


QString TabManager::loadGMtoStringVintage()
{

    QString text;
    QTextStream stream(&text);



    /*
     * Get rock motion from file
     */
    QString newGmPathStr = FEMWidget->findChild<QLineEdit*>("GMPath")->text();
    QFile file(newGmPathStr);
    QStringList xd, yd;
    if(file.open(QIODevice::ReadOnly)) {
        QTextStream in(&file);
        while(!in.atEnd()) {
            QString line = in.readLine();
            QStringList thisLine = line.split(" ");
            if (thisLine.size()<2)
                break;
            xd.append(thisLine[0].trimmed());
            yd.append(thisLine[1].trimmed());
        }
        file.close();
    }

    stream << "xnew = ['x'";
    for (int i=0; i<xd.size(); i++)
        stream << ", "<<xd.at(i);
    stream <<"];" <<endl;

    stream << "ynew = ['Rock motion'";
    for (int i=0; i<yd.size(); i++)
        stream << ", "<<yd.at(i);
    stream <<"];" <<endl;

    /*
     * Get surface motion from file
     */
    //QString surfaceVelFileName = "/Users/simcenter/Codes/SimCenter/SiteResponseTool/bin/out_tcl/vel_surface.txt";
    QString surfaceVelFileName = analysisDir+"/out_tcl/surface.vel";
    QFile surfaceVelFile(surfaceVelFileName);
    QStringList xdSurfaceVel, ydSurfaceVel;
    if(surfaceVelFile.open(QIODevice::ReadOnly)) {
        QTextStream in(&surfaceVelFile);
        while(!in.atEnd()) {
            QString line = in.readLine();
            QStringList thisLine = line.split(" ");
            if (thisLine.size()<2)
                break;
            else
            {
                thisLine.removeAll("");
                if(thisLine.size()>1)
                {
                xdSurfaceVel.append(thisLine[0].trimmed());
                ydSurfaceVel.append(thisLine[1].trimmed());
                }else
                    break;

            }

        }
        surfaceVelFile.close();
    }

    stream << "xSurfaceVel = ['x'";
    for (int i=0; i<xdSurfaceVel.size(); i++)
        stream << ", "<<xdSurfaceVel.at(i);
    stream <<"];" <<endl;

    stream << "ySurfaceVel = ['Surface motion'";
    for (int i=0; i<ydSurfaceVel.size(); i++)
        stream << ", "<<ydSurfaceVel.at(i).toDouble();
    stream <<"];" <<endl;

    QString nodeResponseStr = loadNodeResponse("vel");
    stream << nodeResponseStr;

    writeSurfaceMotion();




    //stream << "       xnew = ['x', 1, 2, 3, 4, 5, 6];" <<endl;
    //stream << "       ynew = ['Ground motion', 70, 180, 190, 180, 80, 250];"<<endl;
    //stream << "       chart.unload();"<<endl;
    stream << "       setTimeout(function () {"<<endl;
    stream << "       chart.load({"<<endl;
    stream << "           columns: ["<<endl;
    stream << "           xnew,"<<endl;
    stream <<"           ynew"<<endl;
    stream <<"           ]"<<endl;
    stream <<"       });"<<endl;

    stream << "       chart.load({"<<endl;
    stream << "           columns: ["<<endl;
    stream << "           xSurfaceVel,"<<endl;
    stream <<"           ySurfaceVel"<<endl;
    stream <<"           ]"<<endl;
    stream <<"       });"<<endl;

    stream <<"       chart.unload({"<<endl;
    stream <<"           ids: 'Demo motion 1'"<<endl;
    stream <<"       });"<<endl;
    stream <<"       chart.unload({"<<endl;
    stream <<"           ids: 'Demo motion 2'"<<endl;
    stream <<"       });"<<endl;
    stream <<"       }, 1000);"<<endl;
    return text;

}


QString TabManager::loadMotions2String(QString motion)
{

    QString text;
    QTextStream stream(&text);

    QStringList *xdBase;
    QStringList *xdSurface;
    QStringList *ydBase;
    QStringList *ydSurface;

    if (motion=="vel")
    {
        xdBase = postProcessor->getxdBaseVel();
        ydBase = postProcessor->getydBaseVel();
        xdSurface = postProcessor->getxdSurfaceVel();
        ydSurface = postProcessor->getydSurfaceVel();
    } else if (motion=="acc")
    {
        xdBase = postProcessor->getxdBaseAcc();
        ydBase = postProcessor->getydBaseAcc();
        xdSurface = postProcessor->getxdSurfaceAcc();
        ydSurface = postProcessor->getydSurfaceAcc();
    } else {
        xdBase = postProcessor->getxdBaseDisp();
        ydBase = postProcessor->getydBaseDisp();
        xdSurface = postProcessor->getxdSurfaceDisp();
        ydSurface = postProcessor->getydSurfaceDisp();
    }


    int overStep;
    if(xdBase && ydBase)
    {
        overStep = int(floor(xdBase->size()/maxStepToShow));
        stream << "xnew = ['x'";
        for (int i=0; i<xdBase->size(); i+=overStep)
            stream << ", "<<xdBase->at(i);
        stream <<"];" <<endl;

        stream << "ynew = ['Rock motion'";
        for (int i=0; i<ydBase->size(); i+=overStep)
            stream << ", "<<ydBase->at(i);
        stream <<"];" <<endl;
    }



    QString nodeResponseStr = loadNodeResponse(motion);
    stream << nodeResponseStr;



    /*
     * Get surface motion from file
     */


    if(xdSurface && ydSurface)
    {
        stream << "xSurfaceVel = ['x'";
        for (int i=0; i<xdSurface->size(); i+=overStep)
            stream << ", "<<xdSurface->at(i);
        stream <<"];" <<endl;

        stream << "ySurfaceVel = ['Surface motion'";
        for (int i=0; i<ydSurface->size(); i+=overStep)
            stream << ", "<<ydSurface->at(i).toDouble();
        stream <<"];" <<endl;
    }

    writeSurfaceMotion();




    //stream << "       xnew = ['x', 1, 2, 3, 4, 5, 6];" <<endl;
    //stream << "       ynew = ['Ground motion', 70, 180, 190, 180, 80, 250];"<<endl;
    stream << "       chart.unload();"<<endl;
    stream << "       setTimeout(function () {"<<endl;
    stream << "       chart.load({"<<endl;
    stream << "           columns: ["<<endl;
    stream << "           xnew,"<<endl;
    stream <<"           ynew"<<endl;
    stream <<"           ]"<<endl;
    stream <<"       });"<<endl;

    stream << "       chart.load({"<<endl;
    stream << "           columns: ["<<endl;
    stream << "           xSurfaceVel,"<<endl;
    stream <<"           ySurfaceVel"<<endl;
    stream <<"           ]"<<endl;
    stream <<"       });"<<endl;

    stream <<"       chart.unload({"<<endl;
    stream <<"           ids: 'Demo motion 1'"<<endl;
    stream <<"       });"<<endl;
    stream <<"       chart.unload({"<<endl;
    stream <<"           ids: 'Demo motion 2'"<<endl;
    stream <<"       });"<<endl;
    stream <<"       }, 500);"<<endl;
    return text;

}

void TabManager::updatePostProcessor(PostProcessor *postProcessort)
{
    postProcessor = postProcessort;
}

QString TabManager::loadNodeResponse(QString motion)
{

    QVector<QVector<double>> *v;
    if(motion=="acc")
    {
        v = postProcessor->getaccAll();
    }
    else if (motion=="vel")
    {
        v = postProcessor->getvelAll();
    }
    else if (motion=="disp")
    {
        v = postProcessor->getdispAll();
    }
    else
        qWarning("motion must be acc, vel or disp!");


    QString text;
    QTextStream stream(&text);



    if(v->size()>0)
    {
        int overStep = int(floor((*v)[0].size()/maxStepToShow));

        stream << "time = ['x'";
        for (int i=0; i<(*v)[0].size(); i+=overStep)
            stream << ", "<<(*v)[0][i];
        stream <<"];" <<endl;

        int eleID = elementModel->getSize();
        for (int j=7;j<v->size();j+=4)
        {
            eleID -= 1;
            stream << "n"+QString::number(eleID)+" = ['Node "+QString::number(eleID)+"'";
            for (int i=0; i<(*v)[j].size(); i+=overStep)
                stream << ", "<<(*v)[j][i];
            stream <<"];" <<endl;
        }
    }


    return text;

}


QString TabManager::loadNodeSa()
{   // TODO: this function is too spaghetti....

    QVector<QVector<double>> *v = postProcessor->getaccAll();
    QVector<QVector<double>> *saVec = postProcessor->getSa();
    QVector<double> *Periods = postProcessor->getPeriods();


    QString text;
    QTextStream stream(&text);

    if(v->size()>0)
    {
        stream << "xnew = ['x'";
        for (int i=0; i<Periods->size(); i++)
            stream << ", "<< (*Periods)[i];
        stream <<"];" <<endl;

        stream << "ynew = ['Rock'";
        for (int i=0; i<(*saVec)[0].size(); i++)
            stream << ", "<<(*saVec)[0][i];
        stream <<"];" <<endl;

        stream << "time = ['x'";
        for (int i=0; i<Periods->size(); i++)
            stream << ", "<<(*Periods)[i];
        stream <<"];" <<endl;


        int eleID = elementModel->getSize()+1;
        for (int j=0;j<saVec->size();j++)
        {
            eleID -= 1;
            //stream << "n1 = ['Node 1'";
            stream << "n"+QString::number(eleID)+" = ['Node "+QString::number(eleID)+"'";
            //stream << "n"+QString::number(eleID)+" = ['Node marked by <'";
            for (int i=0; i<(*saVec)[j].size(); i++)
                stream << ", "<<(*saVec)[j][i];
            stream <<"];" <<endl;
        }

    }else
        return "";


    return text;


}


QString TabManager::loadPWPResponse()
{

    QFile File(postProcessor->getPWPFileName());

    QVector<QVector<double>> v;
    if(File.open(QIODevice::ReadOnly)) {
        QTextStream in(&File);
        int lineCount = 0;
        int numCols = 0;
        while(!in.atEnd()) {
            QString line = in.readLine();
            QStringList thisLine = line.split(" ");
            thisLine.removeAll("");
            int sizeThisLine = thisLine.size();
            lineCount += 1;
            if (lineCount==1)
                numCols = sizeThisLine;
            if (sizeThisLine != numCols && lineCount>1)
            {
                lineCount -= 1;
                break;
            }
            else
            {
                //thisLine.removeAll("");
                for (int i=0; i<thisLine.size();i++)// TODO: 3D?
                {
                    if (lineCount==1)
                    {
                        QVector<double> tmpV;
                        v.append(tmpV);
                    }
                    v[i].append((thisLine[i].trimmed().toDouble()));
                }
            }
        }
        File.close();
    }

    QString text;
    QTextStream stream(&text);

    if(v.size()>0)
    {

        int overStep = int(floor(v[0].size()/maxStepToShow));
        stream << "xnew = ['x'";
        for (int i=0; i<v[0].size(); i+=overStep)
            stream << ", "<<v[0][i];
        stream <<"];" <<endl;

        stream << "ynew = ['Rock'";
        for (int i=0; i<v[2].size(); i+=overStep)
            stream << ", "<<v[2][i];
        stream <<"];" <<endl;


        stream << "time = ['x'";
        for (int i=0; i<v[0].size(); i+=overStep)
            stream << ", "<<v[0][i];
        stream <<"];" <<endl;


        int eleID = elementModel->getSize();
        for (int j=4;j<v.size();j+=2)
        {
            eleID -= 1;
            //stream << "n1 = ['Node 1'";
            stream << "pwp"+QString::number(eleID)+" = ['Node "+QString::number(eleID)+"'";
            //stream << "pwp"+QString::number(eleID)+" = ['Node marked by <'";
            for (int i=0; i<v[j].size(); i+=overStep)
                stream << ", "<<v[j][i];
            stream <<"];" <<endl;
        }
    }


    return text;

}

QString TabManager::loadruPWPResponse()
{

    QFile File(postProcessor->getPWPFileName());

    QVector<QVector<double>> v;
    if(File.open(QIODevice::ReadOnly)) {
        QTextStream in(&File);
        int lineCount = 0;
        int numCols = 0;
        while(!in.atEnd()) {
            QString line = in.readLine();
            QStringList thisLine = line.split(" ");
            thisLine.removeAll("");
            int sizeThisLine = thisLine.size();
            lineCount += 1;
            if (lineCount==1)
                numCols = sizeThisLine;
            if (sizeThisLine != numCols && lineCount>1)
            {
                lineCount -= 1;
                break;
            }
            else
            {
                //thisLine.removeAll("");
                for (int i=0; i<thisLine.size();i++)// TODO: 3D?
                {
                    if (lineCount==1)
                    {
                        QVector<double> tmpV;
                        v.append(tmpV);
                    }
                    v[i].append((thisLine[i].trimmed().toDouble()));
                }
            }
        }
        File.close();
    }

    QString text;
    QTextStream stream(&text);

    if(v.size()>0)
    {

        int overStep = int(floor(v[0].size()/maxStepToShow));
        stream << "time = ['x'";
        for (int i=0; i<v[0].size(); i+=overStep)
            stream << ", "<<v[0][i];
        stream <<"];" <<endl;


        int eleID = elementModel->getSize();
        int eleInd = -1;
        for (int j=4;j<v.size();j+=2)
        {
            eleID -= 1;
            //stream << "n1 = ['Node 1'";
            stream << "rupwp"+QString::number(eleID)+" = ['Element "+QString::number(eleID)+"'";
            eleInd += 1;

            for (int i=0; i<v[j].size(); i+=overStep)
            {
                /*
                double thisepwp = (v[j][i]-v[j][0]);
                double thissigma = m_vStress[eleInd*3+1+2][i];
                if(fabs(thisepwp)<1.e-16)
                    stream << ", "<<0;
                else{
                    if(fabs(thissigma)<1e-16)
                    {
                        thissigma = 1e-16;
                        stream << ", "<<thisepwp/thissigma;
                    }else
                        stream << ", "<<thisepwp/thissigma;
                }
                */

                double thisepwp = (v[j][i]-v[j][0]);
                stream << ", "<<thisepwp;
            }
            stream <<"];" <<endl;
        }
    }


    return text;

}


QVector<QVector<double>> TabManager::getElemResVec(QString fileName)
{
    QFile File(fileName);

    QVector<QVector<double>> v;
    if(File.open(QIODevice::ReadOnly)) {
        QTextStream in(&File);
        int lineCount = 0;
        int numCols = 0;
        while(!in.atEnd()) {
            QString line = in.readLine();
            QStringList thisLine = line.split(" ");
            thisLine.removeAll("");
            int sizeThisLine = thisLine.size();
            lineCount += 1;
            if (lineCount==1)
                numCols = sizeThisLine;
            if (sizeThisLine != numCols && lineCount>1)
            {
                lineCount -= 1;
                break;
            }
            else
            {
                //thisLine.removeAll("");
                for (int i=0; i<thisLine.size();i++)// TODO: 3D?
                {
                    if (lineCount==1)
                    {
                        QVector<double> tmpV;
                        v.append(tmpV);
                    }
                    v[i].append((thisLine[i].trimmed().toDouble()));
                }
            }
        }
        File.close();
    }
    return v;
}

QString TabManager::loadEleResponse(QString motion)
{

    QString fileName;
    QString stressFileName = postProcessor->getStressFileName();
    QString strainFileName = postProcessor->getStrainFileName();
    QVector<QVector<double>> vStress = getElemResVec(stressFileName);
    QVector<QVector<double>> vStrain = getElemResVec(strainFileName);

    if (motion=="strain")
    {
        fileName = postProcessor->getStrainFileName();
        vStrain = getElemResVec(fileName);
    }
    else if (motion=="stress")
    {
        fileName = postProcessor->getStressFileName();
        vStress = getElemResVec(fileName);
        m_vStress.clear();
        m_vStress = vStress;
    }
    else if (motion=="stressstrain")
    {
        QString stressFileName = postProcessor->getStressFileName();
        QString strainFileName = postProcessor->getStrainFileName();
        vStress = getElemResVec(stressFileName);
        vStrain = getElemResVec(strainFileName);
    }

    QVector<QVector<double>> v;



    QString text;
    QTextStream stream(&text);

    if(vStress.size()>0)
    {
        int overStep = int(floor(vStress[0].size()/maxStepToShow));
        stream << "time = ['x'";
        for (int i=0; i<vStress[0].size(); i+=overStep)
            stream << ", "<<vStress[0][i];
        stream <<"];" <<endl;

        QString outTitle;
        if(motion=="strain")
        {
            outTitle = "Strain";
            v = vStrain;
        }
        else if(motion=="stress")
        {
            outTitle="Stress";
            v = vStress;
        }
        else if(motion=="stressstrain")
            outTitle="StressStrain";

        if(motion!="stressstrain")
        {
            int eleID = elementModel->getSize();
            for (int j=3;j<v.size();j+=3)
            {
                eleID -= 1;
                //stream << "n1 = ['Node 1'";
                stream << outTitle+QString::number(eleID)+" = ['Element "+QString::number(eleID)+"'";
                //stream << "pwp"+QString::number(eleID)+" = ['Node marked by <'";
                for (int i=0; i<v[j].size(); i+=overStep)
                    stream << ", "<<v[j][i];
                stream <<"];" <<endl;
            }
        }else{

            int eleID = elementModel->getSize();
            for (int j=3;j<vStress.size();j+=3)
            {
                eleID -= 1;

                stream << "Stress"+QString::number(eleID)+" = ['Element "+QString::number(eleID)+"'";
                for (int i=0; i<vStress[j].size(); i+=overStep)
                {
                    stream << ", "<<vStress[j][i] ;
                }
                stream <<"];" <<endl;

                stream << "Strain"+QString::number(eleID)+" = ['x'";
                for (int i=0; i<vStrain[j].size(); i+=overStep)
                {
                    stream << ", "<<vStrain[j][i] ;
                }
                stream <<"];" <<endl;
            }
        }
    }


    return text;



}

bool TabManager::writeSurfaceMotion()
{
    QString surfaceAccFileName = analysisDir+"/out_tcl/surface.acc";
    QFile surfaceAccFile(surfaceAccFileName);
    QStringList xdSurfaceAcc, ydSurfaceAcc;
    if(surfaceAccFile.open(QIODevice::ReadOnly)) {
        QTextStream in(&surfaceAccFile);
        while(!in.atEnd()) {
            QString line = in.readLine();
            QStringList thisLine = line.split(" ");
            if (thisLine.size()<2)
                break;
            else
            {
                thisLine.removeAll("");
                if(thisLine.size()>1)
                {
                    xdSurfaceAcc.append(thisLine[0].trimmed());
                    ydSurfaceAcc.append(thisLine[1].trimmed());
                }else
                    break;

            }

        }
        surfaceAccFile.close();
    }



    // write surface motion in json file
    QJsonObject surfAcc;
    QFile saveFile(analysisDir + "/EVENT-SRT.json");
    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file.");
    }


    double dT;
    if (xdSurfaceAcc.size()>0)
        dT = (xdSurfaceAcc.at(xdSurfaceAcc.size()-1).toDouble() - xdSurfaceAcc.at(0).toDouble()) / double(xdSurfaceAcc.size()-1);
    else
        dT = 0.005;//TODO



    QJsonObject evt, timeSerix,timeSeriy, patternx, patterny, RV;
    QJsonArray evts,timeSeries,patterns,RVs,tx,ty;


    for (int i=0; i<ydSurfaceAcc.size(); i++)
    {
        tx.append(ydSurfaceAcc.at(i).toDouble()/9.81);
        ty.append(0.0);
    }




    timeSerix["name"]= "accel_X";
    timeSerix["type"]= "Value";
    timeSerix["dT"]= dT;
    timeSerix["data"]= tx;
    timeSeries.append(timeSerix);
    /*
    timeSeriy["name"]= "accel_Y";
    timeSeriy["type"]= "Value";
    timeSeriy["dT"]= dT;
    timeSeriy["data"]= ty;
    timeSeries.append(timeSeriy);
    */

    patternx["type"] = "UniformAcceleration";
    patternx["timeSeries"] = "accel_X";
    patternx["dof"] = 1;
    patterns.append(patternx);
    /*
    patterny["type"] = "UniformAcceleration";
    patterny["timeSeries"] = "accel_Y";
    patterny["dof"] = 2;
    patterns.append(patterny);
    */


    evt["name"] = "SiteResponseTool";
    evt["type"] = "Seismic";
    evt["description"] = "Surface acceleration";
    evt["dT"] = dT;
    evt["numSteps"] = xdSurfaceAcc.size();
    evt["timeSeries"] = timeSeries;
    evt["pattern"] = patterns;
    evt["RandomVariables"] = RVs;



    evts.append(evt);
    surfAcc["Events"] = evts;


    QJsonDocument saveDoc;
    saveDoc.setObject(surfAcc);
    saveFile.write(saveDoc.toJson());
    saveFile.close();

    return 0;
}



double TabManager::getGWTFromConfig()
{
    double GWT=0.0;
    QString filename = femFilename;//"FEM.dat";
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly)) {
        file.close();
        return GWT;
    }
    if(!file.isOpen())
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

    QString filename = femFilename;// "FEM.dat";//QDir(dirname).filePath(femfilename);
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
            stream << "openseesPath,"<<" "<<"Input the full path of OpenSees excutable. " << endl;
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


    for (int i = 0; i < (edtsFEM.size()); i++) {
        edtsFEM[i]->setText(savedPars.at(i));
    }

    //edtsFEM[edtsFEM.size()-1]->setText(savedPars.at(edtsFEM.size()));
    GMPathStr = FEMWidget->findChild<QLineEdit*>("GMPath")->text();
    openseesPathStr = FEMWidget->findChild<QLineEdit*>("openseesPath")->text();
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
    tab->insertTab(0,FEMWidget,"Configure");
    tab->insertTab(1,currentWidget,"Layer properties");
    tab->insertTab(2,GMView,"Response");
    tab->setCurrentIndex(1);

    /*
    QList<QVariant> infos = tableView->getRowInfo(currentRow);
    if (infos.size()>0)
    {
        qDebug() <<"Mat " << infos.at(MATERIAL-2).toString();
    }
    */



    fillMatTab(thisMatType, index);

}

void TabManager::updateGMPath(QString path)
{
    edtsFEM[GMPathPos]->setText(path);
}
void TabManager::updateOpenSeesPath(QString path)
{
    edtsFEM[OpenSeesPathPos]->setText(path);
}

void TabManager::updateLayerTab(QJsonObject l,QJsonObject mat)
{
    QString name = l["name"].toString();
    QString color = l["color"].toString();
    int id = l["id"].toInt();
    QString matType = mat["type"].toString();
    double hPermval = l["hPerm"].toDouble();
    double vPermval = l["vPerm"].toDouble();
    double uBulkval = l["uBulk"].toDouble();


    // update element pars
    if(matType=="Elastic")
    {
        double E = mat["E"].toDouble();
        double poisson = mat["poisson"].toDouble();

        // update
        QLineEdit *EEdt= ElasticIsotropicWidget->findChild<QLineEdit*>("EEdt");
        EEdt->setText(QString::number(E,'g',16));
        QLineEdit *vEdt= ElasticIsotropicWidget->findChild<QLineEdit*>("vEdt");
        vEdt->setText(QString::number(poisson,'g',16));
        QLineEdit *hPerm= ElasticIsotropicWidget->findChild<QLineEdit*>("hPermEdt");
        hPerm->setText(QString::number(hPermval,'g',16));
        QLineEdit *vPerm= ElasticIsotropicWidget->findChild<QLineEdit*>("vPermEdt");
        vPerm->setText(QString::number(vPermval,'g',16));
        QLineEdit *uBulk= ElasticIsotropicWidget->findChild<QLineEdit*>("uBulkEdt");
        uBulk->setText(QString::number(uBulkval,'g',16));

    } else if(matType=="PM4Sand")
    {
        for (int i = 0; i < listPM4SandFEM.size(); ++i) {
            QString edtName = listPM4SandFEM[i] ;
            if(!mat[edtName].isNull())
                edtsPM4SandFEM[i]->setText(QString::number(mat[edtName].toDouble(),'g',16));
        }
        QLineEdit *hPerm= PM4SandWidget->findChild<QLineEdit*>("hPerm");
        hPerm->setText(QString::number(hPermval,'g',16));
        QLineEdit *vPerm= PM4SandWidget->findChild<QLineEdit*>("vPerm");
        vPerm->setText(QString::number(vPermval,'g',16));
        QLineEdit *uBulk= PM4SandWidget->findChild<QLineEdit*>("uBulk");
        uBulk->setText(QString::number(uBulkval,'g',16));
    }

    // send the signal to update FEM cell
    onDataEdited();


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
            //qDebug() << FEMStringList.at(i);
        }

        // for Elastic, check the density are the same as shown in the soil layer table
        if (thisMatType == "Elastic")
        {
            QString densityFromTable = tableModel->record(index.row()).value("DENSITY").toString();
            QLineEdit* DenEdt = ElasticIsotropicWidget->findChild<QLineEdit*>("rhoEdt");
            QString densityFromForm = DenEdt->text();


            if(densityFromTable != densityFromForm)
            {
                //qDebug() << "Den here is different from the above table. ";
                if (densityFromTable == "")
                {
                    tableModel->setData(tableModel->index(index.row(), DENSITY), densityFromForm);
                    DenEdt->setText(densityFromForm);
                }
                else
                {
                    DenEdt->setText(densityFromTable);
                }

                double vsFromTable = tableModel->record(index.row()).value("VS").toDouble();
                QLineEdit* vEdt = ElasticIsotropicWidget->findChild<QLineEdit*>("vEdt");
                double v = vEdt->text().toDouble();
                QLineEdit* EEdt = ElasticIsotropicWidget->findChild<QLineEdit*>("EEdt");
                double E = 2.0 * densityFromTable.toDouble() * vsFromTable * vsFromTable * (1. + v);
                EEdt->setText(QString::number(E,'g',16));
                onDataEdited();// added
            }




            QString esizeFromTable = tableModel->record(index.row()).value("ElementSize").toString();
            QLineEdit* esizeEdt = ElasticIsotropicWidget->findChild<QLineEdit*>("eSize");
            QString esizeFromForm = esizeEdt->text();
            if(esizeFromTable != esizeFromForm)
            {
                //qDebug() << "esize here is different from the above table. ";
                if (esizeFromTable == "")
                {
                    tableModel->setData(tableModel->index(index.row(), ESIZE), esizeFromForm);
                }
                else
                {
                    esizeEdt->setText(esizeFromTable);
                    onDataEdited();
                }
            }
            //onDataEdited();
        }

        // for PM4Sand, check the density are the same as shown in the soil layer table
        if (thisMatType == "PM4Sand")
        {
            QString densityFromTable = tableModel->record(index.row()).value("DENSITY").toString();

            QLineEdit* DenEdt = PM4SandWidget->findChild<QLineEdit*>("Den");
            QString densityFromForm = DenEdt->text();

            if(densityFromTable != densityFromForm)
            {
                //qDebug() << "Den here is different from the above table. ";
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

            QString esizeFromTable = tableModel->record(index.row()).value("ElementSize").toString();
            QLineEdit* esizeEdt = PM4SandWidget->findChild<QLineEdit*>("eSize");
            QString esizeFromForm = esizeEdt->text();
            if(esizeFromTable != esizeFromForm)
            {
                //qDebug() << "esize here is different from the above table. ";
                if (esizeFromTable == "")
                {
                    tableModel->setData(tableModel->index(index.row(), ESIZE), esizeFromForm);
                }
                else
                {
                    esizeEdt->setText(esizeFromTable);
                    onDataEdited();
                }
            }
        }
        //onDataEdited();

    } else
    {
        //qDebug() << "FEMStringList.size() not == edts.size() !";
    }



}

void TabManager::onDataEdited()
{
    //qDebug() << "edited. I feel that.";

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
        numPars = 11;
    else if (thisMatType == "PM4Sand")
        numPars = 28;
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
            tableModel->setData(tableModel->index(index.row(), DENSITY), QString::number(rho,'g',16));
        }

        double vs = tableModel->record(index.row()).value("VS").toDouble();
        if (vs<1.0e-11 & vs>-1.0e-11) //
        {
            vs = 182.0; // set default Vs
            tableModel->setData(tableModel->index(index.row(), VS), QString::number(vs));
        }
        double E = 2.0 * rho * vs * vs * (1. + .3);

        double hPerm = 1.0e-7;
        double vPerm = 1.0e-7;
        double uBulk = 2.2e6;

        double Dr = 0.4663;
        double Gs = 2.67;
        double emax = 0.8;
        double emin = 0.5;
        double evoid = emax-Dr*(emax-emin);
        double rho_d = Gs/(1.0+evoid);
        double rho_s = rho_d*(1.0+evoid/Gs);
        QString defaultElasMat = " 2.0 " + QString::number(E)+" 0.3 "+QString::number(rho,'g',16);
        defaultElasMat += " "+QString::number(Dr);
        defaultElasMat += " "+QString::number(evoid);
        defaultElasMat += " "+QString::number(hPerm);
        defaultElasMat += " "+QString::number(vPerm);
        defaultElasMat += " "+QString::number(rho_d);
        defaultElasMat += " "+QString::number(rho_s);
        defaultElasMat += " "+QString::number(uBulk);


        tableModel->setData(tableModel->index(currentRow, FEM), defaultElasMat);
    }
    else if (thisMatType == "PM4Sand")
    {
        QString density = tableModel->record(index.row()).value("DENSITY").toString();
        if (density=="")
            density = "2.0";
        tableModel->setData(tableModel->index(currentRow, FEM), "2.0 0.47 500.0 0.45 "+ density +" 101.3 -1. 0.8 0.5 0.5 0.1 -1. -1. 250 -1. 33.0 0.3 2.0 -1. -1. 10. 1.5 0.01 -1. -1. "+"1.0e-7 1.0e-7 2.2e6");
        //  "1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24"
    }

}

void TabManager::onRunBtnClicked()
{
    if (tab->count()<4)
    {
        // init the dino view
        QWebEngineView *dinoView = new QWebEngineView(this);
        QString dinoHtmlName = QDir(rootDir).filePath("resources/ui/DinoRun/index.html");
        dinoView->load(QUrl::fromLocalFile(dinoHtmlName));
        dinoView->setVisible(false);

        tab->addTab(dinoView,"Run");
        tab->setCurrentIndex(3);
    }
    else
        tab->setCurrentIndex(3);
}

