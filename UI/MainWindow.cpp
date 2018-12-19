#include <QQuickView>
#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "InsertWindow.h"
#include <QQmlContext>

#include <QTime>

#include <QtQuick/qquickitem.h>
#include <QtQuick/qquickview.h>

#include <QStringList>

#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QThread>

#include <QTabWidget>

#include "SiteResponse.h"

#include <QUiLoader>

#include <QFileInfo>
#include <QMessageBox>


#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <iomanip>



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    // random seed
    unsigned uintTime = static_cast<unsigned>(QTime::currentTime().msec());
    qsrand(uintTime);


    ui->setupUi(this);
    updateCtrl();

    QStringList pages;
    int totalPage = ui->tableView->totalPage();
    for( int i = 1; i <= totalPage; ++i)
        pages.append( QString("%1").arg(i) );
    ui->gotoPageBox->addItems(pages);

    connect(ui->tableView, SIGNAL(insertAct()), this, SLOT(insertAbove()) );
    connect(ui->tableView, SIGNAL(insertBelowAct()), this, SLOT(insertBelow()) );
    connect(ui->tableView, SIGNAL(removeAct()), this, SLOT(remove()) );
    //connect(ui->styleBtn, SIGNAL(clicked(bool)), ui->tableView, SLOT(styleView(bool)) );
    //ui->styleBtn->setVisible(false);

    connect(ui->prePageBtn, SIGNAL(clicked()), this, SLOT(prevPage()) );
    connect(ui->nextPageBtn, SIGNAL(clicked()), this, SLOT(nextPage()) );
    //connect(ui->gotoPageBtn, SIGNAL(clicked()), this, SLOT(gotoPage()) );
    connect(ui->gotoPageBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(gotoPage(int)) );

    connect(ui->totalLayerLineEdit, SIGNAL(editingFinished()), this, SLOT(onTotalLayerEdited()) );


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

    // margins of centralWidget
    centralWidget()->layout()->setContentsMargins(0, 0, 0, 0);
    statusBar()->hide();
    ui->mainToolBar->hide();


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

    //connect(this, SIGNAL(gwtChanged(const QString)), this, SLOT(on_gwtEdit_textChanged(const QString)));

    ui->gwtEdit->setValidator(new QDoubleValidator(0.0,10000.0,2,ui->gwtEdit));

    // add a default layer
    if(ui->tableView->m_sqlModel->rowCount()<1)
    {
        QList<QVariant> valueList;
        valueList << "Layer 1" << "3" << "" << "" << "" << "";
        ui->tableView->insertAt(valueList,0);
        ui->tableView->setTotalHeight(3);
        ui->totalHeight->setText("3");
        ui->totalLayerLineEdit->setText("1");
    }

    // add QQuickwidget for displaying soil layers
    QQuickView *plotView = new QQuickView();
    plotView->rootContext()->setContextProperty("designTableModel", ui->tableView);
    plotView->rootContext()->setContextProperty("soilModel", ui->tableView->m_sqlModel);

    //QWidget *
    plotContainer = QWidget::createWindowContainer(plotView, this);
    //plotContainer->setFixedSize(QSize(200, 800));
    plotContainer->setMinimumSize(layerViewWidth,layerTableHeight);
    plotContainer->setMaximumSize(layerViewWidth,layerTableHeight);
    plotContainer->setFocusPolicy(Qt::TabFocus);
    plotView->setSource(QUrl(QStringLiteral("qrc:/resources/ui/plotView.qml")));
    ui->plotView_verticalLayout->addWidget(plotContainer);

    /*
    // add QQuickwidget for displaying mesh
    QQuickView *meshView = new QQuickView();
    //meshView->rootContext()->setContextProperty("designTableModel", ui->tableView);
    //meshView->rootContext()->setContextProperty("soilModel", ui->tableView->m_sqlModel);
    QWidget *meshContainer = QWidget::createWindowContainer(meshView, this);

    meshContainer->setMinimumSize(meshViewWidth,layerTableHeight);
    meshContainer->setMaximumSize(meshViewWidth,layerTableHeight);
    meshContainer->setFocusPolicy(Qt::TabFocus);
    meshView->setSource(QUrl(QStringLiteral("qrc:/resources/ui/MeshView.qml")));
    ui->meshView_verticalLayout->addWidget(meshContainer);

    connect(ui->meshBtn, SIGNAL(clicked()), this, SLOT(on_meshBtn_clicked(bool)) );

    ui->meshBtn->setVisible(true);

    ui->groupBox_Mesh->setVisible(false);
    */



    connect(ui->tableView->m_sqlModel, SIGNAL(thicknessEdited()), this, SLOT(on_thickness_edited()));

    //resize(830 + 80, 350 + 40);
    resize(830 + 80, 530 + 20);

    ui->tableView->m_sqlModel->deActivateAll();

    connect(ui->tableView, SIGNAL(rowRemoved(int)), this, SLOT(on_rowRemoved(int)));

    //connect(ui->tableView, SIGNAL(pressed(const QModelIndex &)), ui->tableView, SLOT(on_activated(const QModelIndex &)));


    connect(this, SIGNAL(tableMoved()), this, SLOT(refresh()));


    // material dialog
    /*
    QQuickView *matView = new QQuickView();
    //matView->rootContext()->setContextProperty("designTableModel", ui->tableView);
    //matView->rootContext()->setContextProperty("soilModel", ui->tableView->m_sqlModel);
    matContainer = QWidget::createWindowContainer(matView, this);
    matContainer->setMinimumSize(layerTableWidth-10,100);
    matContainer->setMaximumSize(layerTableWidth-10,100);
    matContainer->setFocusPolicy(Qt::TabFocus);
    matView->setSource(QUrl(QStringLiteral("qrc:/resources/ui/PM4Sand.qml")));
    ui->materialLayout->addWidget(matContainer);
    matContainer->hide();
    */

    dinoView = new QWebEngineView(this);
    dinoView->load(QUrl::fromLocalFile(QFileInfo("resources/ui/DinoRun/index.html").absoluteFilePath()));
    dinoView->setVisible(false);


    //ui->reBtn->setVisible(false);

    /*

    dinoView = new QWebEngineView(this);
    dinoView->load(QUrl("file:////Users/simcenter/Codes/Sandbox/SRT/SiteResponseTool/resources/ui/DinoRun/index.html"));
    //view->show();
    //dinoView->setMinimumHeight(400);
    //dinoView->setMaximumHeight(400);
    //ui->materialLayout->addWidget(dinoView);
    dinoView->setVisible(false);
    //ui->reBtn->setVisible(false);
    ui->tabWidget->addTab(dinoView, "Run");
    */

