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

int PostProcessor::checkDim(){
    QFile file(elementFileName);
    file.open(QIODevice::ReadOnly); //| QIODevice::Text)
    QTextStream in(&file);
    while( !in.atEnd())
    {
        QString line = in.readLine();
        QStringList list1 = line.split(' ');
        if (list1.count()>7)
            dim = 3;
        else
            dim = 2;
        break;
    }
    return dim;
}

void PostProcessor::check3DStress()
{   // if tcl recorder is like this:
    // recorder Element -file out_tcl/stress.out -time -dT $recDT  -eleRange 1 30  stress 6
    // no need to run this function
    // if it is like this:
    // recorder Element -file out_tcl/stress.out -time -dT $recDT  -eleRange 1 30  stress
    // then the dimention of each out put of an element is 7
    // run this function to trim it to be 6

    //stressFileName
    QFile file(esmat3DFileName);
    eleCount = 0;
    file.open(QIODevice::ReadOnly); //| QIODevice::Text)
    QTextStream in(&file);
    QVector<int> ind;
    int count = 1;
    while( !in.atEnd())
    {
        QString line = in.readLine();
        QStringList list1 = line.split(' ');
        if (list1[1]=="PIMY" || list1[1]=="PDMY" || list1[1]=="PDMY02")
        {
            count += 7;
            ind.append(count);
        } else
            count += 6;
    }
    file.close();


    if (ind.size() > 0)
    {
        QFile stressFile(stressFileName);
        eleCount = 0;
        stressFile.open(QIODevice::ReadOnly); //| QIODevice::Text)
        QTextStream instress(&stressFile);
        QString stresstext;
        QTextStream stressstream(&stresstext);
        while( !instress.atEnd())
        {
            QString line = instress.readLine();
            QStringList liststress = line.split(' ');
            for (int i = ind.size()-1; i >= 0; i--) {
                liststress.removeAt(ind[i]);
            }
            stressstream << liststress.join(" ") << "\n";
        }
        stressFile.close();


        QFile::remove(stressFileName);
        // write to index.html
        QFile newfile(stressFileName);
        if(newfile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            newfile.write(stresstext.toUtf8());
            newfile.close();
        }
    }


}


