#include "PostProcessor.h"

PostProcessor::PostProcessor(QWidget *parent) : QDialog(parent)
{

}

PostProcessor::PostProcessor(QTabWidget *tab,QWidget *parent) : QDialog(parent)
{

}

int PostProcessor::getEleCount()
{
    QFile file("out_tcl/elementInfo.dat");
    eleCount = 0;
    file.open(QIODevice::ReadOnly); //| QIODevice::Text)
    QTextStream in(&file);
    while( !in.atEnd())
    {
        in.readLine();
        eleCount++;
    }
    return eleCount;
}

void PostProcessor::update()
{
    calcDepths();
    calcRuDepths();
    calcPGA();
    calcGamma();
    calcDisp();
    calcRu();
    emit updateFinished();
}


void PostProcessor::calcDepths()
{


    QString nodesFileName = "out_tcl/nodesInfo.dat";
    QFile nodesFile(nodesFileName);
    QVector<double> depths;
    if(nodesFile.open(QIODevice::ReadOnly)) {
        QTextStream in(&nodesFile);
        while(!in.atEnd()) {
            QString line = in.readLine();
            QStringList thisLine = line.split(" ");
            if (thisLine.size()<3)
                break;
            else
            {
                depths.append(thisLine[2].toDouble());
            }
        }
        nodesFile.close();
    }



    std::sort( depths.begin(), depths.end() );
    depths.erase( std::unique(depths.begin(), depths.end() ), depths.end() );//remove duplicates
    double maxDepth = depths.last();
    for(int i=0;i<depths.size();i++)
        depths[i] = maxDepth - depths[i];
    std::sort( depths.begin(), depths.end() );

    if (!m_depths.isEmpty())
        m_depths.clear();
    for(int i=0;i<depths.size();i++)
        m_depths.append( depths[i] );


    QFile saveFile(QStringLiteral("out_tcl/depths.dat"));
    if (!saveFile.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        qWarning("Couldn't open save file.");
    }
    QTextStream out(&saveFile);


    for (int i=0;i<m_depths.size();i++)
        out << QString::number(m_depths[i]) << "\n";
    saveFile.close();

}

void PostProcessor::calcRuDepths()
{
    m_ruDepths.clear();
    m_ruDepths << m_depths[0];
    for (int i=0; i<(m_depths.size()-1); i++)
        m_ruDepths << (m_depths[i+1]+m_depths[i]) * 0.5;
    m_ruDepths << m_depths.last();
}

void PostProcessor::calcPGA()
{
    QString accFileName = "out_tcl/acceleration.out";
    QFile accFile(accFileName);
    QVector<double> pga;
    if(accFile.open(QIODevice::ReadOnly)) {
        QTextStream in(&accFile);
        while(!in.atEnd()) {
            QString line = in.readLine();
            QStringList thisLine = line.split(" ");
            if (thisLine.size()<2)
                break;
            else
            {
                thisLine.removeAll("");
                QVector<double> thispga;
                for (int i=1; i<thisLine.size();i+=4)// TODO: 3D?
                {
                    double tmp = fabs(thisLine[i].trimmed().toDouble());
                    thispga << tmp;
                }
                if(pga.size()!=thispga.size() && pga.size()<1)
                {
                    for (int j=0;j<thispga.size();j++)
                        pga << thispga[j];
                }
                if (pga.size()==thispga.size())
                {
                    for (int j=0;j<thispga.size();j++)
                    {
                        if (thispga[j]>pga[j])
                            pga[j] = thispga[j];
                    }
                }
            }
        }
        accFile.close();
    }

    if (m_pga.size()>0)
        m_pga.clear();
    for(int i=0;i<pga.size();i++)
        m_pga.append( pga[i] / g );



    QFile saveFile(QStringLiteral("out_tcl/pga.dat"));
    if (!saveFile.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        qWarning("Couldn't open save file.");
    }
    QTextStream out(&saveFile);


    for (int i=0;i<m_pga.size();i++)
        out << QString::number(m_pga[i]) << "\n";
    saveFile.close();





}


