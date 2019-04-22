#ifndef POSTPROCESSOR_H
#define POSTPROCESSOR_H

#include <QObject>
#include <QDialog>
#include <QTabWidget>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <math.h>
#include <QApplication>
#include <QStandardPaths>

class PostProcessor : public QDialog
{
    Q_OBJECT
public:
    explicit PostProcessor(QWidget *parent = nullptr);
    PostProcessor(QTabWidget *tab,QWidget *parent = nullptr);
    PostProcessor(QString outDir) : m_outputDir(outDir){}
    void calcPGA();
    void update();
    void calcDepths();
    void calcGamma();
    void calcSigma();
    void calcDisp();
    void calcRu();
    void calcRupwp();
    void calcRuDepths();

    double newmark(double mass,double damping, double k, double disp_init, double vel_init, double gamma, double beta, double dt, QVector<double> f );


    void loadMotions();
    void calcMotion(QString, QString);
    void calcAllMotion(QString motion);
    void calcSa();

    void calcSurfaceMotions();
    int getEleCount();

    QVector<double> getPga(){return m_pga;}
    QVector<double> getDepths(){return m_depths;}
    QVector<double> getRuDepths(){return m_ruDepths;}
    QVector<double> getGamma(){return m_gamma;}
    QVector<double> getSigma(){return m_sigma;}
    QVector<double> getDisp(){return m_disp;}
    QVector<double> getRu(){return m_ru;}
    QVector<double> getRupwp(){return m_rupwp;}
    QVector<double> getInitialStress(){return m_initialStress;}

    QString getElementFileName(){return elementFileName;}
    QString getNodesFileName(){return nodesFileName;}
    QString getDephsFileName(){return dephsFileName;}
    QString getAccFileName(){return accFileName;}
    QString getDispFileName(){return dispFileName;}
    QString getVelFileName(){return velFileName;}
    QString getStrainFileName(){return strainFileName;}
    QString getStressFileName(){return stressFileName;}
    QString getPWPFileName(){return pwpFileName;}

    QStringList * getxdBaseVel(){return xdBaseVel;}
    QStringList * getydBaseVel(){return ydBaseVel;}
    QStringList * getxdSurfaceVel(){return xdSurfaceVel;}
    QStringList * getydSurfaceVel(){return ydSurfaceVel;}

    QStringList * getxdBaseDisp(){return xdBaseDisp;}
    QStringList * getydBaseDisp(){return ydBaseDisp;}
    QStringList * getxdSurfaceDisp(){return xdSurfaceDisp;}
    QStringList * getydSurfaceDisp(){return ydSurfaceDisp;}

    QStringList * getxdBaseAcc(){return xdBaseAcc;}
    QStringList * getydBaseAcc(){return ydBaseAcc;}
    QStringList * getxdSurfaceAcc(){return xdSurfaceAcc;}
    QStringList * getydSurfaceAcc(){return ydSurfaceAcc;}

    QVector<QVector<double>> *getvelAll(){return velAll;}
    QVector<QVector<double>> *getaccAll(){return accAll;}
    QVector<QVector<double>> *getdispAll(){return dispAll;}

    QVector<QVector<double>> *getSa(){return saVec;}
    QVector<double> *getPeriods(){return Periods;}



signals:
    void updateFinished();
private:
    QString rootDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);//qApp->applicationDirPath();
    QString analysisName = "analysis";
    QString analysisDir = QDir(rootDir).filePath(analysisName);

    QVector<double> m_pga;
    QVector<double> m_depths;
    QVector<double> m_ruDepths;
    QVector<double> m_gamma;
    QVector<double> m_sigma;
    QVector<double> m_disp;
    QVector<double> m_ru;
    QVector<double> m_rupwp;
    QVector<double> m_initialStress;
    double g=9.81;
    int eleCount;
    QString m_outputDir;
    // opensees results
    QString elementFileName = QDir(m_outputDir).filePath("elementInfo.dat");
    QString nodesFileName = QDir(m_outputDir).filePath("nodesInfo.dat");
    QString dephsFileName = QDir(m_outputDir).filePath("depths.dat");
    QString accFileName = QDir(m_outputDir).filePath("acceleration.out");
    QString dispFileName = QDir(m_outputDir).filePath("displacement.out");
    QString velFileName = QDir(m_outputDir).filePath("velocity.out");
    QString strainFileName = QDir(m_outputDir).filePath("strain.out");
    QString stressFileName = QDir(m_outputDir).filePath("stress.out");
    QString pwpFileName = QDir(m_outputDir).filePath("porePressure.out");

    // processed dat
    QString pgaFileName = QDir(m_outputDir).filePath("pga.dat");
    QString ruFileName = QDir(m_outputDir).filePath("ru.dat");
    QString rupwpFileName = QDir(m_outputDir).filePath("rupwp.dat");
    QString gammaMaxFileName = QDir(m_outputDir).filePath("gammaMax.dat");
    QString sigmaMaxFileName = QDir(m_outputDir).filePath("sigmaMax.dat");
    QString dispMaxFileName = QDir(m_outputDir).filePath("dispMax.dat");

    QStringList *xdBaseVel = new QStringList();
    QStringList *ydBaseVel = new QStringList();
    QStringList *xdBaseDisp = new QStringList();
    QStringList *ydBaseDisp = new QStringList();
    QStringList *xdBaseAcc = new QStringList();
    QStringList *ydBaseAcc = new QStringList();
    QStringList *xdSurfaceVel = new QStringList();
    QStringList *ydSurfaceVel = new QStringList();
    QStringList *xdSurfaceDisp = new QStringList();
    QStringList *ydSurfaceDisp = new QStringList();
    QStringList *xdSurfaceAcc = new QStringList();
    QStringList *ydSurfaceAcc = new QStringList();

    QVector<QVector<double>> *velAll = new QVector<QVector<double>>;
    QVector<QVector<double>> *accAll = new QVector<QVector<double>>;
    QVector<QVector<double>> *dispAll = new QVector<QVector<double>>;

    QVector<double> *Periods = new QVector<double>;
    QVector<QVector<double>> *saVec = new QVector<QVector<double>>;








};

#endif // POSTPROCESSOR_H