void PostProcessor::update()
{
    checkDim();
    //if (dim==3) check3DStress();

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
    if (dim==3)
    {
        calcMotion3D("base", "vel");
        calcMotion3D("base", "disp");
        calcMotion3D("base", "acc");
        calcMotion3D("surface", "vel");
        calcMotion3D("surface", "disp");
        calcMotion3D("surface", "acc");

        calcAllMotion("vel");
        calcAllMotion("disp");
        calcAllMotion("acc");

        calcSa();
    } else {
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

void PostProcessor::calcMotion3D(QString pos, QString motion)
{
    QString vaseVelFileName = analysisDir+"/out_tcl/"+pos+"."+motion;
    QFile baseVelFile(vaseVelFileName);
    QStringList *xv;
    QStringList *yv;
    QStringList *yvx2;
    if(pos=="base")
    {
        if (motion == "vel")
        {xv = xdBaseVel; yv = ydBaseVel;yvx2 = ydBaseVelx2; }
        else if (motion == "acc")
        {xv = xdBaseAcc; yv = ydBaseAcc;yvx2 = ydBaseAccx2;}
        else // (motion == "disp")
        {xv = xdBaseDisp; yv = ydBaseDisp;yvx2 = ydBaseDispx2;}
    }else{
        if (motion == "vel")
        {xv = xdSurfaceVel; yv = ydSurfaceVel;yvx2 = ydSurfaceVelx2;}
        else if (motion == "acc")
        {xv = xdSurfaceAcc; yv = ydSurfaceAcc;yvx2 = ydSurfaceAccx2;}
        else // (motion == "disp")
        {xv = xdSurfaceDisp; yv = ydSurfaceDisp;yvx2 = ydSurfaceDispx2;}
    }

    if(xv && yv &&yvx2)
    {xv->clear(); yv->clear(); yvx2->clear();}

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
                   yvx2->append(thisLine[3].trimmed());
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
                int thisstep = dim==3 ? 8 : 4;
                int thisind = 0 ;


                // add time row to v
                if (lineCount==1)
                {
                    QVector<double> tmpV;
                    v->append(tmpV);
                }
                (*v)[0].append((thisLine[0].trimmed().toDouble()));
                thisind += 1;


                // add motion rows to v
                for (int i=1; (i+thisstep-1)<thisLine.size();i+=thisstep)// TODO: 3D?
                {
                    for (int j=i; j<i+4; j++)
                    {
                        if (lineCount==1)
                        {
                            QVector<double> tmpV;
                            v->append(tmpV);
                        }
                        (*v)[thisind].append((thisLine[j].trimmed().toDouble()));
                        thisind += 1;
                    }
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
    QVector<double> pgax2;
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
                QVector<double> thispgax2;
                int thisstep = dim==3 ? 8 :4;
                for (int i=1; i<thisLine.size();i+=thisstep)// TODO: 3D?
                {
                    double tmp = fabs(thisLine[i].trimmed().toDouble());
                    thispga << tmp;
                    double tmpx2 = fabs(thisLine[i+1].trimmed().toDouble());
                    thispgax2 << tmpx2;
                }
                if(pga.size()!=thispga.size() && pga.size()<1)
                {
                    for (int j=0;j<thispga.size();j++)
                    {
                        pga << thispga[j];
                        pgax2 << thispgax2[j];
                    }
                }
                if (pga.size()==thispga.size())
                {
                    for (int j=0;j<thispga.size();j++)
                    {
                        if (thispga[j]>pga[j])
                        {
                            pga[j] = thispga[j];
                            pgax2[j] = thispgax2[j];
                        }
                    }
                }
            }
        }
        accFile.close();
    }

    if (m_pga.size()>0)
    {
        m_pga.clear();
        m_pgax2.clear();
    }
    for(int i=0;i<pga.size();i++)
    {
        m_pga.append( pga[i] / g );
        m_pgax2.append( pgax2[i] / g );
    }



    QFile saveFile(pgaFileName);
    if (!saveFile.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        qWarning("Couldn't open save file.");
    }
    QTextStream out(&saveFile);

    QFile saveFilex2(pgaFileNamex2);
    if (!saveFilex2.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        qWarning("Couldn't open save file.");
    }
    QTextStream outx2(&saveFilex2);


    for (int i=0;i<m_pga.size();i++)
    {
        out << QString::number(m_pga[i]) << "\n";
        outx2 << QString::number(m_pgax2[i]) << "\n";
    }
    saveFile.close();
    saveFilex2.close();





}


void PostProcessor::calcGamma()
{
    QString FileName = strainFileName;
    QFile File(FileName);
    QVector<double> v;
    QVector<double> v13;
    QVector<double> v23;
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
                QVector<double> thisv13;
                QVector<double> thisv23;
                int startpoint = dim==3 ? 4 : 3;
                int step = dim==3 ? 6 : 3;
                for (int i=startpoint; i<thisLine.size();i+=step)// TODO: 3D?
                {
                    double tmp = fabs(thisLine[i].trimmed().toDouble());
                    thisv << tmp;
                    if (dim==3)
                    {
                        thisv13 << fabs(thisLine[i+1].trimmed().toDouble());
                        thisv23 << fabs(thisLine[i+2].trimmed().toDouble());
                    }

                }
                if(v.size()!=thisv.size() && v.size()<1)
                {
                    for (int j=0;j<thisv.size();j++)
                    {
                        v << thisv[j];
                        if (dim==3)
                        {
                            v13 << thisv13[j];
                            v23 << thisv23[j];
                        }
                    }
                }
                if (v.size()==thisv.size())
                {
                    for (int j=0;j<thisv.size();j++)
                    {
                        if (thisv[j]>v[j])
                        {
                            v[j] = thisv[j];
                            if (dim==3)
                            {
                                v13[j] = thisv13[j];
                                v23[j] = thisv23[j];
                            }
                        }
                    }
                }
            }
        }
        File.close();
    }

    if (m_gamma.size()>0)
    {
        m_gamma.clear();
        if (dim==3)
        {
            m_gamma13.clear();m_gamma23.clear();
        }
    }
    for(int i=0;i<v.size();i++)
    {
        m_gamma.append( v[i] * 100 );
        if (dim==3)
        {
            m_gamma13.append( v13[i] * 100 );m_gamma23.append( v23[i] * 100 );
        }
    }



    QFile saveFile(gammaMaxFileName);
    if (!saveFile.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        qWarning("Couldn't open save file.");
    }
    QTextStream out(&saveFile);


    for (int i=0;i<m_gamma.size();i++)
        out << QString::number(m_gamma[i]) << "\n";
    saveFile.close();

}


