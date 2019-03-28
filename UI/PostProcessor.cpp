#ifdef _MSC_VER
    #define _USE_MATH_DEFINES
    #include <cmath>
#endif

#include "PostProcessor.h"

PostProcessor::PostProcessor(QWidget *parent) : QDialog(parent)
{

}

PostProcessor::PostProcessor(QTabWidget *tab,QWidget *parent) : QDialog(parent)
{

}

int PostProcessor::getEleCount()
{
//    /QFile file("out_tcl/elementInfo.dat");
    QFile file(elementFileName);
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
    calcSigma();
    calcDisp();
    calcRu();
    calcRupwp();

    loadMotions();

    emit updateFinished();
}

void PostProcessor::loadMotions()
{
    calcMotion("base", "vel");
    calcMotion("base", "disp");
    calcMotion("base", "acc");
    calcMotion("surface", "vel");
    calcMotion("surface", "disp");
    calcMotion("surface", "acc");

    calcAllMotion("vel");
    calcAllMotion("disp");
    calcAllMotion("acc");

    calcSa();
}

void PostProcessor::calcSa()
{
    QVector<QVector<double>> v = *accAll;
    if(v.size()>0)
    {
        saVec->clear();
        Periods->clear();

        double disp_init = 0.0;
        double vel_init = 0.0;
        double dt = v[0][1] - v[0][0];
        double mass = 1.0;
        double dampingRatio = 0.05;

        int numT = 100;
        QVector<QVector<double>> saVectmp((v.size()-1)/4,QVector<double>(numT));
        *saVec = saVectmp;
        //QVector<double> Periods;

        double pi = M_PI;
        int ind = -1;
        for(int i=1;i<v.size();i+=4)// for each node
        {
            ind += 1;
            QVector<double> acc = v[i];
            QVector<double> f = acc; // because mass=1.

            QVector<double> thisSa(numT);
            for(int n=0;n<numT;n++)
            {
                double T = 0.04+n*0.02;
                if(i==1)
                    Periods->append(T);
                double k = (2.*pi/T)*(2.*pi/T)*mass;
                double natural_freq = 2*pi/T;
                double damping = 2.*mass*natural_freq*dampingRatio;

                double gamma = 0.5;
                double beta = 0.25;

                double maxDisp = newmark(mass,damping, k, disp_init, vel_init, gamma, beta, dt, f);
                thisSa[n]=(maxDisp*natural_freq*natural_freq/9.81);
            }
            (*saVec)[ind]=(thisSa);
        }
    }
}


double PostProcessor::newmark(double mass,double damping, double stiffness, double disp_init, double vel_init, double gamma, double beta, double time_step, QVector<double> force_hist)
{
    QVector<double> disps(force_hist.size()+1);
    QVector<double> vels(force_hist.size()+1);
    QVector<double> accels(force_hist.size()+1);

    // set initial values
    disps[0]=(disp_init) ;
    vels[0]=(vel_init);
    accels[0]=((force_hist[0] - damping * vels[0] - stiffness * disps[0]) / mass);

    // Constants
    double k_hat = stiffness + gamma * damping / (beta * time_step) + mass / (beta * time_step*time_step);

    for (int i=0;i<force_hist.size();i++)//index, force in enumerate(force_hist[1:], 1):
    {
        int index = i+1;
        double force = force_hist[i];
        // Prediction step
        disps[index]=(disps[index - 1]);
        vels[index]=((1.0 - gamma / beta) * vels[index - 1] + \
                time_step * (1.0 - gamma / (2.0 * beta)) * accels[index - 1]);
        accels[index]=((-1.0 / (beta * time_step)) * vels[index - 1] + \
                (1.0 - 1.0/ (2.0 * beta)) * accels[index - 1]);

        // Correction step
        double d_disp = (force - mass * accels[index] - damping * vels[index] - stiffness * disps[index]) / k_hat;
        disps[index]=(disps[index] + d_disp);
        vels[index]=(vels[index] + gamma * d_disp / (beta * time_step));
        accels[index]=(accels[index] + d_disp / (beta * time_step*time_step));
    }
    double maxDisp=0;
    for(int i=0;i<disps.size();i++)
    {
        double thisDisp = fabs(disps[i]);
        if (thisDisp>maxDisp)
            maxDisp = thisDisp;
    }
    return maxDisp;

    //return disps, vels, accels
}

void PostProcessor::calcMotion(QString pos, QString motion)
{
    QString vaseVelFileName = analysisDir+"/out_tcl/"+pos+"."+motion;
    QFile baseVelFile(vaseVelFileName);
    QStringList *xv;
    QStringList *yv;
    if(pos=="base")
    {
        if (motion == "vel")
        {xv = xdBaseVel; yv = ydBaseVel;}
        else if (motion == "acc")
        {xv = xdBaseAcc; yv = ydBaseAcc;}
        else // (motion == "disp")
        {xv = xdBaseDisp; yv = ydBaseDisp;}
    }else{
        if (motion == "vel")
        {xv = xdSurfaceVel; yv = ydSurfaceVel;}
        else if (motion == "acc")
        {xv = xdSurfaceAcc; yv = ydSurfaceAcc;}
        else // (motion == "disp")
        {xv = xdSurfaceDisp; yv = ydSurfaceDisp;}
    }

    if(xv && yv)
    {xv->clear(); yv->clear();}

    if(baseVelFile.open(QIODevice::ReadOnly)) {
        QTextStream in(&baseVelFile);
        while(!in.atEnd()) {
            QString line = in.readLine();
            QStringList thisLine = line.split(" ");
            if (thisLine.size()<2)
                break;
            else
            {
                thisLine.removeAll("");
                if(thisLine.size()>1)
                {
                   xv->append(thisLine[0].trimmed());
                   yv->append(thisLine[1].trimmed());
                }else
                    break;

            }

        }
        baseVelFile.close();
    }
}

