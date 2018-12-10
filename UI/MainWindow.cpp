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

#include "TabManager.h"

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

    // add QQuickwidget for displaying mesh
    QQuickView *meshView = new QQuickView();
    meshView->rootContext()->setContextProperty("designTableModel", ui->tableView);
    meshView->rootContext()->setContextProperty("soilModel", ui->tableView->m_sqlModel);
    QWidget *meshContainer = QWidget::createWindowContainer(meshView, this);

    meshContainer->setMinimumSize(meshViewWidth,layerTableHeight);
    meshContainer->setMaximumSize(meshViewWidth,layerTableHeight);
    meshContainer->setFocusPolicy(Qt::TabFocus);
    meshView->setSource(QUrl(QStringLiteral("qrc:/resources/ui/MeshView.qml")));
    ui->meshView_verticalLayout->addWidget(meshContainer);

    connect(ui->meshBtn, SIGNAL(clicked()), this, SLOT(on_meshBtn_clicked(bool)) );

    ui->meshBtn->setVisible(true);

    ui->groupBox_Mesh->setVisible(false);

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
    dinoView->load(QUrl("file:////Users/simcenter/Codes/SimCenter/SiteResponseTool/resources/ui/DinoRun/index.html"));
    dinoView->setVisible(false);

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

    TabManager* theTabManager = new TabManager(ui->tableView, this);
    theTabManager->init(ui->tabWidget);
    //connect(ui->tableView, SIGNAL(cellClicked(const QModelIndex &)), theTabManager, SLOT(onTableViewClicked(const QModelIndex &)));
    connect(ui->tableView->m_sqlModel, SIGNAL(dataChanged(const QModelIndex&,const QModelIndex&)), theTabManager, SLOT(onTableViewUpdated(const QModelIndex&,const QModelIndex&)));

    ui->materialLayout->setSizeConstraint(QLayout::SetMaximumSize);



    //SiteResponse srt ;



}

MainWindow::~MainWindow()
{
    delete ui;
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
    ui->tableView->setTotalHeight(ui->totalHeight->text().toDouble());
    ui->tableView->divideByLayers();
    //emit gwtChanged(QString::number(ui->tableView->getGWT()));



    qDebug()<<"height changed. => " << ui->totalHeight->text();
}

void MainWindow::on_thickness_edited()
{

    ui->totalHeight->setText(QString::number(ui->tableView->totalHeight()));

    // move GWT to new position
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
    qDebug()<<"gwt changed." << newGWT;
}



void MainWindow::on_reBtn_clicked()
{
    //ui->tableView->hide();


    //dinoView = new QWebEngineView(this);
    //dinoView->load(QUrl("file:////Users/simcenter/Codes/Sandbox/SRT/SiteResponseTool/resources/ui/DinoRun/index.html"));
    ui->tabWidget->addTab(dinoView, "Run");

    ui->tabWidget->setCurrentIndex(1);

    ui->tabWidget->setMovable(true);
    //dinoView->setVisible(true);

    //view->show();
    //dinoView->setMinimumHeight(400);
    //dinoView->setMaximumHeight(400);
    //ui->materialLayout->addWidget(dinoView);


    /*
    QWidget *w1 = this->ui->tableView;
    QLabel *label = new QLabel(w1);
    label->resize(w1->size());
    label->setPixmap(w1->grab());
    label->show();

    QPropertyAnimation *animation= new QPropertyAnimation(w1,"geometry");
    animation->setDuration(500);
    animation->setStartValue(w1->geometry());
    animation->setEndValue(QRect(0, 50, w1->width(), w1->height()));
    animation->start();


    emit tableMoved();

    plotContainer->setWindowOpacity(0.1);
    qDebug() << this->ui->tableView->geometry();
    qDebug() << QRect(this->ui->tableView->width(), 0, this->ui->tableView->width(), this->ui->tableView->height());
    */

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




