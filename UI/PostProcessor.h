#ifndef POSTPROCESSOR_H
#define POSTPROCESSOR_H

#include <QObject>
#include <QDialog>
#include <QTabWidget>
#include <QFile>
#include <QTextStream>
#include <math.h>

class PostProcessor : public QDialog
{
    Q_OBJECT
public:
    explicit PostProcessor(QWidget *parent = nullptr);
    PostProcessor(QTabWidget *tab,QWidget *parent = nullptr);
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

};

#endif // POSTPROCESSOR_H