void PostProcessor::calcAllMotion(QString motion)
{
    QString motionFileName;
    QVector<QVector<double>> *v;
    if(motion=="acc")
    {
        motionFileName = getAccFileName();
        v = accAll;
    }
    else if (motion=="vel")
    {
        motionFileName = getVelFileName();
        v = velAll;
    }
    else if (motion=="disp")
    {
        motionFileName = getDispFileName();
        v = dispAll;
    }
    else
        qWarning("motion must be acc, vel or disp!");

    v->clear();

    QFile File(motionFileName);


    if(File.open(QIODevice::ReadOnly)) {
        QTextStream in(&File);
        int lineCount = 0;
        int numCols = 0;
        while(!in.atEnd()) {
            QString line = in.readLine();
            QStringList thisLine = line.split(" ");
            thisLine.removeAll("");
            int sizeThisLine = thisLine.size();
            lineCount += 1;
            if (lineCount==1)
                numCols = sizeThisLine;
            if (sizeThisLine != numCols && lineCount>1)
            {
                lineCount -= 1;
                break;
            }
            else
            {
                //thisLine.removeAll("");
                for (int i=0; i<thisLine.size();i++)// TODO: 3D?
                {
                    if (lineCount==1)
                    {
                        QVector<double> tmpV;
                        v->append(tmpV);
                    }
                    (*v)[i].append((thisLine[i].trimmed().toDouble()));
                }
            }
        }
        File.close();
    }
}


void PostProcessor::calcDepths()
{


    //QString nodesFileName = "out_tcl/nodesInfo.dat";
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
    //std::sort( depths.begin(), depths.end() );

    if (!m_depths.isEmpty())
        m_depths.clear();
    for(int i=0;i<depths.size();i++)
        m_depths.append( depths[i] );


    QFile saveFile(dephsFileName);
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
    //QString accFileName = accFileName;
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



    QFile saveFile(pgaFileName);
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
    QString FileName = strainFileName;
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



    QFile saveFile(gammaMaxFileName);
    if (!saveFile.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        qWarning("Couldn't open save file.");
    }
    QTextStream out(&saveFile);


    for (int i=0;i<m_gamma.size();i++)
        out << QString::number(m_gamma[i]) << "\n";
    saveFile.close();

}


void PostProcessor::calcSigma()
{
    QString FileName = stressFileName;
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

    if (m_sigma.size()>0)
        m_sigma.clear();
    for(int i=0;i<v.size();i++)
        m_sigma.append( v[i] );



    QFile saveFile(sigmaMaxFileName);
    if (!saveFile.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        qWarning("Couldn't open save file.");
    }
    QTextStream out(&saveFile);


    for (int i=0;i<m_sigma.size();i++)
        out << QString::number(m_sigma[i]) << "\n";
    saveFile.close();

}


void PostProcessor::calcDisp()
{
    QString FileName = dispFileName;
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
                    double tmp = (thisLine[i].trimmed().toDouble());
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
                        thisDisp = fabs(thisv[j]-thisv[0]);
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
        m_disp.append( v[i] );



    QFile saveFile(dispMaxFileName);
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
    QString FileName = stressFileName;
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



    QFile saveFile(ruFileName);
    if (!saveFile.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        qWarning("Couldn't open save file.");
    }
    QTextStream out(&saveFile);


    for (int i=0;i<m_ru.size();i++)
        out << QString::number(m_ru[i]) << "\n";
    saveFile.close();

}



void PostProcessor::calcRupwp()
{
    QVector<QVector<double>> pwp;
    QFile pwpFile(pwpFileName);
    if(pwpFile.open(QIODevice::ReadOnly)) {
        QTextStream in(&pwpFile);
        while(!in.atEnd()) {
            QString line = in.readLine();
            QStringList thisLine = line.split(" ");
            if (thisLine.size()<2)
                break;
            else
            {
                thisLine.removeAll("");
                QVector<double> thisv;
                for (int i=1; i<thisLine.size();i+=2)// TODO: 3D?
                {
                    double tmp = (thisLine[i].trimmed().toDouble());
                    thisv << tmp;
                }
                pwp.append(thisv);
            }
        }
        pwpFile.close();
    }

    QString FileName = stressFileName;
    QFile File(FileName);
    QVector<double> v;
    QVector<double> v1;
    double thisValue;

    eleCount = getEleCount();

    if(File.open(QIODevice::ReadOnly)) {
        QTextStream in(&File);
        int timestep = -1;
        QVector<double> pwp1 = pwp[0];
        while(!in.atEnd()) {
            timestep += 1;
            QVector<double> pwpthis = pwp[timestep];
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
                        //thisValue = -(thisv[j]-v1[j]) / v1[j];
                        int topNode = j+1;
                        int bottomnode = j;
                        thisValue = -0.5*(pwpthis[topNode]-pwp1[topNode]+pwpthis[bottomnode]-pwp1[bottomnode]) / v1[j];

                        //thisValue = -(pwpthis[topNode]-pwp1[topNode]) / v1[j];
                        if (thisValue>v[j])
                            v[j] = thisValue;


                    }
                }
            }
        }
        File.close();
    }

    if (m_rupwp.size()>0)
        m_rupwp.clear();
    for(int i=0;i<v.size();i++)
        m_rupwp.append( v[i] * 100 );



    QFile saveFile(rupwpFileName);
    if (!saveFile.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        qWarning("Couldn't open save file.");
    }
    QTextStream out(&saveFile);


    for (int i=0;i<m_rupwp.size();i++)
        out << QString::number(m_rupwp[i]) << "\n";
    saveFile.close();

}
