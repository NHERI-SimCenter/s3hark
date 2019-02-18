#ifndef POSTPROCESSOR_H
#define POSTPROCESSOR_H

#include <QObject>
#include <QDialog>
#include <QTabWidget>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <math.h>

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
    void calcDisp();
    void calcRu();
    void calcRuDepths();
    int getEleCount();

    QVector<double> getPga(){return m_pga;}
    QVector<double> getDepths(){return m_depths;}
    QVector<double> getRuDepths(){return m_ruDepths;}
    QVector<double> getGamma(){return m_gamma;}
    QVector<double> getDisp(){return m_disp;}
    QVector<double> getRu(){return m_ru;}

signals:
    void updateFinished();
private:
    QVector<double> m_pga;
    QVector<double> m_depths;
    QVector<double> m_ruDepths;
    QVector<double> m_gamma;
    QVector<double> m_disp;
    QVector<double> m_ru;
    double g=9.81;
    int eleCount;
    QString m_outputDir;
    // opensees results
    QString elementFileName = QDir(m_outputDir).filePath("elementInfo.dat");
    QString nodesFileName = QDir(m_outputDir).filePath("nodesInfo.dat");
    QString dephsFileName = QDir(m_outputDir).filePath("depths.dat");
    QString accFileName = QDir(m_outputDir).filePath("acceleration.out");
    QString dispFileName = QDir(m_outputDir).filePath("displacement.out");
    QString strainFileName = QDir(m_outputDir).filePath("strain.out");
    QString stressFileName = QDir(m_outputDir).filePath("stress.out");

    // processed dat
    QString pgaFileName = QDir(m_outputDir).filePath("pga.dat");
    QString ruFileName = QDir(m_outputDir).filePath("ru.dat");
    QString gammaMaxFileName = QDir(m_outputDir).filePath("gammaMax.dat");
    QString dispMaxFileName = QDir(m_outputDir).filePath("dispMax.dat");







};

#endif // POSTPROCESSOR_H