/*
    // add QQuickwidget for displaying soil layers
    QQuickView *FEMView = new QQuickView();
    FEMView->rootContext()->setContextProperty("designTableModel", ui->tableView);
    FEMView->rootContext()->setContextProperty("soilModel", ui->tableView->m_sqlModel);
    QWidget *FEMContainer = QWidget::createWindowContainer(FEMView, this);
    //plotContainer->setFixedSize(QSize(200, 800));
    FEMContainer->setMinimumSize(layerViewWidth,180);
    FEMContainer->setMaximumSize(layerViewWidth,180);
    FEMContainer->setFocusPolicy(Qt::TabFocus);
    FEMView->setSource(QUrl(QStringLiteral("qrc:/resources/ui/FEMView.qml")));
    ui->tabWidget->addTab(FEMContainer,"FEM");
    */


/*
    InsertWindow* insertDlg = new InsertWindow(this);
    QList<QVariant> infos = ui->tableView->currentRowInfo();
    if( infos.count() >= 5)
        insertDlg->initInfo(infos);

    connect(insertDlg, SIGNAL(accepted(QList<QVariant>)),
            ui->tableView, SLOT(insert(QList<QVariant>)) );
    ui->tabWidget->addTab(insertDlg,"FEM");
    */

    /*
    //set path of ui
    QUiLoader uiLoader;
    QString uiFileName = ":/UI/test.ui";
    QFile uiFile(uiFileName);
    uiFile.open(QIODevice::ReadOnly);
    // setWorkingDirectory: if uiFile depended on other resources,
    // setWorkingDirectory needs to be set here
    //const QDir &workdir(uifileWorkPath);
    //uiLoader.setWorkingDirectory(workdir);
    //load ui
    QWidget* getWidget = uiLoader.load(&uiFile,this);
    ui->tabWidget->addTab(getWidget,"FEM");
    //ui->meshView_verticalLayout->addWidget(getWidget);
    */

    theTabManager = new TabManager(ui->tableView, this);
    theTabManager->init(ui->tabWidget);
    //connect(ui->tableView, SIGNAL(cellClicked(const QModelIndex &)), theTabManager, SLOT(onTableViewClicked(const QModelIndex &)));
    connect(ui->tableView->m_sqlModel, SIGNAL(dataChanged(const QModelIndex&,const QModelIndex&)), theTabManager, SLOT(onTableViewUpdated(const QModelIndex&,const QModelIndex&)));
    connect(ui->gwtEdit, SIGNAL(editingFinished()), theTabManager, SLOT(onFEMTabEdited()));
    ui->materialLayout->setSizeConstraint(QLayout::SetMaximumSize);
    connect(this, SIGNAL(runBtnClicked(QWebEngineView*)), theTabManager, SLOT(onRunBtnClicked(QWebEngineView*)));

    // init GWT
    ui->gwtEdit->setText(QString::number(theTabManager->getGWTFromConfig()));




    //--------------------------------------------------------------------------------//
    // Init Mesher and Mesh View
    //--------------------------------------------------------------------------------//

    mesher = new Mesher();
    mesher->mesh2DColumn();

    // add QQuickwidget for displaying mesh
    meshView = new QQuickView();
    meshView->rootContext()->setContextProperty("designTableModel", ui->tableView);
    meshView->rootContext()->setContextProperty("soilModel", ui->tableView->m_sqlModel);

    elementModel = new ElementModel;
    //std::sort(mesher->elements.begin(),mesher->elements.end(),
    //          [](const Quad &a, const Quad &b) { return  a.tag() > b.tag(); });
    elementModel->clear();
    elementModel->setWidth(mesher->eSizeH());
    for(std::vector<int>::size_type n = mesher->elements.size(); n > 0; n--)
    {
        int tag = mesher->elements[n-1]->tag();
        int i = mesher->elements[n-1]->i();
        int j = mesher->elements[n-1]->j();
        int k = mesher->elements[n-1]->k();
        int l = mesher->elements[n-1]->l();
        double t = mesher->elements[n-1]->thickness();
        QString color = QString::fromStdString(mesher->elements[n-1]->color());
        elementModel->addElement("quad",tag,i,j,k,l,t,color);
    }
    elementModel->refresh();

    meshView->rootContext()->setContextProperty("elements", elementModel);
    meshView->rootContext()->setContextProperty("GWT", ui->gwtEdit->text().toDouble());
    meshView->rootContext()->setContextProperty("totalHeight", mesher->totalHeight());

    QWidget *meshContainer = QWidget::createWindowContainer(meshView, this);
    meshContainer->setMinimumSize(meshViewWidth,layerTableHeight);
    meshContainer->setMaximumSize(meshViewWidth,layerTableHeight);
    meshContainer->setFocusPolicy(Qt::TabFocus);
    meshView->setSource(QUrl(QStringLiteral("qrc:/resources/ui/MeshView.qml")));
    ui->meshView_verticalLayout->addWidget(meshContainer);

    connect(ui->meshBtn, SIGNAL(clicked()), this, SLOT(on_meshBtn_clicked(bool)) );
    ui->meshBtn->setVisible(true);
    ui->groupBox_Mesh->setVisible(false);


    // opensees


    openseesProcess = new QProcess(this);
    //connect(openseesProcess, SIGNAL(readyReadStandardOutput()),this,SLOT(onOpenSeesFinished()));
    connect(openseesProcess, SIGNAL(readyReadStandardError()),this,SLOT(onOpenSeesFinished()));



}

