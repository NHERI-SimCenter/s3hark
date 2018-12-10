#ifndef INSERTDIALOG_H
#define INSERTDIALOG_H

#include <QDialog>

namespace Ui {
class InsertWindow;
}

class InsertWindow : public QDialog
{
    Q_OBJECT

public:
    explicit InsertWindow(QWidget *parent = nullptr);
    ~InsertWindow();
    void initInfo(const QList<QVariant> &list);

signals:
    void accepted( const QList<QVariant> & );

private slots:
    void submitx();

private:
    Ui::InsertWindow *ui;
};

#endif // INSERTDIALOG_H
