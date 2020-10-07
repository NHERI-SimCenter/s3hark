#ifndef ELEMENTMODEL_H
#define ELEMENTMODEL_H

#include <QAbstractListModel>
#include "Mesher.h"


class ElementModel : public QAbstractListModel
{
    Q_OBJECT
    //Q_PROPERTY(double m_nGWT READ getGWT WRITE setGWT NOTIFY gwtChanged)
    //(double m_nGWT MEMBER m_nGWT NOTIFY gwtChanged)
    Q_PROPERTY(int activeID READ getActiveID NOTIFY activeIDChanged)



public:
    enum ElementRoles {
        typeRole = Qt::UserRole + 1,
        tagRole,
        iRole,
        jRole,
        kRole,
        lRole,
        hRole,
        mcolorRole,
        activeRole
    };

    ElementModel(QObject *parent = nullptr);


    QHash<int,QByteArray>roleNames() const;
    int rowCount(const QModelIndex &parent=QModelIndex()) const;
    int rowCount() const;
    int getSize(){return mRecords.size();}
    int columnCount(const QModelIndex &parent=QModelIndex()) const;
    QModelIndex index(int row, int column, const QModelIndex &parent=QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role=Qt::DisplayRole) const;

    void addElement(QString type, int tag, int i, int j, int k, int l, double h, QString color, bool active);
    void clear();
    void refresh();
    void setWidth(double w){m_w = w;}
    void setTotalHeight(double h){m_h = h;}
    void setNodes(std::vector<Nodex*> nods){nodes = nods;}

    Q_INVOKABLE void setActive(int row)
    {
        activeID = row;
        for (int i=0;i<mRecords.size();i++)
            mRecords[i][activeRole] = false;
        mRecords[row][activeRole] = true;
        emit dataChanged(index(0,activeRole),index(mRecords.size()-1,activeRole));
        emit activeIDChanged(row); //this is emitted from tabmanager
    }


signals:
    void activeIDChanged(int actID);

public slots:
    double getWidth(){return m_w;}
    double getTotalHeight(){
        return m_h;}
    double getCurrentHeight(){
        double hpos = 0.0;
        for (int i=0;i<activeID;i++)
            hpos += mRecords[i][hRole].toDouble();
        return hpos;}
    int getActiveID() {return activeID;}
    void reSetActive()
    {
       setActive(getActiveID());
    }
private:
    QHash<int,QByteArray> mRoleNames;
    QList<QHash<int,QVariant>> mRecords;
    double m_w ;
    double m_h = 0.0;
    int activeID = 0;
    std::vector<Nodex*> nodes;
};



#endif // ELEMENTMODEL_H
