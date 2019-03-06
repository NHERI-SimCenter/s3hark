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

class ProfileManager : public QDialog
{
    Q_OBJECT
public:

    explicit ProfileManager(QWidget *parent = nullptr);
    ProfileManager(QTabWidget *tab,PostProcessor *postProcessort,QWidget *parent = nullptr);
    void updatePGAHtml();
    void updateGammaHtml();
    void updateStressStrainHtml();
    void updateDispHtml();
    void updateRuHtml();
    void updatePostProcessor(PostProcessor *);

public slots:
    void onPostProcessorUpdated();
    void onTabBarClicked(int);
public:
    QString rootDir = qApp->applicationDirPath();
private:
    QTabWidget *m_tab;
    QWebEngineView *pgaHtmlView;
    QWebEngineView *gammaHtmlView;
    QWebEngineView *stressStrainHtmlView;
    QWebEngineView *dispHtmlView;
    QWebEngineView *ruHtmlView;
    PostProcessor *postProcessor;
    QString pgaHtmlName = QDir(rootDir).filePath("resources/ui/Profile/pga.html");
    QString gammaHtmlName = QDir(rootDir).filePath("resources/ui/Profile/gamma.html");
    QString stressStrainHtmlName = QDir(rootDir).filePath("resources/ui/Profile/stressstrain.html");
    QString dispHtmlName = QDir(rootDir).filePath("resources/ui/Profile/disp.html");
    QString ruHtmlName = QDir(rootDir).filePath("resources/ui/Profile/ru.html");
};

#endif // PROFILEMANAGER_H
