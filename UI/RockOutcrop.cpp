#include <QQuickView>
#include "RockOutcrop.h"
#include "ui_RockOutcrop.h"
#include "InsertWindow.h"
#include <QQmlContext>

#include <QTime>
#include <QTimer>

#include <QtQuick/qquickitem.h>
#include <QtQuick/qquickview.h>

#include <QStringList>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QThread>
#include <QTabWidget>
#include <QUiLoader>
#include <QFileInfo>
#include <QMessageBox>
#include <QSizePolicy>
#include <QTabBar>


#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <iomanip>



RockOutcrop::RockOutcrop(QWidget *parent) :
    SimCenterAppWidget(parent),
    ui(new Ui::RockOutcrop)
{

    if(!QDir(analysisDir).exists())
    {
        QDir newDir(analysisDir);
        newDir.mkpath(".");
    }

    if(!loadPreviousResults)
    {
        QDir resDir(QDir(rootDir).filePath("resources"));
        resDir.removeRecursively();
    }

    copyDir(QDir(qApp->applicationDirPath()).filePath("resources"),QDir(rootDir).filePath("resources"),true);

    // create analysis dir
    if(!QDir(analysisDir).exists())
        QDir().mkdir(analysisDir);

    // random seed
    unsigned uintTime = static_cast<unsigned>(QTime::currentTime().msec());
    qsrand(uintTime);

    ui->setupUi(this);
    updateCtrl();

    // page setting
    QStringList pages;
    int totalPage = ui->tableView->totalPage();
    for( int i = 1; i <= totalPage; ++i)
        pages.append( QString("%1").arg(i) );
    ui->gotoPageBox->addItems(pages);

    // some connections
    connect(ui->tableView, SIGNAL(insertAct()), this, SLOT(insertAbove()) );
    connect(ui->tableView, SIGNAL(insertBelowAct()), this, SLOT(insertBelow()) );
    connect(ui->tableView, SIGNAL(removeAct()), this, SLOT(remove()) );
    connect(ui->prePageBtn, SIGNAL(clicked()), this, SLOT(prevPage()) );
    connect(ui->nextPageBtn, SIGNAL(clicked()), this, SLOT(nextPage()) );
    connect(ui->gotoPageBox, SIGNAL(currentIndexChanged(int)), this, SLOT(gotoPage(int)) );
    connect(ui->totalLayerLineEdit, SIGNAL(editingFinished()), this, SLOT(onTotalLayerEdited()) );

    // styling some buttons
    ui->prePageBtn->setFocusPolicy(Qt::NoFocus);
    ui->prePageBtn->setFixedSize(20, 20);
    ui->prePageBtn->setIconSize(ui->prePageBtn->size());
    ui->prePageBtn->setStyleSheet("border:none;");
    ui->prePageBtn->setIcon(QIcon(":/resources/images/left.png"));

    ui->nextPageBtn->setFocusPolicy(Qt::NoFocus);
    ui->nextPageBtn->setFixedSize(20, 20);
    ui->nextPageBtn->setIconSize(ui->nextPageBtn->size());
    ui->nextPageBtn->setStyleSheet("border:none;");
    ui->nextPageBtn->setIcon(QIcon(":/resources/images/right.png"));

    ui->gotoPageBox->setFixedWidth(60);
    ui->gotoPageBox->hide();
    ui->curPageLabel->hide();

    ui->heightLabel->setToolTip("meter");
    ui->gwtLabel->setToolTip("meter");


    // set the global stylesheet
    QFile file(":/resources/styles/stylesheet.css");
    if(file.open(QFile::ReadOnly)) {
      QString styleSheet = QLatin1String(file.readAll());
      this->setStyleSheet(styleSheet);
    }


    // validator for total layers, must be int between 0 and MAXLAYERS
    ui->totalLayerLineEdit->setValidator(new QIntValidator(0, MAXLAYERS, this));

    // when an element is selected, the whole row is selected
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

    // validator for height, must be double
    ui->totalHeight->setValidator(new QDoubleValidator(0.0,10000.0,2,ui->totalHeight));

    // change the total height if soil layer is edited
    connect(ui->totalHeight, SIGNAL(editingFinished()),this, SLOT(totalHeightChanged()) );
    connect(ui->gwtEdit, SIGNAL(editingFinished()), this, SLOT(on_gwtEdit_editingFinished()));

    // set limits for ground water table to be (0.0,1000)
    ui->gwtEdit->setValidator(new QDoubleValidator(0.0,10000.0,2,ui->gwtEdit));

    // height and count of layers not allowed to edit
    ui->totalHeight->setReadOnly(true);
    ui->totalLayerLineEdit->setReadOnly(true);















    //--------------------------------------------------------------------------------//
    // Init Mesher and Mesh View
    //--------------------------------------------------------------------------------//

    // init a 2D mesher
    mesher = new Mesher(srtFileName.toStdString());
    mesher->mesh2DColumn();


    // init the elements model to be used in mesh plot
    elementModel = new ElementModel;
    //std::sort(mesher->elements.begin(),mesher->elements.end(),
    //          [](const Quad &a, const Quad &b) { return  a.tag() > b.tag(); });
    elementModel->clear();
    elementModel->setWidth(mesher->eSizeH());
    elementModel->setTotalHeight(ui->totalHeight->text().toDouble());
    elementModel->setNodes(mesher->nodes);
    for(std::vector<int>::size_type n = mesher->elements.size(); n > 0; n--)
    {
        int tag = mesher->elements[n-1]->tag();
        int i = mesher->elements[n-1]->i();
        int j = mesher->elements[n-1]->j();
        int k = mesher->elements[n-1]->k();
        int l = mesher->elements[n-1]->l();
        double t = mesher->elements[n-1]->thickness();
        QString color = QString::fromStdString(mesher->elements[n-1]->color());
        bool isActive = false;
        elementModel->addElement("quad",tag,i,j,k,l,t,color,isActive);
    }
    elementModel->refresh();


    // add QQuickwidget for displaying mesh
    meshView = new QQuickView();

    meshView->rootContext()->setContextProperty("elements", elementModel);
    meshView->rootContext()->setContextProperty("sqlModel", ui->tableView->m_sqlModel);
    meshView->setSource(QUrl(QStringLiteral("qrc:/resources/ui/MeshView.qml")));
    QWidget *meshContainer = QWidget::createWindowContainer(meshView, this);






    resultsTab = new QTabWidget;
    resultsTab->setMinimumSize(200,layerTableHeight);
    resultsTab->setMaximumSize(200,1e5);
    resultsTab->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    resultsTab->addTab(meshContainer,"Mesh");

    postProcessor = new PostProcessor(outputDir);
    profiler = new ProfileManager(resultsTab, postProcessor ,this);   
    //connect(postProcessor, SIGNAL(updateFinished()), profiler, SLOT(onPostProcessorUpdated()));
    //postProcessor->update();


    // add the profile tab into the ui's layout
    ui->meshLayout->addWidget(resultsTab);

    // set stylesheet for the profile tab
    QFile profileCSSfile(":/resources/styles/profile.css");
    if(profileCSSfile.open(QFile::ReadOnly)) {
      QString styleSheet = QLatin1String(profileCSSfile.readAll());
      //QTabBar *configTabBar = resultsTab->findChild<QTabBar *>("qt_tabwidget_tabbar");
      resultsTab->setStyleSheet(styleSheet);
      profileCSSfile.close();
    }









    // add QQuickwidget for displaying soil layers
    QQuickView *plotView = new QQuickView();
    plotView->rootContext()->setContextProperty("designTableModel", ui->tableView);
    plotView->rootContext()->setContextProperty("soilModel", ui->tableView->m_sqlModel);
    plotContainer = QWidget::createWindowContainer(plotView, this);
    //plotContainer->setFixedSize(QSize(200, 800));
    //plotContainer->setMinimumSize(layerViewWidth,layerTableHeight);
    plotContainer->setMinimumSize(200,layerTableHeight);
    plotContainer->setMaximumSize(200,1e5);
    plotContainer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    //plotContainer->setFocusPolicy(Qt::TabFocus);
    plotView->setSource(QUrl(QStringLiteral("qrc:/resources/ui/PlotView.qml")));
    ui->soilColumnLayout->addWidget(plotContainer);
    //plotContainer->setWindowFlags(Qt::Widget | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowStaysOnTopHint);




    // hide some buttons
    ui->nextPageBtn->hide();
    ui->prePageBtn->hide();
    ui->reBtn->setVisible(false);
    ui->meshBtn->setVisible(true);

    // add some connections
    connect(ui->tableView->m_sqlModel, SIGNAL(thicknessEdited()), this, SLOT(onThicknessEdited()));
    connect(ui->tableView, SIGNAL(rowRemoved(int)), this, SLOT(on_rowRemoved(int)));
    //connect(ui->tableView, SIGNAL(pressed(const QModelIndex &)), ui->tableView, SLOT(on_activated(const QModelIndex &)));
    connect(this, SIGNAL(tableMoved()), this, SLOT(refresh()));

    resize(830 + 80, 350 + 40);
    //resize(830 + 80 + 280, 530 + 20);


    ui->tableView->m_sqlModel->deActivateAll();


    // init the tab manager on the right-bottom
    theTabManager = new TabManager(ui->tableView, elementModel ,this);
    theTabManager->updatePostProcessor(postProcessor);
    theTabManager->init(ui->tabWidget);
    //connect(ui->tableView, SIGNAL(cellClicked(const QModelIndex &)), theTabManager, SLOT(onTableViewClicked(const QModelIndex &)));
    connect(ui->tableView->m_sqlModel, SIGNAL(dataChanged(const QModelIndex&,const QModelIndex&)), theTabManager, SLOT(onTableViewUpdated(const QModelIndex&,const QModelIndex&)));
    connect(ui->gwtEdit, SIGNAL(editingFinished()), theTabManager, SLOT(onFEMTabEdited()));
    connect(this, SIGNAL(runBtnClicked()), theTabManager, SLOT(onRunBtnClicked()));
    // if configure tab changed, update SRT.json
    connect(theTabManager, SIGNAL(configTabUpdated()), this, SLOT(onConfigTabUpdated()));

    /*
    // init the dino view
    dinoView = new QWebEngineView(this);

    dinoView->load(QUrl::fromLocalFile(dinoHtmlName));
    dinoView->setVisible(false);
    //dinoView->setMinimumSize(layerTableWidth,300);
    //dinoView->setMaximumSize(1e5,1e5);
    //dinoView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    */

    // init GWT
    ui->gwtEdit->setText(QString::number(theTabManager->getGWTFromConfig()));

    // size control on the right
    ui->tableView->setMinimumSize(layerTableWidth,200);
    ui->tableView->setMaximumSize(1e5,1e5);
    ui->tableView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->tabWidget->setMinimumSize(layerTableWidth,300);
    ui->tabWidget->setMaximumSize(1e5,1e5);
    ui->tabWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);



    if (useJSONasInput)
    {
        elementModel->clear();
        elementModel->refresh();
        if(loadPreviousResults) loadFromJson(); // this is for frank,
        //because he doesn't like previous analysis loaded by default

        if(ui->tableView->m_sqlModel->rowCount()<1)
        {
            QList<QVariant> valueListRock;
            valueListRock << "Rock" << "-" << DefaultDensity << DefaultVs << DefaultEType << "-";
            ui->tableView->insertAt(valueListRock,0);

            ui->tableView->setTotalHeight(0);
            ui->totalHeight->setText("0");
            ui->totalLayerLineEdit->setText("1");


            QList<QVariant> valueList;
            valueList << "Layer 1" << DefaultThickness << DefaultDensity << DefaultVs << DefaultEType << DefaultESize << "#64B5F6";
            ui->tableView->insertAt(valueList,0);
            ui->totalHeight->setText(QString::number(DefaultThickness));
            ui->totalLayerLineEdit->setText("2");

            /*
            QList<QVariant> valueList2;
            valueList2 << "Layer 2" << DefaultThickness << DefaultDensity << DefaultVs << DefaultEType << DefaultESize << "green";
            ui->tableView->insertAt(valueList2,0);
            ui->totalHeight->setText(QString::number(DefaultThickness * 2.0));
            ui->totalLayerLineEdit->setText("3");

            QList<QVariant> valueList3;
            valueList2 << "Layer 3" << DefaultThickness << DefaultDensity << DefaultVs << DefaultEType << DefaultESize << "green";
            ui->tableView->insertAt(valueList3,0);
            ui->totalHeight->setText(QString::number(DefaultThickness * 3.0));
            ui->totalLayerLineEdit->setText("4");
            */
        }

    } else
    {
        // add a default layer
        if(ui->tableView->m_sqlModel->rowCount()<1)
        {
            QList<QVariant> valueListRock;
            valueListRock << "Rock" << "-" << DefaultDensity << DefaultVs << DefaultEType << "-";
            ui->tableView->insertAt(valueListRock,0);
            ui->tableView->setTotalHeight(0);
            ui->totalHeight->setText("0");
            ui->totalLayerLineEdit->setText("1");
        }
    }



    // init the opensess process
    openseesProcess = new QProcess(this);
    openseesProcess->setWorkingDirectory(analysisDir);
    //connect(openseesProcess, SIGNAL(readyReadStandardOutput()),this,SLOT(onOpenSeesFinished()));
    connect(openseesProcess, SIGNAL(readyReadStandardError()),this,SLOT(onOpenSeesFinished()));

    ui->rightLayout->setContentsMargins(0,0,0,0);
    ui->rightLayout->setSpacing(0);

    ui->progressBar->hide();
    connect( this, SIGNAL( signalProgress(int) ), ui->progressBar, SLOT( setValue(int) ) );

    connect( this, SIGNAL( signalInvokeInternalFEA() ), this, SLOT( onInternalFEAInvoked() ) );


    if(!QDir(outputDir).exists())
        QDir().mkdir(outputDir);

    //ui->tabWidget->hide();

}

