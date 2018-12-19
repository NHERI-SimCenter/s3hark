#include "InsertWindow.h"
#include "ui_InsertWindow.h"

#include <QQuickView>

InsertWindow::InsertWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InsertWindow)
{
    ui->setupUi(this);

    /*
    //ui->label_4->hide();
    QQuickView *FEMView = new QQuickView();
    //FEMView->rootContext()->setContextProperty("designTableModel", ui->tableView);
    QWidget *FEMContainer = QWidget::createWindowContainer(FEMView, this);
    FEMView->setSource(QUrl(QStringLiteral("qrc:/resources/ui/FEMView.qml")));

    ui->horizontalLayout->addWidget(FEMContainer);
    */

    connect(this, SIGNAL(accepted()),
            this, SLOT(submitx()) );
}

InsertWindow::~InsertWindow()
{
    delete ui;
}

void InsertWindow::initInfo(const QList<QVariant> &list)
{
    ui->layerNameEdt->setText(list.at(0).toString());
    ui->thicknessEdit->setText(list.at(1).toString());
    ui->botomposEdt->setText(list.at(2).toString());
    ui->VsEdt->setText(list.at(3).toString());
    ui->DrEdt->setPlainText(list.at(4).toString());

}

void InsertWindow::submitx()
{
    QList<QVariant> list;
    QString layerName = ui->layerNameEdt->text();
    QString thickness = ui->thicknessEdit->text();
    QString botompos = ui->botomposEdt->text();
    QString Vs = ui->VsEdt->text();
    QString Dr = ui->DrEdt->toPlainText();
    list << layerName << thickness << botompos << Vs << Dr;

    emit accepted(list);
}