MainWindow::~MainWindow()
{
    delete ui;
}

ElementModel* MainWindow::getElementModel() const
{
    return elementModel;
}

void MainWindow::onTotalLayerEdited()
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



void MainWindow::on_meshBtn_clicked(bool checked)
{

    Q_UNUSED(checked);
    if (ui->groupBox_Mesh->isVisible())
    {
        ui->groupBox_Mesh->setVisible(false);
        int w = layerViewWidth + ui->groupBox_SoilLayersTable->size().width();
        int h = ui->groupBox_Graphic->size().height() ;
        this->resize(w+80,h+20);


    }else{
        ui->groupBox_Mesh->setVisible(true);
        //ui->groupBox_Graphic->setVisible(false);
        //int w =  meshViewWidth + ui->groupBox_SoilLayersTable->size().width();
        int w = layerViewWidth + meshViewWidth + ui->groupBox_SoilLayersTable->size().width();
        int h = ui->groupBox_Graphic->size().height() ;
        this->resize(w+80,h+20);
    }

}

void MainWindow::updateCtrl()
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

}

void MainWindow::insertWithDialog()
{

}



void MainWindow::insert()// insert from dialog
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

void MainWindow::insertAbove()
{
    QList<QVariant> emptyList;
    ui->tableView->insertAbove(emptyList);
    updateCtrl();
}

