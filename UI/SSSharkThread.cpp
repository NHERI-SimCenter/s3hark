#include "SSSharkThread.h"

SSSharkThread::SSSharkThread(QString srtFileNametmp, QString analysisDirtmp,QString outputDirtmp,QString femLogtmp, QObject *parent) :
    analysisDir(analysisDirtmp),
    outputDir(outputDirtmp),
    srtFileName(srtFileNametmp),
    femLog(femLogtmp),
    QThread(parent)
{

}

