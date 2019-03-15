#include "SSSharkThread.h"

SSSharkThread::SSSharkThread(QString srtFileNametmp, QString analysisDirtmp,QString outputDirtmp,QObject *parent) :
    analysisDir(analysisDirtmp),
    outputDir(outputDirtmp),
    srtFileName(srtFileNametmp),
    QThread(parent)
{

}