void MainWindow::insertBelow()
{
    QList<QVariant> emptyList;
    ui->tableView->insertBelow(emptyList);
    updateCtrl();
}

void MainWindow::remove()
{
    ui->tableView->remove();
    updateCtrl();
    int totalPage = ui->tableView->totalPage();

    int count = ui->gotoPageBox->count();
    if( count > totalPage)
    {
        ui->gotoPageBox->removeItem(count-1);
    }
}

void MainWindow::gotoPage(int index)
{
    index++;
    ui->tableView->gotoPage(index);
    updateCtrl();
}

void MainWindow::nextPage()
{
    ui->tableView->nextPage();
    updateCtrl();
}

void MainWindow::prevPage()
{
    ui->tableView->previousPage();
    updateCtrl();
}

void MainWindow::on_delRowBtn_clicked()
{
    emit ui->tableView->removeAct();
}

void MainWindow::on_addRowBtn_clicked()
{
    emit ui->tableView->insertBelowAct();
}

/**
 * when user input a new height value
 * @brief MainWindow::totalHeightChanged
 * @return
 */
void MainWindow::totalHeightChanged()
{
    if(abs(ui->totalHeight->text().toDouble() - ui->tableView->totalHeight())>1e-5)
    {
        ui->tableView->setTotalHeight(ui->totalHeight->text().toDouble());
        ui->tableView->divideByLayers(ui->totalHeight->text().toDouble(), ui->tableView->totalSize());
        //emit gwtChanged(QString::number(ui->tableView->getGWT()));
        qDebug()<<"height changed. => " << ui->totalHeight->text();
    }

}

void MainWindow::on_thickness_edited()
{

    ui->totalHeight->setText(QString::number(ui->tableView->totalHeight()));

    // move GWT to new position in UI
    double originalGWT = ui->tableView->getGWT();
    ui->gwtEdit->textChanged(QString::number(0));
    ui->gwtEdit->textChanged(QString::number(originalGWT));

}

void MainWindow::on_rowRemoved(int row)
{
    Q_UNUSED(row);
    // move GWT to new position
    double originalGWT = ui->tableView->getGWT();
    ui->gwtEdit->textChanged(QString::number(0));
    ui->gwtEdit->textChanged(QString::number(originalGWT));
}



void MainWindow::on_gwtEdit_textChanged(const QString &newGWT)
{
    ui->tableView->setGWT(newGWT.toDouble());
}