void RockOutcrop::on_gwtEdit_editingFinished()
{
    // update SRT.json
    ui->reBtn->click();
}

void RockOutcrop::onConfigTabUpdated()
{
    // update SRT.json
    ui->reBtn->click();
}



bool  RockOutcrop::copyFiles(QString &destDir)
{
    QString fileName = "EVENT.json";
    QFile::copy(evtjFileName, destDir + "/" + fileName);
    return true;
}


void RockOutcrop::loadFromJson()
{


    QString in;
    QFile inputFile(srtFileName);
    if(inputFile.open(QFile::ReadOnly)) {
    //inputFile.open(QIODevice::ReadOnly | QIODevice::Text);
    in = inputFile.readAll();
    inputFile.close();
    }else{
        // if no input file provided add a default layer
        if(ui->tableView->m_sqlModel->rowCount()<1)
        {
            QList<QVariant> valueListRock;
            valueListRock << "Rock" << "-" << DefaultDensity << DefaultVs << DefaultEType << "-";
            ui->tableView->insertAt(valueListRock,0);
            ui->tableView->setTotalHeight(0);
            ui->totalHeight->setText("0");
            ui->totalLayerLineEdit->setText("1");


            QList<QVariant> valueList;
            valueList << "Layer 1" << DefaultThickness << DefaultDensity << DefaultVs << DefaultEType << DefaultESize << "#64B5F6";
            ui->tableView->insertAt(valueList,0);
            ui->totalHeight->setText(QString::number(DefaultThickness));
            ui->totalLayerLineEdit->setText("2");

        }
    }

    QJsonDocument indoc = QJsonDocument::fromJson(in.toUtf8());
    QJsonObject inobj = indoc.object();




    QJsonObject basicSettings = inobj["basicSettings"].toObject();
    QString groundMotion = basicSettings["groundMotion"].toString();
    if(groundMotion=="") groundMotion = "Input the path of a ground motion file. ";
    theTabManager->updateGMPath(groundMotion);
    QString OpenSeesPath = basicSettings["OpenSeesPath"].toString();
    if(OpenSeesPath=="") OpenSeesPath = "Input the full path of OpenSees excutable. ";
    theTabManager->updateOpenSeesPath(OpenSeesPath);
    QString slopex1 = QString::number(basicSettings["slopex1"].toDouble(), 'g', 16);
    QString slopex2 = QString::number(basicSettings["slopex2"].toDouble(), 'g', 16);


    QJsonArray soilLayers = inobj["soilProfile"].toObject()["soilLayers"].toArray();
    QJsonArray materials = inobj["materials"].toArray();
    for (int i=soilLayers.size()-1; i>=0; i--)
    {
        QJsonObject l = soilLayers[i].toObject();
        QString name = l["name"].toString();
        QString color = l["color"].toString();
        int id = l["id"].toInt();
        QJsonObject mat = materials[i].toObject();
        QString material = materials[i].toObject()["type"].toString();
        double Dr = l["Dr"].toDouble();
        double density = l["density"].toDouble();
        double eSize = l["eSize"].toDouble();
        double hPerm = l["hPerm"].toDouble();
        double vPerm = l["vPerm"].toDouble();
        double thickness = l["thickness"].toDouble();
        double vs = l["vs"].toDouble();

        if (i==soilLayers.size()-1)// Rock
        {
            QList<QVariant> valueListRock;
            valueListRock << "Rock" << "-" << density << vs << DefaultEType << "-";
            ui->tableView->insertAt(valueListRock,0);
        }else{
            QList<QVariant> valueList;
            if (color=="")
                color = QColor::fromRgb(QRandomGenerator::global()->generate()).name();
            valueList << name << thickness << density << vs << material << eSize << color;
            //ui->tableView->insertAtSilent(valueList,0);
            ui->tableView->insertAt(valueList,0);

        }



        theTabManager->updateLayerTab(l,mat);

    }







    ui->gwtEdit->setText(QString::number(basicSettings["groundWaterTable"].toDouble()));

    ui->totalLayerLineEdit->setText(QString::number(soilLayers.size()));

    theTabManager->updateConfigureTabFromOutside(slopex1, slopex2);


    ui->reBtn->click();



}


