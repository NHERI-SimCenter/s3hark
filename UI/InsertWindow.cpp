#include "InsertWindow.h"
#include "ui_InsertWindow.h"

InsertWindow::InsertWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InsertWindow)
{
    ui->setupUi(this);

    connect(this, SIGNAL(accepted()),
            this, SLOT(submite()) );
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

void InsertWindow::submite()
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
