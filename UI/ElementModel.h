#ifndef ELEMENTMODEL_H
#define ELEMENTMODEL_H

#include <QAbstractListModel>


class ElementModel : public QAbstractListModel
{
    Q_OBJECT
    //Q_PROPERTY(double m_nGWT READ getGWT WRITE setGWT NOTIFY gwtChanged)
    //(double m_nGWT MEMBER m_nGWT NOTIFY gwtChanged)

public:
    enum ElementRoles {
        typeRole = Qt::UserRole + 1,
        tagRole,
        iRole,
        jRole,
        kRole,
        lRole,
        hRole,
        mcolorRole
    };

    ElementModel(QObject *parent = nullptr);


    QHash<int,QByteArray>roleNames() const;
    int rowCount(const QModelIndex &parent=QModelIndex()) const;
    int rowCount() const;
    int columnCount(const QModelIndex &parent=QModelIndex()) const;
    QModelIndex index(int row, int column, const QModelIndex &parent=QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role=Qt::DisplayRole) const;

    void addElement(QString type, int tag, int i, int j, int k, int l, double h, QString color);
    void clear();
    void refresh();
    void setWidth(double w){m_w = w;}
public slots:
    double getWidth(){return m_w;}
private:
    QHash<int,QByteArray> mRoleNames;
    QList<QHash<int,QVariant>> mRecords;
    double m_w ;
};



#endif // ELEMENTMODEL_H