void RockOutcrop::cleanTable()
{
    for (int i=0; i<ui->tableView->m_sqlModel->rowCount();i++)
        ui->tableView->removeOneRow(0);

    ui->tableView->cleanTable();
    ui->gwtEdit->setText("0");
    ui->totalHeight->setText("0");
    ui->totalLayerLineEdit->setText("0");
    elementModel->clear();
    elementModel->refresh();
}



bool
RockOutcrop::outputAppDataToJSON(QJsonObject &jsonObject) {

    //
    // per API, need to add name of application to be called in AppLication
    // and all data to be used in ApplicationDate
    //

    jsonObject["EventClassification"]="Earthquake";
    jsonObject["Application"] = "Site Response";
    QJsonObject dataObj;
    jsonObject["ApplicationData"] = dataObj;
    return true;
}

bool
RockOutcrop::inputAppDataFromJSON(QJsonObject &jsonObject) {
    return true;
}

bool RockOutcrop::inputFromJSON(QJsonObject& inobj)
{
    cleanTable();
    cleanTable();
    cleanTable();

    QJsonObject basicSettings = inobj["basicSettings"].toObject();
    QString groundMotion = basicSettings["groundMotion"].toString();
    theTabManager->updateGMPath(groundMotion);
    QString OpenSeesPath = basicSettings["OpenSeesPath"].toString();
    theTabManager->updateOpenSeesPath(OpenSeesPath);
    QString slopex1 = QString::number(basicSettings["slopex1"].toDouble(), 'g', 16);
    QString slopex2 = QString::number(basicSettings["slopex2"].toDouble(), 'g', 16);


    QJsonArray soilLayers = inobj["soilProfile"].toObject()["soilLayers"].toArray();
    QJsonArray materials = inobj["materials"].toArray();
    for (int i=soilLayers.size()-1; i>=0; i--)
    {
        QJsonObject l = soilLayers[i].toObject();
        QString name = l["name"].toString();
        QString color = l["color"].toString();
        int id = l["id"].toInt();
        QJsonObject mat = materials[i].toObject();
        QString material = materials[i].toObject()["type"].toString();
        double Dr = l["Dr"].toDouble();
        double density = l["density"].toDouble();
        double eSize = l["eSize"].toDouble();
        double hPerm = l["hPerm"].toDouble();
        double vPerm = l["vPerm"].toDouble();
        double thickness = l["thickness"].toDouble();
        double vs = l["vs"].toDouble();

        if (i==soilLayers.size()-1)// Rock
        {
            QList<QVariant> valueListRock;
            valueListRock << "Rock" << "-" << density << vs << DefaultEType << "-";
            ui->tableView->insertAt(valueListRock,0);
        }else{
            QList<QVariant> valueList;
            if (color=="")
                color = QColor::fromRgb(QRandomGenerator::global()->generate()).name();
            valueList << name << thickness << density << vs << material << eSize << color;
            //ui->tableView->insertAtSilent(valueList,0);
            ui->tableView->insertAt(valueList,0);

        }



        theTabManager->updateLayerTab(l,mat);

    }







    ui->gwtEdit->setText(QString::number(basicSettings["groundWaterTable"].toDouble()));

    ui->totalLayerLineEdit->setText(QString::number(soilLayers.size()));

    theTabManager->updateConfigureTabFromOutside(slopex1, slopex2);


    ui->reBtn->click();

    return true;

}


bool RockOutcrop::inputFromJSON_old(QJsonObject& inobj) {

    cleanTable();
    cleanTable();

    /*
    qWarning() << inobj.value(QString("author"));
    qWarning() << inobj["author"];
    qWarning() << inobj["soilProfile"].toObject()["soilLayers"].toArray();
    */

    QJsonArray soilLayers = inobj["soilProfile"].toObject()["soilLayers"].toArray();
    QJsonArray materials = inobj["materials"].toArray();
    for (int i=soilLayers.size()-1; i>=0; i--)
    {
        QJsonObject l = soilLayers[i].toObject();
        QString name = l["name"].toString();
        QString color = l["color"].toString();
        int id = l["id"].toInt();
        QJsonObject mat = materials[i].toObject();
        QString material = materials[i].toObject()["type"].toString();
        double Dr = l["Dr"].toDouble();
        double density = l["density"].toDouble();
        double eSize = l["eSize"].toDouble();
        double hPerm = l["hPerm"].toDouble();
        double vPerm = l["vPerm"].toDouble();
        double thickness = l["thickness"].toDouble();
        double vs = l["vs"].toDouble();

        if (i==soilLayers.size()-1)// Rock
        {
            QList<QVariant> valueListRock;
            valueListRock << "Rock" << "-" << density << vs << DefaultEType << "-";
            ui->tableView->insertAt(valueListRock,0);
        }else{
            QList<QVariant> valueList;
            if (color=="")
                color = QColor::fromRgb(QRandomGenerator::global()->generate()).name();
            valueList << name << thickness << density << vs << material << eSize << color;
            ui->tableView->insertAtSilent(valueList,0);

        }

        theTabManager->updateLayerTab(l,mat);

    }



    QJsonObject basicSettings = inobj["basicSettings"].toObject();
    QString groundMotion = basicSettings["groundMotion"].toString();
    theTabManager->updateGMPath(groundMotion);
    QString OpenSeesPath = basicSettings["OpenSeesPath"].toString();
    theTabManager->updateOpenSeesPath(OpenSeesPath);

    ui->gwtEdit->setText(QString::number(basicSettings["groundWaterTable"].toDouble()));

    ui->totalLayerLineEdit->setText(QString::number(soilLayers.size()));


    ui->reBtn->click();

    return true;
}

