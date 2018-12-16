#ifndef MESHER_H
#define MESHER_H

#include <QObject>
#include <QWidget>
#include "BonzaTableModel.h"
#include <vector>

class Node
{

public:
    Node(int tag, double x, double y, double z) :
    m_tag(tag),m_x(x),m_y(y),m_z(z)
    {}
    double x(){return m_x;}
    double y(){return m_y;}
    double z(){return m_z;}
    int tag(){return m_tag;}

private:
    int m_tag;
    double m_x;
    double m_y;
    double m_z;
};

class quad
{
    quad(int tag, int i, int j, int k, int l) :
    m_tag(tag),m_i(i),m_j(j),m_k(k),m_l(l)
    {}
public:
    int i(){return m_i;}
    int j(){return m_j;}
    int k(){return m_k;}
    int l(){return m_l;}
    int tag(){return m_tag;}
private:
    int m_tag;
    int m_i;
    int m_j;
    int m_k;
    int m_l;
};


class Mesher: public QObject
{
    Q_OBJECT
public:
    Mesher();
    Mesher(BonzaTableModel* tableModel);
    bool mesh2DColumn();

    std::vector<Node*> nodes;
    std::vector<quad*> elements;

public slots:


private:
    BonzaTableModel* m_tableModel;

};

#endif // MESHER_H
