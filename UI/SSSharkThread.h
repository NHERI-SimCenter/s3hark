#ifndef SSSHARKTHREAD_H
#define SSSHARKTHREAD_H

#include "SiteResponse.h"
#include <QThread>
#include <QObject>
#include <QtMath>
using namespace std::placeholders;

class SSSharkThread : public QThread
{
    Q_OBJECT
signals:
    void resultReady();
    void updateProgress(double);

public:
    explicit SSSharkThread(QObject *parent = nullptr);
    SSSharkThread(QString srtFileNametmp,QString analysisDirtmp,QString outputDirtmp,QObject *parent = nullptr);

    void updateProgressBar(double step){emit updateProgress(step);}
    std::function<void(double)> m_callbackFunction = std::bind(&SSSharkThread::updateProgressBar,this, _1);

    void run() override {
        SiteResponse *srt = new SiteResponse(srtFileName.toStdString(),
                                             analysisDir.toStdString(),outputDir.toStdString(), m_callbackFunction );
        srt->run();
    }

private:

    QString analysisDir ;
    QString outputDir ;
    QString srtFileName ;


};



#endif // SSSHARKTHREAD_H
