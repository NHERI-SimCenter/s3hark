#ifndef PROFILEMANAGER_H
#define PROFILEMANAGER_H

#include <QObject>
#include <QDialog>
#include <QTabWidget>
#include <QWebEngineView>
#include <QFileInfo>
#include <QDir>
#include <QApplication>
#include "PostProcessor.h"
#include <QStandardPaths>

class ProfileManager : public QDialog
{
    Q_OBJECT
public:

    explicit ProfileManager(QWidget *parent = nullptr);
    ProfileManager(QTabWidget *tab,PostProcessor *postProcessort,QWidget *parent = nullptr);
    void updatePGAHtml(); void updatePGAHtml3D();
    void updateGammaHtml(); void updateGammaHtml3D();
    void updaterupwpHtml();
    void updateDispHtml(); void updateDispHtml3D();
    void updateRuHtml();
    void updatePostProcessor(PostProcessor *);
    int checkDim();

public slots:
    void onPostProcessorUpdated();
    void onTabBarClicked(int);
public:
    QString rootDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);//qApp->applicationDirPath();
private:
    QTabWidget *m_tab;
    QWebEngineView *pgaHtmlView;
    QWebEngineView *gammaHtmlView;
    QWebEngineView *rupwpHtmlView;
    QWebEngineView *dispHtmlView;
    QWebEngineView *ruHtmlView;
    PostProcessor *postProcessor;
    QString pgaHtmlName = QDir(rootDir).filePath("resources/ui/Profile/pga.html");
    QString gammaHtmlName = QDir(rootDir).filePath("resources/ui/Profile/gamma.html");
    QString rupwpHtmlName = QDir(rootDir).filePath("resources/ui/Profile/rupwp.html");
    QString dispHtmlName = QDir(rootDir).filePath("resources/ui/Profile/disp.html");
    QString ruHtmlName = QDir(rootDir).filePath("resources/ui/Profile/ru.html");
};

#endif // PROFILEMANAGER_H
