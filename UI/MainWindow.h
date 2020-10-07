#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
class RockOutcrop;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void newFile();
    void open();
    void openFile(QString filename);
    bool save();
    bool saveAs();

    void about();
    void manual();
    void submitFeedback();
    void submitFeatureRequest();
    void version();
    void copyright();
    void cite();

signals:

private:

private:
    void createActions();
    void setCurrentFile(const QString &fileName);
    bool saveFile(const QString &fileName);
    void loadFile(const QString &fileName);

    RockOutcrop *theRockOutcropWidget;

    QMenu *fileMenu;
    QMenu *editMenu;

    QString currentFile;

    /*
    QMenu *selectSubMenu;
    QMenu *toolsMenu;
    QMenu *optionsMenu;
    QMenu *helpMenu;
    QToolBar *fileToolBar;
    QToolBar *editToolBar;
*/
};



#endif // MAINWINDOW_H