RockOutcrop::~RockOutcrop()
{
    delete ui;

}

ElementModel* RockOutcrop::getElementModel() const
{
    return elementModel;
}

void RockOutcrop::onTotalLayerEdited()
{
    double previousHeight = ui->totalHeight->text().toDouble();
    int previousNumLayers = ui->tableView->totalSize();
    int newNumLayers = ui->totalLayerLineEdit->text().toInt();

    if(newNumLayers>MAXLAYERS | abs(newNumLayers)<1e-5)
    {
        ui->totalLayerLineEdit->setText(QString::number(previousNumLayers));
        QMessageBox::information(nullptr, "error", "Num of layers could not be zero or greater than "+QString::number(MAXLAYERS)+".");
    }
    else{

    if(previousNumLayers < newNumLayers)
    {
        QList<QVariant> emptyList;
        for (int i=0; i<(newNumLayers-previousNumLayers); i++)
            ui->tableView->insertAtEnd(emptyList);
    }
    if(previousNumLayers > newNumLayers & newNumLayers>=0)
    {
        for (int i=previousNumLayers; i>newNumLayers; i--)
            ui->tableView->removeOneRow(i-1);
    }
    if(previousNumLayers != newNumLayers)
    {
        qDebug() << "total layer changed from "<< previousNumLayers << " to " << newNumLayers;
        ui->tableView->divideByLayers(previousHeight,newNumLayers);
    }

    }

}



void RockOutcrop::on_meshBtn_clicked()
{

    //Q_UNUSED(checked);
    if (resultsTab->isVisible()){
        ui->meshBtn->setText(">");

        /*
        QPropertyAnimation *animation= new QPropertyAnimation(this, "windowOpacity");
        animation->setDuration(1000);
        animation->setStartValue(1);
        animation->setEndValue(0);
        animation->start();
        */

        if (resultsTab->currentIndex()<1)
            currentTabID = 0;
        else {
            currentTabID = resultsTab->currentIndex();
        }
        resultsTab->setCurrentIndex(1);

        QPropertyAnimation *anim1=new QPropertyAnimation(resultsTab, "pos");

        anim1->setDuration(300);

        anim1->setStartValue(resultsTab->pos());

        anim1->setEndValue(QPoint(resultsTab->pos().x()-210,resultsTab->pos().y()));

        anim1->setEasingCurve(QEasingCurve::OutBounce);

        anim1->start();

        QTimer::singleShot(400, this, SLOT(hideShowTab()));

        ui->coolSpacer->changeSize(0,0,QSizePolicy::Maximum,QSizePolicy::Maximum);




    }
    else{

        QPropertyAnimation *anim1=new QPropertyAnimation(resultsTab, "pos");

        anim1->setDuration(100);

        anim1->setStartValue(resultsTab->pos());

        anim1->setEndValue(QPoint(resultsTab->pos().x()+210,resultsTab->pos().y()));

        anim1->setEasingCurve(QEasingCurve::OutBounce);

        anim1->start();

        ui->meshBtn->setText("<");

        if(resultsTab->currentIndex()<1)
        {
            resultsTab->setCurrentIndex(1);
            resultsTab->setCurrentIndex(0);
        }
        resultsTab->show();

        ui->coolSpacer->changeSize(5,5,QSizePolicy::Maximum,QSizePolicy::Maximum);
        QTimer::singleShot(50, this, SLOT(showShowTab()));
    }


}

void RockOutcrop::showShowTab()
{
    if (currentTabID<1)
        resultsTab->setCurrentIndex(0);
    resultsTab->show();
}

void RockOutcrop::hideShowTab()
{
    resultsTab->hide();
}

void RockOutcrop::updateCtrl()
{

    ui->totalHeight->setText(QString::number(ui->tableView->totalHeight()));
    ui->gwtEdit->setText(QString::number(ui->tableView->getGWT()));

    int total = ui->tableView->totalSize();
    if(total>1)
    {
        //ui->totalLabel->setText( tr("Total Layers %1").arg(total) );
        ui->totalLabel->setText( tr("Total Layers") );
        ui->totalLayerLineEdit->setText(QString::number(total));
    }else{
        //ui->totalLabel->setText( tr("Total Layer %1").arg(total) );
        ui->totalLabel->setText( tr("Total Layer"));
        ui->totalLayerLineEdit->setText(QString::number(total));
    }

    /*
    int curPage = ui->tableView->currentPage();
    int totalPage = ui->tableView->totalPage();
    ui->curPageLabel->setText( tr("Page %1/%2").arg(curPage).arg(totalPage));

    ui->nextPageBtn->setEnabled(true);
    ui->prePageBtn->setEnabled(true);

    if ( totalPage == 1)
    {
        ui->nextPageBtn->setDisabled(true);
        ui->prePageBtn->setDisabled(true);
    }else{
        if(curPage >= totalPage)
        {
            ui->nextPageBtn->setDisabled(true);
        }else if(curPage == 1)
        {
            ui->prePageBtn->setDisabled(true);
        }else
        {

        }
    }
    */

}

void RockOutcrop::insertWithDialog()
{

}



void RockOutcrop::insert()// insert from dialog
{

    QList<QVariant> emptyList;
    ui->tableView->insertBelow(emptyList);
    /*
    InsertWindow* insertDlg = new InsertWindow(this);
    QList<QVariant> infos = ui->tableView->currentRowInfo();
    if( infos.count() >= 5)
        insertDlg->initInfo(infos);

    connect(insertDlg, SIGNAL(accepted(QList<QVariant>)),
            ui->tableView, SLOT(insert(QList<QVariant>)) );
    int ok = insertDlg->exec();
    if( ok )
    {
        updateCtrl();
    }
    insertDlg->deleteLater();

    int totalPage = ui->tableView->totalPage();
    int count = ui->gotoPageBox->count();
    if( count < totalPage)
    {
        ui->gotoPageBox->addItem(QString("%1").arg(count+1));
    }
    */

}

void RockOutcrop::insertAbove()
{
    QList<QVariant> emptyList;
    ui->tableView->insertAbove(emptyList);
    updateCtrl();
    ui->reBtn->click();
}

void RockOutcrop::insertBelow()
{
    QList<QVariant> emptyList;
    ui->tableView->insertBelow(emptyList);
    updateCtrl();
    //ui->reBtn->click();

}

void RockOutcrop::remove()
{
    ui->tableView->remove();
    updateCtrl();
    int totalPage = ui->tableView->totalPage();

    int count = ui->gotoPageBox->count();
    if( count > totalPage)
    {
        ui->gotoPageBox->removeItem(count-1);
    }
    ui->reBtn->click();
}

void RockOutcrop::gotoPage(int index)
{
    index++;
    ui->tableView->gotoPage(index);
    updateCtrl();
}

void RockOutcrop::nextPage()
{
    ui->tableView->nextPage();
    updateCtrl();
}

void RockOutcrop::prevPage()
{
    ui->tableView->previousPage();
    updateCtrl();
}

void RockOutcrop::on_delRowBtn_clicked()
{
    emit ui->tableView->removeAct();
}

