#ifndef SSSHARKTHREAD_H
#define SSSHARKTHREAD_H

#include "SiteResponse.h"
#include <QThread>
#include <QObject>
#include <QtMath>
using namespace std::placeholders;

//#include <iostream>
//#include <QDateTime>

class SSSharkThread : public QThread
{
    Q_OBJECT
signals:
    void resultReady();
    void updateProgress(double);

public:
    explicit SSSharkThread(QObject *parent = nullptr);
    SSSharkThread(QString srtFileNametmp,QString analysisDirtmp,QString outputDirtmp,QString femLog,QObject *parent = nullptr);

    bool updateProgressBar(double step){emit updateProgress(step);return forward;}
    std::function<bool(double)> m_callbackFunction = std::bind(&SSSharkThread::updateProgressBar,this, _1);
    void setStopSignal(){forward = false;}
    bool getForward(){return forward;}


    void run() override {
        if (isInterruptionRequested())
        {   forward = false;
            srt->kill();
        }
        while (!isInterruptionRequested())
        {

            if (!exist) {
                srt = new SiteResponse(srtFileName.toStdString(),
                                                                 analysisDir.toStdString(),
                                                                 outputDir.toStdString(),
                                                                 femLog.toStdString(),
                                                                 m_callbackFunction );
                exist = true;
            }

            if (!isRunning){
                srt->run();
                isRunning = true;
            } else {
                //QDateTime local(QDateTime::currentDateTime());
                //std::cout << "Local time is:" << local.toString().toStdString() << endln;
            }

        }

    }
    /*
    ~SSSharkThread() Q_DECL_OVERRIDE{
            requestInterruption();
            quit();
            wait();
        }
    */


private:

    QString analysisDir ;
    QString outputDir ;
    QString srtFileName ;
    QString femLog ;
    bool isRunning = false;
    bool exist = false;
    SiteResponse *srt;
    bool forward = true;


};



#endif // SSSHARKTHREAD_H
