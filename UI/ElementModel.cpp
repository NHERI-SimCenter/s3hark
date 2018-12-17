#include "ElementModel.h"

ElementModel::ElementModel(QObject *parent)
    :QAbstractListModel(parent)
{
    mRoleNames[typeRole] = "eleType";
    mRoleNames[tagRole] = "tag";
    mRoleNames[iRole] = "i";
    mRoleNames[jRole] = "j";
    mRoleNames[kRole] = "k";
    mRoleNames[lRole] = "l";
    mRoleNames[hRole] = "h";
    mRoleNames[mcolorRole] = "mcolor";
}


int ElementModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return mRecords.size();

}
int ElementModel::rowCount() const
{
    return mRecords.size();
}



int ElementModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;   // qml never use column to retrive data
}
QModelIndex ElementModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if((row >= 0)&&(row < mRecords.size()))
    {
        return createIndex(row,column);
    }
    return QModelIndex();   //return an invalid null index
}


/*
QModelIndex ElementModel::parent(const QModelIndex &child) const
{
    return QModelIndex();  // there is parent node in 2D table
}
*/


QVariant ElementModel::data(const QModelIndex &index, int role) const
{
    //if(index.isValid())
        return mRecords[index.row()][role];
}



void ElementModel::addElement(QString type, int tag, int i, int j, int k, int l, double h, QString color)
{

    QHash<int,QVariant> row;
    row[typeRole] = type;
    row[tagRole] = QString::number(tag);
    row[iRole] = QString::number(i);
    row[jRole] = QString::number(j);
    row[kRole] = QString::number(k);
    row[lRole] = QString::number(l);
    row[hRole] = QString::number(h);
    row[mcolorRole] = color;
    mRecords.append(row);


}

void ElementModel::clear()
{

    beginResetModel();
    mRecords.clear();

}

void ElementModel::refresh()
{

    endResetModel();

}



QHash<int, QByteArray> ElementModel::roleNames() const
{

    return mRoleNames;
}