void RockOutcrop::on_addRowBtn_clicked()
{
    if(ui->tableView->m_sqlModel->rowCount()<1)
    {
        QList<QVariant> valueListRock;
        valueListRock << "Rock" << "-" << DefaultDensity << DefaultVs << DefaultEType << "-";
        ui->tableView->insertAt(valueListRock,0);

        ui->tableView->setTotalHeight(0);
        ui->totalHeight->setText("0");
        ui->totalLayerLineEdit->setText("1");


        QList<QVariant> valueList;
        valueList << "Layer 1" << DefaultThickness << DefaultDensity << DefaultVs << DefaultEType << DefaultESize;
        ui->tableView->insertAt(valueList,0);
        ui->totalLayerLineEdit->setText("2");

    }else
    {
        emit ui->tableView->insertBelowAct();
    }
}

/**
 * when user input a new height value
 * @brief RockOutcrop::totalHeightChanged
 * @return
 */
void RockOutcrop::totalHeightChanged()
{
    if(abs(ui->totalHeight->text().toDouble() - ui->tableView->totalHeight())>1e-5)
    {
        ui->tableView->setTotalHeight(ui->totalHeight->text().toDouble());
        ui->tableView->divideByLayers(ui->totalHeight->text().toDouble(), ui->tableView->totalSize());
        //emit gwtChanged(QString::number(ui->tableView->getGWT()));
        qDebug()<<"height changed. => " << ui->totalHeight->text();
    }

}

void RockOutcrop::onThicknessEdited()
{

    ui->totalHeight->setText(QString::number(ui->tableView->totalHeight()));


    // move GWT to new position in UI
    double originalGWT = ui->tableView->getGWT();
    ui->gwtEdit->textChanged(QString::number(0));
    ui->gwtEdit->textChanged(QString::number(originalGWT));

    ui->reBtn->click();
    //updateMesh();

}

void RockOutcrop::on_rowRemoved(int row)
{
    Q_UNUSED(row);
    // move GWT to new position
    double originalGWT = ui->tableView->getGWT();
    ui->gwtEdit->textChanged(QString::number(0));
    ui->gwtEdit->textChanged(QString::number(originalGWT));
}



void RockOutcrop::on_gwtEdit_textChanged(const QString &newGWT)
{
    ui->tableView->setGWT(newGWT.toDouble());
}


bool RockOutcrop::outputToJSON(QJsonObject &root)
{
    on_reBtn_clicked();

    QString in;
    QFile inputFile(srtFileName);
    if(inputFile.open(QFile::ReadOnly)) {
    //inputFile.open(QIODevice::ReadOnly | QIODevice::Text);
    in = inputFile.readAll();
    inputFile.close();
    }else{
        // if not input file provided add a default layer
        if(ui->tableView->m_sqlModel->rowCount()<1)
        {
            QList<QVariant> valueListRock;
            valueListRock << "Rock" << "-" << DefaultDensity << DefaultVs << DefaultEType << "-";
            ui->tableView->insertAt(valueListRock,0);
            ui->tableView->setTotalHeight(0);
            ui->totalHeight->setText("0");
            ui->totalLayerLineEdit->setText("1");
        }
    }

    QJsonDocument indoc = QJsonDocument::fromJson(in.toUtf8());
    //qWarning() << indoc.isNull();
    if (indoc.isNull())
    {
        qWarning() << "SRT.json is missing.";
        return false;
    }
    else{
        root = indoc.object();
        return true;
    }


}

void RockOutcrop::on_killBtn_clicked()
{
        openseesProcess->kill();
        if (shark)
        {
            shark->setStopSignal();
            shark->requestInterruption();
            shark->quit();
            shark->wait();
            delete shark;
            shark = nullptr;
        }
        emit signalProgress(0);
        ui->progressBar->hide();

}

void RockOutcrop::on_reBtn_clicked()
{

    BonzaTableModel* tableModel = ui->tableView->m_sqlModel;

    QWidget* FEMtab = ui->tabWidget->widget(0);

    double GWT = ui->tableView->getGWT();

    json root = {
        {"name","Configuration of Site Response Analysis of A Demo Site"},
        {"author","SimCenter Site Response Tool"}
    };

    //TODO: this part is actually doing nothing?
    QList<QString> listFEMtab = {"eSizeH", "eSizeV", "RockVs", "RockDen", "DashpotCoeff", "VisC", "GMPath"  };
    for (int i = 0; i < listFEMtab.size(); ++i) {
        QString edtName = listFEMtab[i] ;
        FEMtab->findChild<QLineEdit*>(edtName)->text();
    }


    json basicSettings;

    bool is3D2D = FEMtab->findChild<QCheckBox*>("shakeDimCheck")->isChecked();
    basicSettings["simType"] = is3D2D ? "3D2D" : "2D1D";
    basicSettings["slopex1"] = FEMtab->findChild<QLineEdit*>("slopex1")->text().toDouble();
    basicSettings["slopex2"] = FEMtab->findChild<QLineEdit*>("slopex2")->text().toDouble();
    basicSettings["eSizeH"] = FEMtab->findChild<QLineEdit*>("eSizeH")->text().toDouble();
    basicSettings["eSizeV"] = FEMtab->findChild<QLineEdit*>("eSizeV")->text().toDouble();

    double rockVs_tmp = FEMtab->findChild<QLineEdit*>("RockVs")->text().toDouble();
    double rockDen_tmp = FEMtab->findChild<QLineEdit*>("RockDen")->text().toDouble();
    basicSettings["rockVs"] = rockVs_tmp;
    basicSettings["rockDen"] = rockDen_tmp;
    basicSettings["dashpotCoeff"] = rockVs_tmp*rockDen_tmp;// FEMtab->findChild<QLineEdit*>("DashpotCoeff")->text().toDouble();
    basicSettings["dampingCoeff"] = FEMtab->findChild<QLineEdit*>("VisC")->text().toDouble();
    basicSettings["groundMotion"] = FEMtab->findChild<QLineEdit*>("GMPath")->text().toStdString();
    basicSettings["OpenSeesPath"] = FEMtab->findChild<QLineEdit*>("openseesPath")->text().toStdString();
    basicSettings["groundWaterTable"] = GWT;
    root["basicSettings"] = basicSettings;

    json soilProfile = { };

    json layer, soilLayers, material, materials;

    //QString FEMString = tableModel->record(index.row()).value("FEM").toString();
    int numLayers = tableModel->rowCount();

    for (int i=0; i<numLayers; i++)
    {
        QList<QVariant> list = tableModel->getRowInfo(i);
        QStringList thisFEMList = list.at(FEM-2).toString().split(" ");
        if(thisFEMList.count()>1)
        {
            std::istringstream iss(list.at(FEM-2).toString().toStdString());
            std::vector<std::string> pars((std::istream_iterator<std::string>(iss)),
                                          std::istream_iterator<std::string>());
            double eSize = atof(pars[0].c_str());
            int id = i;

            int DrInd=0, hPermInd=0, vPermInd=0, uBulkInd=0, voidInd=0;
            if(list.at(MATERIAL-2).toString()=="Elastic")
            {
                DrInd = 4; hPermInd = 6; vPermInd = 7; uBulkInd = 10; voidInd = 5;
            }else if(list.at(MATERIAL-2).toString()=="PM4Sand")
            {
                DrInd = 1; hPermInd = 25; vPermInd = 26; uBulkInd = 27; voidInd = 28;
            }else if(list.at(MATERIAL-2).toString()=="PM4Silt")
            {
                DrInd = 1; hPermInd = 27; vPermInd = 28; uBulkInd = 29; voidInd = 30;
            }else if(list.at(MATERIAL-2).toString()=="PIMY")
            {//TODO: double check
                DrInd = 1; hPermInd = 12; vPermInd = 13; uBulkInd = 14; voidInd = 15;
            }else if(list.at(MATERIAL-2).toString()=="PDMY")
            {//TODO: double check
                DrInd = 1; hPermInd = 24; vPermInd = 25; uBulkInd = 26; voidInd = 27;
            }else if(list.at(MATERIAL-2).toString()=="PDMY02")
            {//TODO: double check
                DrInd = 1; hPermInd = 26; vPermInd = 27; uBulkInd = 28; voidInd = 29;
            }else if(list.at(MATERIAL-2).toString()=="ManzariDafalias")
            {//TODO: double check
                DrInd = 1; hPermInd = 20; vPermInd = 21; uBulkInd = 22; voidInd = 23;
            }else if(list.at(MATERIAL-2).toString()=="J2Bounding")
            {//TODO: double check
                DrInd = 1; hPermInd = 10; vPermInd = 11; uBulkInd = 12; voidInd = 13;
            }


            if(!list.at(LAYERNAME-2).toString().toStdString().compare("Rock"))
            {
                double rockVsTmp = list.at(VS-2).toDouble();
                double rockDenTmp = list.at(DENSITY-2).toDouble();
                root["basicSettings"]["rockVs"] = rockVsTmp;
                root["basicSettings"]["rockDen"] = rockDenTmp;
                root["basicSettings"]["dashpotCoeff"] = rockVsTmp*rockDenTmp;

                double esizeH_tmp = FEMtab->findChild<QLineEdit*>("eSizeH")->text().toDouble();
                double area_tmp = is3D2D ? esizeH_tmp*esizeH_tmp : esizeH_tmp*1.0;
                root["basicSettings"]["dampingCoeff"] = area_tmp*rockVsTmp*rockDenTmp;
            }

            layer = {
                {"id",id+1},
                {"name",list.at(LAYERNAME-2).toString().toStdString()},
                {"thickness",list.at(THICKNESS-2).toDouble()},
                {"density",list.at(DENSITY-2).toDouble()},
                {"vs",list.at(VS-2).toDouble()},
                {"material", id+1},
                {"color",list.at(COLOR-2).toString().toStdString()},
                {"eSize",eSize},
                {"Dr",thisFEMList.at(DrInd).toDouble()},
                {"hPerm",thisFEMList.at(hPermInd).toDouble()},
                {"vPerm",thisFEMList.at(vPermInd).toDouble()},
                {"uBulk",thisFEMList.at(uBulkInd).toDouble()},
                {"void",thisFEMList.at(voidInd).toDouble()},
            };
            material =  createMaterial(i+1, list.at(MATERIAL-2).toString().toStdString(),list.at(FEM-2).toString().toStdString());
            materials.push_back(material);
            soilLayers.push_back(layer);
        }
    }







    soilProfile["soilLayers"] = soilLayers;

    root["soilProfile"]=soilProfile;
    root["materials"]=materials;

    // write prettified JSON to another file
    QString file_name = srtFileName;//"SRT.json";
    //QString file_name = QFileDialog::getSaveFileName(this, tr("Choose Path for saving the analysis"), "", tr("Config Files (*.json)"));


    if (!file_name.isNull())
    {
        std::ofstream o(file_name.toStdString());
        o << std::setw(4) << root << std::endl;
    } else {
        QMessageBox::information(this, "error", "Failed to get file name.");
    }


    updateMesh(root);


}