void PostProcessor::calcGamma()
{
    QString FileName = "out_tcl/strain.out";
    QFile File(FileName);
    QVector<double> v;
    if(File.open(QIODevice::ReadOnly)) {
        QTextStream in(&File);
        while(!in.atEnd()) {
            QString line = in.readLine();
            QStringList thisLine = line.split(" ");
            if (thisLine.size()<2)
                break;
            else
            {
                thisLine.removeAll("");
                QVector<double> thisv;
                for (int i=3; i<thisLine.size();i+=3)// TODO: 3D?
                {
                    double tmp = fabs(thisLine[i].trimmed().toDouble());
                    thisv << tmp;
                }
                if(v.size()!=thisv.size() && v.size()<1)
                {
                    for (int j=0;j<thisv.size();j++)
                        v << thisv[j];
                }
                if (v.size()==thisv.size())
                {
                    for (int j=0;j<thisv.size();j++)
                    {
                        if (thisv[j]>v[j])
                            v[j] = thisv[j];
                    }
                }
            }
        }
        File.close();
    }

    if (m_gamma.size()>0)
        m_gamma.clear();
    for(int i=0;i<v.size();i++)
        m_gamma.append( v[i] * 100 );



    QFile saveFile(QStringLiteral("out_tcl/gammaMax.dat"));
    if (!saveFile.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        qWarning("Couldn't open save file.");
    }
    QTextStream out(&saveFile);


    for (int i=0;i<m_gamma.size();i++)
        out << QString::number(m_gamma[i]) << "\n";
    saveFile.close();

}


void PostProcessor::calcDisp()
{
    QString FileName = "out_tcl/displacement.out";
    QFile File(FileName);
    QVector<double> v;
    QVector<double> v1;
    double thisDisp;
    if(File.open(QIODevice::ReadOnly)) {
        QTextStream in(&File);
        while(!in.atEnd()) {
            QString line = in.readLine();
            QStringList thisLine = line.split(" ");
            if (thisLine.size()<2)
                break;
            else
            {
                thisLine.removeAll("");
                QVector<double> thisv;
                for (int i=1; i<thisLine.size();i+=4)// TODO: 3D?
                {
                    double tmp = fabs(thisLine[i].trimmed().toDouble());
                    thisv << tmp;
                }
                if(v.size()!=thisv.size() && v.size()<1)
                {
                    for (int j=0;j<thisv.size();j++)
                    {
                        v1 << thisv[j];
                        v << 0.0;
                    }
                }
                if (v.size()==thisv.size())
                {
                    for (int j=0;j<thisv.size();j++)
                    {
                        thisDisp = thisv[j]-v1[j];
                        if (thisDisp>v[j])
                            v[j] = thisDisp;
                    }
                }
            }
        }
        File.close();
    }

    if (m_disp.size()>0)
        m_disp.clear();
    for(int i=0;i<v.size();i++)
        m_disp.append( v[i] * 100 );



    QFile saveFile(QStringLiteral("out_tcl/dispMax.dat"));
    if (!saveFile.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        qWarning("Couldn't open save file.");
    }
    QTextStream out(&saveFile);


    for (int i=0;i<m_disp.size();i++)
        out << QString::number(m_disp[i]) << "\n";
    saveFile.close();

}


void PostProcessor::calcRu()
{
    QString FileName = "out_tcl/stress.out";
    QFile File(FileName);
    QVector<double> v;
    QVector<double> v1;
    double thisValue;

    eleCount = getEleCount();

    if(File.open(QIODevice::ReadOnly)) {
        QTextStream in(&File);
        while(!in.atEnd()) {
            QString line = in.readLine();
            QStringList thisLine = line.split(" ");
            if (thisLine.size()<2)
                break;
            else
            {
                thisLine.removeAll("");
                QVector<double> thisv;
                for (int i=2; i<thisLine.size();i+=3)// TODO: 3D?
                {
                    double tmp = (thisLine[i].trimmed().toDouble());
                    thisv << tmp;
                }
                if(v.size()!=thisv.size() && v.size()<1)// first time
                {
                    for (int j=0;j<thisv.size();j++)
                    {
                        v1 << thisv[j];
                        v << 0.0;
                    }
                }
                if (v.size()==thisv.size())
                {
                    for (int j=0;j<thisv.size();j++)
                    {
                        thisValue = -(thisv[j]-v1[j]) / v1[j];
                        if (thisValue>v[j])
                            v[j] = thisValue;
                    }
                }
            }
        }
        File.close();
    }

    if (m_ru.size()>0)
        m_ru.clear();
    for(int i=0;i<v.size();i++)
        m_ru.append( v[i] * 100 );



    QFile saveFile(QStringLiteral("out_tcl/ru.dat"));
    if (!saveFile.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        qWarning("Couldn't open save file.");
    }
    QTextStream out(&saveFile);


    for (int i=0;i<m_ru.size();i++)
        out << QString::number(m_ru[i]) << "\n";
    saveFile.close();

}
