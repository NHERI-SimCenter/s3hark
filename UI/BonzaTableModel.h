#ifndef BONZATABLEMODEL_H
#define BONZATABLEMODEL_H

#include "GlobalConstances.h"

#include <QSqlTableModel>
#include <QTableView>
#include <QEvent>
#include <QThread>
#include <QSqlRecord>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QItemDelegate>
#include <QApplication>
#include <QMouseEvent>
#include <QHeaderView>
#include <QComboBox>
#include <QRandomGenerator>
#include <QColor>

#include <QDebug>

class QSqlDatabase;

class BonzaTableModel : public QSqlTableModel
{
    Q_OBJECT

signals:
    void thicknessEdited();
    void rowActivated(int row);

public:
        explicit BonzaTableModel(QWidget *parent = nullptr, QSqlDatabase db = QSqlDatabase()):
        QSqlTableModel(parent, db)
    {
    }


    Qt::ItemFlags flags( const QModelIndex &index ) const
    {
        if(!index.isValid())
            return nullptr;

        if ( index.column() == CHECKED )
            return (QSqlTableModel::flags(index) & Qt::ItemIsEditable);

        return  QSqlTableModel::flags(index);
    }

    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole )
    {
        if(!index.isValid())
            return false;

        if( index.column() != CHECKED )
        {
            QSqlTableModel::setData(index, value, Qt::EditRole);
            if(index.column()==THICKNESS)
                emit thicknessEdited();

            return submitAll();
        }

        QSqlTableModel::setData(index, value, Qt::EditRole);
        return submitAll();

    }

    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const
    {
        if(!index.isValid())
            return QVariant();

        if (role == Qt::DisplayRole)
        {
            if ( index.column() == CHECKED )
            {
                int checked = this->record(index.row()).value(CHECKED).toInt();
                return  checked;// == 1 ? Qt::Checked : Qt::Unchecked;
            }
        }

        return QSqlTableModel::data(index, role);
    }
    Q_INVOKABLE QString getLayerName(int row)
    {
        return this->record(row).value(LAYERNAME).toString();
    }
    Q_INVOKABLE double getThickness(int row)
    {
        return this->record(row).value(THICKNESS).toDouble();
    }
    Q_INVOKABLE double getBotompos(int row)
    {
        double botompos = 0.0;
        int i;
        for (i=0;i<=row;++i)
            botompos+=this->record(i).value(THICKNESS).toDouble();
        return botompos;
    }
    Q_INVOKABLE double getToppos(int row)
    {
        double toppos = 0.0;
        int i;
        for (i=0;i<row;++i)
            toppos+=this->record(i).value(THICKNESS).toDouble();
        return toppos;
    }
    Q_INVOKABLE QString getSoilColor(int row)
    {
        Q_UNUSED(row)
        return this->record(row).value(COLOR).toString();
    }
    Q_INVOKABLE double getTotalHeight()
    {
        double totalHeight = 0.0;
        int i;
        for (i=0;i<this->rowCount() ;++i)
            totalHeight += this->record(i).value(THICKNESS).toDouble();
        return totalHeight;
    }

    Q_INVOKABLE bool isActive(int row)
    {
        return this->record(row).value(CHECKED).toBool();
    }

    Q_INVOKABLE void setActive(int row)
    {
        int i;

        for (i=0;i<this->rowCount() ;++i)
            setData(this->index(i, CHECKED), "0");

        setData(this->index(row, CHECKED), "1");

        submitAll();

        //emit rowActivated(row);

    }


    Q_INVOKABLE void setActiveFromView(int row)
    {
        int i;
        for (i=0;i<this->rowCount() ;++i)
            setData(this->index(i, CHECKED), "0");

        setData(this->index(row, CHECKED), "1");
        submitAll();
        //emit rowActivated(row);
        qDebug()<< "row " << row << " activated., said the model.";
    }

    Q_INVOKABLE void deActivateAll()
    {
        int i;
        for (i=0;i<this->rowCount() ;++i)
            setData(this->index(i, CHECKED), "0");
        submitAll();
    }

public slots:
    void setActive(const QModelIndex &index)
    {
/*
        int i;
        for (i=0;i<this->rowCount() ;++i)
            setData(this->index(i, CHECKED), "0");

        setData(this->index(index.row(), CHECKED), "1");
        submitAll();
*/
        //emit rowActivated(index.row());
        //setActive(index.row());
        qDebug() << "model slot says: activated: " << index.row();

    }

    QList<QVariant> getRowInfo(int row) const;


};

#endif // BONZATABLEMODEL_H