void RockOutcrop::updateMesh(json &j)
{
    mesher->mesh2DColumnFromJson(j);
    elementModel->clear();
    elementModel->setTotalHeight(ui->totalHeight->text().toDouble());
    elementModel->setNodes(mesher->nodes);

    //ElementModel* newElementModel = new ElementModel;
    for(std::vector<int>::size_type n = mesher->elements.size(); n > 0; n--)
    {
        //Quad *ele = it;
        int tag = mesher->elements[n-1]->tag();
        int i = mesher->elements[n-1]->i();
        int j = mesher->elements[n-1]->j();
        int k = mesher->elements[n-1]->k();
        int l = mesher->elements[n-1]->l();
        double t = mesher->elements[n-1]->thickness();
        QString color = QString::fromStdString(mesher->elements[n-1]->color());
        bool isActive = false;
        elementModel->addElement("quad",tag,i,j,k,l,t,color,isActive);
    }
    elementModel->refresh();
}

int RockOutcrop::checkDimension()
{
    if(theTabManager->is2Dmotion())//2D motion set in the configure tab
    {
        // check if there is PM4Sand or PM4Silt

        int layerContaining2DOnlyModel = ui->tableView->m_sqlModel->has2DOnlyModel();
        if (layerContaining2DOnlyModel>0)
        {
            dimMsg = "Layer "+ QString::number(layerContaining2DOnlyModel)+" contains material which can only be used in 2D simulation.";
            return -1;
        }
        return 3; // 3D column
    }else {
        // check nd
        int layerContaining3DOnlyModel = ui->tableView->m_sqlModel->has3DOnlyModel();
        if (layerContaining3DOnlyModel>0)
        {
            dimMsg = "Layer "+ QString::number(layerContaining3DOnlyModel)+" contains material (J2Bounding) which can only be used in 3D simulation.";
            return -1;
        }

        return 2; // 2D column
    }

}


void RockOutcrop::on_runBtn_clicked()
{
    GoogleAnalytics::ReportLocalRun();
    //cleanTable();cleanTable();

    int numLayers = ui->totalLayerLineEdit->text().toInt();
    int simDim = checkDimension();

    if (numLayers<=1)
    {
        QMessageBox::information(this,tr("Soil err"), "You need to add at least one soil layer.", tr("OK."));

    } else if(simDim < 0)
    {
        QMessageBox::information(this,tr("Dimension err"), dimMsg, tr("OK."));
    }
    else{
        QString openseespath = theTabManager->openseespath();;//"OpenSees";//theTabManager->openseespath();
        //QProcess* openseesTesterProcess = new QProcess(this);
        //bool osrun = openseesProcess->startDetached(openseespath);
        //osrun = true;
        //int osrun = openseesProcess->execute("bash", QStringList() << "-c" <<  openseespath);




        QString rockmotionpath =  theTabManager->rockmotionpath();
        bool openseesEmpty = openseespath=="" || openseespath=="Input the full path of OpenSees excutable.";
        bool rockEmpty = rockmotionpath=="" || rockmotionpath=="Input the path of a ground motion file.";

        QFile rocMojsonFile(rockmotionpath);

        if(rockEmpty || !rocMojsonFile.exists())
        {   // didn't find rock motion file
            int msg = QMessageBox::information(this,tr("Path error"), "You need to specify rock motion file's path in the configure tab.", tr("OK, I'll do it."), tr("Tutorial"));
            if(msg==1)
            {
                QString link = "https://nheri-simcenter.github.io/s3hark-Documentation/common/user_manual/quickstart/quickstart.html";
                QDesktopServices::openUrl(QUrl(link));
            }
            theTabManager->getTab()->setCurrentIndex(0);

        }else{

            QFile openseesExefile(openseespath);
            // build tcl file
            ui->reBtn->click();

            if(!QDir(outputDir).exists())
                QDir().mkdir(outputDir);

            ui->progressBar->show();

            if(openseesExefile.exists())
            {   // do FEA in opensees

                SiteResponse *srt = new SiteResponse(srtFileName.toStdString(),
                                                     analysisDir.toStdString(),outputDir.toStdString(),femLog.toStdString());
                if (simDim==3)
                {
                    theTabManager->setSimulationD(3);
                    srt->buildTcl3D();
                }
                else
                {
                    theTabManager->setSimulationD(2);
                    srt->buildTcl();
                }

                openseesProcess->start(openseespath,QStringList()<<tclName);
                openseesErrCount = 1;
                emit runBtnClicked();

            } else {// internal FEA
                if (simDim==3) theTabManager->setSimulationD(3);
                else theTabManager->setSimulationD(2);


                emit signalInvokeInternalFEA();
                //emit runBtnClicked();

                /*
                SiteResponse *srt = new SiteResponse(srtFileName.toStdString(),
                                                     analysisDir.toStdString(),
                                                     outputDir.toStdString(),
                                                     femLog.toStdString(),
                                                     m_callbackptr );
                if (simDim==3)
                {
                    theTabManager->setSimulationD(3);
                    srt->run3D();
                }
                else
                {
                    theTabManager->setSimulationD(2);
                    srt->run();
                }
                */

                /*
                int msg = QMessageBox::information(this,tr("Path error"), "Please specify the correct path of OpenSees in the Configure tab.", tr("OK, I'll do it."), tr("Tutorial"));
                //QMessageBox::information(this,tr("Path error"), "OpenSees is not found in your environment. Analysis didn't run", tr("OK."));
                if(msg==1)
                {
                    QString link = "https://nheri-simcenter.github.io/s3hark/#/start";
                    QDesktopServices::openUrl(QUrl(link));
                }
                ui->progressBar->hide();
                theTabManager->getTab()->setCurrentIndex(0);
                */

                //int msg = QMessageBox::information(this,tr("FEA Running"), "s3hark started.", tr("OK"));
                //QMessageBox::information(this,tr("Path error"), "OpenSees is not found in your environment. Analysis didn't run", tr("OK."));


            }


        }

    }
}