void PostProcessor::calcSigma()// actually tao, not sigma
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
                int startpoint = dim==3 ? 4 : 3;
                int step = dim==3 ? 3 : 6;
                for (int i=startpoint; i<thisLine.size();i+=step)// TODO: 3D?
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
    QVector<double> vx2;
    QVector<double> v1x2;
    double thisDispx2;
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
                QVector<double> thisvx2;
                int startpoint = dim==3 ? 1 : 1;
                int step = dim==3 ? 8 : 4;
                for (int i=startpoint; i<thisLine.size();i+=step)// TODO: 3D?
                {
                    double tmp = (thisLine[i].trimmed().toDouble());
                    thisv << tmp;
                    double tmpx2 = (thisLine[i+1].trimmed().toDouble());
                    thisvx2 << tmpx2;
                }

                if(v.size()!=thisv.size() && v.size()<1)
                {
                    for (int j=0;j<thisv.size();j++)
                    {
                        v1 << thisv[j];
                        v << 0.0;
                        v1x2 << thisvx2[j];
                        vx2 << 0.0;
                    }
                }
                if (v.size()==thisv.size())
                {
                    for (int j=0;j<thisv.size();j++)
                    {
                        thisDisp = fabs(thisv[j]-thisv[0]);
                        if (thisDisp>v[j])
                            v[j] = thisDisp;

                        thisDispx2 = fabs(thisvx2[j]-thisvx2[0]);
                        if (thisDispx2>vx2[j])
                            vx2[j] = thisDispx2;
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

    if (m_dispx2.size()>0)
        m_dispx2.clear();
    for(int i=0;i<vx2.size();i++)
        m_dispx2.append( vx2[i] );



    QFile saveFile(dispMaxFileName);
    if (!saveFile.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        qWarning("Couldn't open save file.");
    }
    QTextStream out(&saveFile);


    for (int i=0;i<m_disp.size();i++)
        out << QString::number(m_disp[i]) << "\n";
    saveFile.close();

    QFile saveFilex2(dispMaxFileNamex2);
    if (!saveFilex2.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        qWarning("Couldn't open save file.");
    }
    QTextStream outx2(&saveFilex2);


    for (int i=0;i<m_dispx2.size();i++)
        outx2 << QString::number(m_dispx2[i]) << "\n";
    saveFilex2.close();

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
        double tmp = 0.0;
        while(!in.atEnd()) {
            QString line = in.readLine();
            QStringList thisLine = line.split(" ");
            if (thisLine.size()<2)
                break;
            else
            {
                thisLine.removeAll("");
                QVector<double> thisv;
                int startpoint = dim==3 ? 2 : 2;
                int step = dim==3 ? 6 : 3;
                for (int i=startpoint; i<thisLine.size();i+=step)// TODO: 3D?
                {
                    tmp = (thisLine[i].trimmed().toDouble());
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
                int startpoint = dim==3 ? 1 : 1;
                int step = dim==3 ? 4 : 2;
                for (int i=startpoint; i<thisLine.size();i+=step)// TODO: 3D?
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
                // for PIMY, PDMY, PDMY02 in 3D, there are 7 stresses output, not 6!
                int startpoint = dim==3 ? 2 : 2;
                int step = dim==3 ? 6 : 3;
                for (int i=startpoint; i<thisLine.size();i+=step)// TODO: 3D?
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

    if (m_initialStress.size()>0)
        m_initialStress.clear();
    for(int i=0;i<v1.size();i++)
        m_initialStress.append( v1[i] );



    QFile saveFile(rupwpFileName);
    if (!saveFile.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        qWarning("Couldn't open save file.");
    }
    QTextStream out(&saveFile);


    for (int i=0;i<m_rupwp.size();i++)
        out << QString::number(m_rupwp[i]) << "\n";
    saveFile.close();

}