void MainWindow::on_reBtn_clicked()
{




    BonzaTableModel* tableModel = ui->tableView->m_sqlModel;

    QWidget* FEMtab = ui->tabWidget->widget(0);

    double GWT = ui->tableView->getGWT();

    json root = {
        {"name","Configureation of Site Response Analysis of A Demo Site"},
        {"author","SimCenter Site Response Tool"}
    };
    QList<QString> listFEMtab = {"eSizeH", "eSizeV", "RockVs", "RockDen", "DashpotCoeff", "VisC", "GMPath"  };
    for (int i = 0; i < listFEMtab.size(); ++i) {
        QString edtName = listFEMtab[i] ;
        FEMtab->findChild<QLineEdit*>(edtName)->text();
    }
    json basicSettings;
    basicSettings["eSizeH"] = FEMtab->findChild<QLineEdit*>("eSizeH")->text().toDouble();
    basicSettings["eSizeV"] = FEMtab->findChild<QLineEdit*>("eSizeV")->text().toDouble();
    basicSettings["rockVs"] = FEMtab->findChild<QLineEdit*>("RockVs")->text().toDouble();
    basicSettings["rockDen"] = FEMtab->findChild<QLineEdit*>("RockDen")->text().toDouble();
    basicSettings["dashpotCoeff"] = FEMtab->findChild<QLineEdit*>("DashpotCoeff")->text().toDouble();
    basicSettings["dampingCoeff"] = FEMtab->findChild<QLineEdit*>("VisC")->text().toDouble();
    basicSettings["groundMotion"] = FEMtab->findChild<QLineEdit*>("GMPath")->text().toStdString();
    basicSettings["groundWaterTable"] = GWT;
    root["basicSettings"] = basicSettings;

    json soilProfile = {

    };

    json layer, soilLayers, material, materials;

    //QString FEMString = tableModel->record(index.row()).value("FEM").toString();
    int numLayers = tableModel->rowCount();
    for (int i=0; i<numLayers; i++)
    {
        QList<QVariant> list = tableModel->getRowInfo(i);
        std::istringstream iss(list.at(FEM-2).toString().toStdString());
        std::vector<std::string> pars((std::istream_iterator<std::string>(iss)),
                                         std::istream_iterator<std::string>());
        double eSize = atof(pars[0].c_str());
        int id = i;
        QStringList thisFEMList = list.at(FEM-2).toString().split(" ");
        int DrInd=0, hPermInd=0, vPermInd=0, uBulkInd=0;
        if(list.at(MATERIAL-2).toString()=="Elastic")
        {
            DrInd = 4; hPermInd = 6; vPermInd = 7; uBulkInd = 10;
        }else if(list.at(MATERIAL-2).toString()=="PM4Sand")
        {
            DrInd = 1; hPermInd = 25; vPermInd = 26; uBulkInd = 27;
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
        };
        material =  createMaterial(i+1, list.at(MATERIAL-2).toString().toStdString(),list.at(FEM-2).toString().toStdString());
        materials.push_back(material);
        soilLayers.push_back(layer);

    }






    soilProfile["soilLayers"] = soilLayers;

    root["soilProfile"]=soilProfile;
    root["materials"]=materials;

    // write prettified JSON to another file
    QString file_name = "SRT.json";
    //QString file_name = QFileDialog::getSaveFileName(this, tr("Choose Path for saving the analysis"), "", tr("Config Files (*.json)"));

    if (!file_name.isNull())
    {
        std::ofstream o(file_name.toStdString());
        o << std::setw(4) << root << std::endl;
    } else {
        QMessageBox::information(this, "error", "Failed to get file name.");
    }



    mesher->mesh2DColumn();
    elementModel->clear();

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
        elementModel->addElement("quad",tag,i,j,k,l,t,color);
    }
    elementModel->refresh();



}

void MainWindow::on_runBtn_clicked()
{
    SiteResponse srt;
    //openseesProcess->start("ls");
    openseesProcess->start("/Users/simcenter/Codes/OpenSees/bin/opensees",QStringList()<<"/Users/simcenter/Codes/SimCenter/SiteResponseTool/bin/model.tcl");
    openseesErrCount = 1;
    //openseesProcess->write ("exit\n\r");

    emit runBtnClicked(dinoView);

    /*
    ui->tabWidget->addTab(dinoView, "Run");
    ui->tabWidget->setCurrentIndex(1);
    */
}

void MainWindow::onOpenSeesFinished()
{

    QString str_err = openseesProcess->readAllStandardError();

    if(openseesErrCount==1)
    {
        if(str_err.contains("Site response analysis is finished."))
        {
            QMessageBox::information(this,tr("OpenSees Information"), "Analysis is done.", tr("OK."));
            qDebug() << "opensees says:" << str_err;
            openseesErrCount = 2;
            theTabManager->getTab()->setCurrentIndex(2);
        }
    }

}

json MainWindow::createMaterial(int tag, std::string matType, std::string parameters)
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
    }
    return mat;
}

void MainWindow::refresh()
{
    //ui->tableView->move(0,50);
    QTime dieTime= QTime::currentTime().addMSecs(500);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    resize(ui->centralWidget->width(),ui->centralWidget->height()+1);



/*
    QPropertyAnimation *animation1= new QPropertyAnimation(ui->tableView,"geometry");
    animation1->setDuration(500);
    animation1->setStartValue(ui->tableView->geometry());
    animation1->setEndValue(QRect(0, 0, ui->tableView->width(), ui->tableView->height()));
    animation1->start();
    */


    //ui->tableView->setVisible(true);


}