// callback setups
bool RockOutcrop::refreshRun(double step) {
    int p = int(floor(step));
    qDebug() << step;
    if(step<100.)
    {
        emit signalProgress(p);
    }
    else
    {
        /*
        theTabManager->getTab()->setCurrentIndex(2);
        theTabManager->setGMViewLoaded();
        theTabManager->reFreshGMTab();
        theTabManager->reFreshGMView();

        resultsTab->setCurrentIndex(1);

        postProcessor = new PostProcessor(outputDir);
        profiler->updatePostProcessor(postProcessor);
        theTabManager->updatePostProcessor(postProcessor);
        connect(postProcessor, SIGNAL(updateFinished()), profiler, SLOT(onPostProcessorUpdated()));
        postProcessor->update();

        emit signalProgress(100);
        ui->progressBar->hide();
        QMessageBox::information(this,tr("s3hark Information"), "Analysis in s3hark is done.", tr("OK."));
        */

        on_killBtn_clicked();

        postProcessor = new PostProcessor(outputDir);
        profiler->updatePostProcessor(postProcessor);
        theTabManager->updatePostProcessor(postProcessor);
        connect(postProcessor, SIGNAL(updateFinished()), profiler, SLOT(onPostProcessorUpdated()));
        postProcessor->update();

        theTabManager->setGMViewLoaded();
        theTabManager->reFreshGMTab();

        theTabManager->reFreshGMView();
        theTabManager->getTab()->setCurrentIndex(2);
        resultsTab->setCurrentIndex(1);

        QMessageBox::information(this,tr("s3hark Information"), "Analysis in s3hark is done.", tr("OK."));

        emit signalProgress(100);
        ui->progressBar->hide();



    }
    return true;
}

void RockOutcrop::onInternalFEAInvoked()
{

    shark = new SSSharkThread(srtFileName,analysisDir,outputDir,femLog, this);
    connect(shark,SIGNAL(updateProgress(double)), this, SLOT(onInternalFEAUpdated(double)));
    shark->start();
}


void RockOutcrop::onOpenSeesFinished()
{

    //writeSurfaceMotion();
    QString str_err = openseesProcess->readAllStandardError();

    if(openseesErrCount==1)
    {
        if(str_err.contains("Site response analysis is finished."))
        {

            //qDebug() << "opensees says:" << str_err;
            openseesErrCount = 2;

            postProcessor = new PostProcessor(outputDir);
            profiler->updatePostProcessor(postProcessor);
            theTabManager->updatePostProcessor(postProcessor);
            connect(postProcessor, SIGNAL(updateFinished()), profiler, SLOT(onPostProcessorUpdated()));
            postProcessor->update();

            theTabManager->setGMViewLoaded();
            theTabManager->reFreshGMTab();

            theTabManager->reFreshGMView();
            theTabManager->getTab()->setCurrentIndex(2);
            resultsTab->setCurrentIndex(1);

            QMessageBox::information(this,tr("OpenSees Information"), "Analysis is done.", tr("I know."));




            emit signalProgress(100);
            ui->progressBar->hide();

        }else{
            QRegExp rxlen("(.+)(%)");
            int pos = rxlen.indexIn(str_err);
            if (pos > -1) {
                QString value = rxlen.cap(1);
                int step = int(ceil(value.toDouble()));
                if(step>0)
                    emit signalProgress(step);
            }


        }
    }

}



void RockOutcrop::writeSurfaceMotion()
{

}

json RockOutcrop::createMaterial(int tag, std::string matType, std::string parameters)
{
    json mat = {
        {"id", tag},
        {"type", matType}
    };
    std::istringstream iss(parameters);
    std::vector<std::string> pars((std::istream_iterator<std::string>(iss)),
                                     std::istream_iterator<std::string>());
    if (!matType.compare("Elastic"))
    {
        double eSize = atof(pars[0].c_str());
        double E = atof(pars[1].c_str());
        double poisson = atof(pars[2].c_str());
        double density = atof(pars[3].c_str());

        mat["E"] = E;
        mat["poisson"] = poisson;
        mat["density"] = density;
    } else if (!matType.compare("PM4Sand"))
    {
        mat["Dr"] = atof(pars[1].c_str());
        mat["G0"] = atof(pars[2].c_str());
        mat["hpo"] = atof(pars[3].c_str());
        mat["Den"] = atof(pars[4].c_str());
        mat["P_atm"] = atof(pars[5].c_str());
        mat["h0"] = atof(pars[6].c_str());
        mat["emax"] = atof(pars[7].c_str());
        mat["emin"] = atof(pars[8].c_str());
        mat["nb"] = atof(pars[9].c_str());
        mat["nd"] = atof(pars[10].c_str());
        mat["Ado"] = atof(pars[11].c_str());
        mat["z_max"] = atof(pars[12].c_str());
        mat["cz"] = atof(pars[13].c_str());
        mat["ce"] = atof(pars[14].c_str());
        mat["phic"] = atof(pars[15].c_str());
        mat["nu"] = atof(pars[16].c_str());
        mat["cgd"] = atof(pars[17].c_str());
        mat["cdr"] = atof(pars[18].c_str());

        mat["ckaf"] = atof(pars[19].c_str());
        mat["Q"] = atof(pars[20].c_str());
        mat["R"] = atof(pars[21].c_str());
        mat["m"] = atof(pars[22].c_str());
        mat["Fsed_min"] = atof(pars[23].c_str());
        mat["p_sedo"] = atof(pars[24].c_str());
    } else if (!matType.compare("PM4Silt"))
    {
        mat["Dr"] = atof(pars[1].c_str());
        mat["S_u"] = atof(pars[2].c_str());
        mat["Su_Rat"] = atof(pars[3].c_str());
        mat["G_o"] = atof(pars[4].c_str());
        mat["h_po"] = atof(pars[5].c_str());
        mat["Den"] = atof(pars[6].c_str());
        mat["Su_factor"] = atof(pars[7].c_str());
        mat["P_atm"] = atof(pars[8].c_str());
        mat["nu"] = atof(pars[9].c_str());
        mat["nG"] = atof(pars[10].c_str());
        mat["h0"] = atof(pars[11].c_str());
        mat["eInit"] = atof(pars[12].c_str());
        mat["lambda"] = atof(pars[13].c_str());
        mat["phicv"] = atof(pars[14].c_str());
        mat["nb_wet"] = atof(pars[15].c_str());
        mat["nb_dry"] = atof(pars[16].c_str());
        mat["nd"] = atof(pars[17].c_str());
        mat["Ado"] = atof(pars[18].c_str());

        mat["ru_max"] = atof(pars[19].c_str());
        mat["z_max"] = atof(pars[20].c_str());
        mat["cz"] = atof(pars[21].c_str());
        mat["ce"] = atof(pars[22].c_str());
        mat["cgd"] = atof(pars[23].c_str());
        mat["ckaf"] = atof(pars[24].c_str());

        mat["m_m"] = atof(pars[25].c_str());
        mat["CG_consol"] = atof(pars[26].c_str());

    } else if (!matType.compare("PIMY"))
    {
        mat["Dr"] = atof(pars[1].c_str());
        mat["nd"] = atof(pars[2].c_str());
        mat["rho"] = atof(pars[3].c_str());
        mat["refShearModul"] = atof(pars[4].c_str());
        mat["refBulkModul"] = atof(pars[5].c_str());
        mat["cohesi"] = atof(pars[6].c_str());
        mat["peakShearStra"] = atof(pars[7].c_str());
        mat["frictionAng"] = atof(pars[8].c_str());
        mat["refPress"] = atof(pars[9].c_str());
        mat["pressDependCoe"] = atof(pars[10].c_str());
        mat["noYieldSurf"] = atof(pars[11].c_str());

    } else if (!matType.compare("PDMY"))
    {
        mat["Dr"] = atof(pars[1].c_str());
        mat["nd"] = atof(pars[2].c_str());
        mat["rho"] = atof(pars[3].c_str());
        mat["refShearModul"] = atof(pars[4].c_str());
        mat["refBulkModul"] = atof(pars[5].c_str());
        mat["frictionAng"] = atof(pars[6].c_str());
        mat["peakShearStra"] = atof(pars[7].c_str());
        mat["refPress"] = atof(pars[8].c_str());
        mat["pressDependCoe"] = atof(pars[9].c_str());
        mat["PTAng"] = atof(pars[10].c_str());
        mat["contrac"] = atof(pars[11].c_str());
        mat["dilat1"] = atof(pars[12].c_str());
        mat["dilat2"] = atof(pars[13].c_str());
        mat["liquefac1"] = atof(pars[14].c_str());
        mat["liquefac2"] = atof(pars[15].c_str());
        mat["liquefac3"] = atof(pars[16].c_str());
        mat["e"] = atof(pars[17].c_str());
        mat["cs1"] = atof(pars[18].c_str());

        mat["cs2"] = atof(pars[19].c_str());
        mat["cs3"] = atof(pars[20].c_str());
        mat["pa"] = atof(pars[21].c_str());
        mat["c"] = atof(pars[22].c_str());
        mat["noYieldSurf"] = atof(pars[23].c_str());


    } else if (!matType.compare("PDMY02"))
    {
        mat["Dr"] = atof(pars[1].c_str());
        mat["nd"] = atof(pars[2].c_str());
        mat["rho"] = atof(pars[3].c_str());
        mat["refShearModul"] = atof(pars[4].c_str());
        mat["refBulkModul"] = atof(pars[5].c_str());
        mat["frictionAng"] = atof(pars[6].c_str());
        mat["peakShearStra"] = atof(pars[7].c_str());
        mat["refPress"] = atof(pars[8].c_str());
        mat["pressDependCoe"] = atof(pars[9].c_str());
        mat["PTAng"] = atof(pars[10].c_str());
        mat["contrac1"] = atof(pars[11].c_str());
        mat["contrac3"] = atof(pars[12].c_str());
        mat["dilat1"] = atof(pars[13].c_str());
        mat["dilat3"] = atof(pars[14].c_str());
        mat["contrac2"] = atof(pars[15].c_str());
        mat["dilat2"] = atof(pars[16].c_str());
        mat["liquefac1"] = atof(pars[17].c_str());
        mat["liquefac2"] = atof(pars[18].c_str());

        mat["e"] = atof(pars[19].c_str());
        mat["cs1"] = atof(pars[20].c_str());
        mat["cs2"] = atof(pars[21].c_str());
        mat["cs3"] = atof(pars[22].c_str());
        mat["pa"] = atof(pars[23].c_str());
        mat["c"] = atof(pars[24].c_str());
        mat["noYieldSurf"] = atof(pars[25].c_str());


    } else if (!matType.compare("ManzariDafalias"))
    {
        mat["Dr"] = atof(pars[1].c_str());
        mat["G0"] = atof(pars[2].c_str());
        mat["nu"] = atof(pars[3].c_str());
        mat["e_init"] = atof(pars[4].c_str());
        mat["Mc"] = atof(pars[5].c_str());
        mat["c"] = atof(pars[6].c_str());
        mat["lambda_c"] = atof(pars[7].c_str());
        mat["e0"] = atof(pars[8].c_str());
        mat["ksi"] = atof(pars[9].c_str());
        mat["P_atm"] = atof(pars[10].c_str());
        mat["m"] = atof(pars[11].c_str());
        mat["h0"] = atof(pars[12].c_str());
        mat["ch"] = atof(pars[13].c_str());
        mat["nb"] = atof(pars[14].c_str());
        mat["A0"] = atof(pars[15].c_str());
        mat["nd"] = atof(pars[16].c_str());
        mat["z_max"] = atof(pars[17].c_str());
        mat["cz"] = atof(pars[18].c_str());
        mat["Den"] = atof(pars[19].c_str());

    } else if (!matType.compare("J2Bounding"))
    {
        mat["Dr"] = atof(pars[1].c_str());
        mat["G"] = atof(pars[2].c_str());
        mat["K"] = atof(pars[3].c_str());
        mat["su"] = atof(pars[4].c_str());
        mat["rho"] = atof(pars[5].c_str());
        mat["h"] = atof(pars[6].c_str());
        mat["m"] = atof(pars[7].c_str());
        mat["k_in"] = atof(pars[8].c_str());
        mat["beta"] = atof(pars[9].c_str());

    }
    return mat;
}

void RockOutcrop::refresh()
{
    //ui->tableView->move(0,50);
    QTime dieTime= QTime::currentTime().addMSecs(500);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    //resize(ui->centralWidget->width(),ui->centralWidget->height()+1);



    /*
    QPropertyAnimation *animation1= new QPropertyAnimation(ui->tableView,"geometry");
    animation1->setDuration(500);
    animation1->setStartValue(ui->tableView->geometry());
    animation1->setEndValue(QRect(0, 0, ui->tableView->width(), ui->tableView->height()));
    animation1->start();
    */


    //ui->tableView->setVisible(true);


}

bool RockOutcrop::copyDir(const QDir& from, const QDir& to, bool cover=true)
{

    if (!to.exists())
        {
        if (!to.mkdir(to.absolutePath()))
            return false;
    } else {

    }

    QFileInfoList fileInfoList = from.entryInfoList();
    foreach(QFileInfo fileInfo, fileInfoList)
    {
        if (fileInfo.fileName() == "." || fileInfo.fileName() == "..")
            continue;

        if (fileInfo.isDir()){
            if (!copyDir(fileInfo.filePath(), to.filePath(fileInfo.fileName())))
                return false;
        }
        else{
            if (cover && to.exists(fileInfo.fileName())){
                //to.remove(fileInfo.fileName());
            }
            if (!QFile::copy(fileInfo.filePath(), to.filePath(fileInfo.fileName()))){
                return false;
            }
        }
    }
    return true;
}




