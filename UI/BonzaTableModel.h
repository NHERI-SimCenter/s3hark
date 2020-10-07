#ifndef BONZATABLEMODEL_H
#define BONZATABLEMODEL_H

#include "GlobalConstances.h"
#include <QAbstractTableModel>
#include <QTableView>
#include <QEvent>
#include <QThread>
#include <QPainter>

#include <QApplication>
#include <QMouseEvent>
#include <QHeaderView>
#include <QComboBox>
#include <QRandomGenerator>
#include <QColor>

#include <QDebug>


class BonzaTableModel : public QAbstractTableModel
{
    Q_OBJECT

signals:
    void thicknessEdited();
    void rowActivated(int row);

public:
    explicit BonzaTableModel(QObject *parent = nullptr):
        QAbstractTableModel(parent){}
    Qt::ItemFlags flags( const QModelIndex &index ) const;
    virtual int rowCount(const QModelIndex &parent=QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent=QModelIndex()) const;
    bool insertRows(int position, int rows, const QModelIndex &index=QModelIndex());
    bool removeRows(int position, int rows, const QModelIndex &index=QModelIndex());
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::DisplayRole );
    bool setData( int ir, int ic, const QVariant &value, int role = Qt::DisplayRole);
    bool setDataSilent( const QModelIndex &index, const QVariant &value, int role = Qt::DisplayRole );
    bool addData( int row, int col, const QVariant &value, int role = Qt::DisplayRole);
    bool editData( int row, int col, const QVariant &value, int role = Qt::DisplayRole);
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole);



    Q_INVOKABLE bool isActive(int row)
    {
        bool thisActive = data(this->index(row, CHECKED)).toBool();;
        return thisActive;
    }
    Q_INVOKABLE void setActive(int row)
    {
        beginResetModel();
        for (int i=0;i<numRow ;++i)
        {
            QModelIndex ind = this->index(i, CHECKED);
            int thisValue = data(ind).toInt();
            if(abs(i-row)>1e-5 && thisValue>0)
            {
                setData(ind, "0");
            }
        }

        setData(this->index(row, CHECKED), "1");

        //emit dataChanged(this->index(0, CHECKED), this->index(numRow-1, CHECKED));
        endResetModel();
    }

    Q_INVOKABLE QString getLayerName(int row)
    {
        return data(createIndex(row,LAYERNAME)).toString();
        //return this->record(row).value(LAYERNAME).toString();//commentednotsure
    }

    Q_INVOKABLE double getThickness(int row)
    {
        return data(createIndex(row,THICKNESS)).toDouble();
        //return this->record(row).value(THICKNESS).toDouble();//commentednotsure
    }

    Q_INVOKABLE double getBotompos(int row)
    {
        double botompos = 0.0;
        int i;
        for (i=0;i<=row;++i)
            botompos += data(createIndex(i,THICKNESS)).toDouble();
            //botompos+=this->record(i).value(THICKNESS).toDouble();//commentednotsure
        return botompos;
    }

    Q_INVOKABLE double getToppos(int row)
    {
        double toppos = 0.0;
        int i;
        for (i=0;i<row;++i)
            toppos += data(createIndex(i, THICKNESS)).toDouble();
            //toppos+=this->record(i).value(THICKNESS).toDouble();//commentednotsure
        return toppos;
    }

    Q_INVOKABLE QString getSoilColor(int row)
    {
        Q_UNUSED(row)
        //return this->record(row).value(COLOR).toString();//commentednotsure
        return data(createIndex(row, COLOR)).toString();
    }

    Q_INVOKABLE double getTotalHeight()
    {
        double totalHeight = 0.0;
        int i;
        for (i=0;i<this->rowCount() ;++i)
            totalHeight += data(createIndex(i, THICKNESS)).toDouble();//commentednotsure
        return totalHeight;
    }

    Q_INVOKABLE int has2DOnlyModel()
    {
        int layerContaining2D = 0;
        QString matTypeTmp;
        for (int i=0;i<this->rowCount() ;++i){
            matTypeTmp = data(createIndex(i, MATERIAL)).toString();//commentednotsure
            if(matTypeTmp == "PM4Sand" || matTypeTmp == "PM4Silt")
            {
                layerContaining2D = i+1;
                break;
            }
        }
        return layerContaining2D;
    }

    Q_INVOKABLE int has3DOnlyModel()
    {
        int layerContaining3D = 0;
        QString matTypeTmp;
        for (int i=0;i<this->rowCount() ;++i){
            matTypeTmp = data(createIndex(i, MATERIAL)).toString();//commentednotsure
            if(matTypeTmp == "J2Bounding")
            {
                layerContaining3D = i+1;
                break;
            }
        }
        return layerContaining3D;
    }


    Q_INVOKABLE void setActiveFromView(int row)
    {

        for (int i=0;i<numRow ;++i)
            setData(this->index(i, CHECKED), "0");

        setData(this->index(row, CHECKED), "1");

        QModelIndex indtop = createIndex(0, CHECKED);
        QModelIndex indbot = createIndex(numRow-1, CHECKED);
        emit dataChanged(indtop, indbot, { Qt::EditRole, Qt::DisplayRole });

        //submitAll(); //commentednotsure
        //emit rowActivated(row);
        qDebug()<< "row " << row << " activated., said the model.";
    }

    Q_INVOKABLE void deActivateAll()
    {
        int i;
        for (i=0;i<this->rowCount() ;++i)
            setData(this->index(i, CHECKED), "0");
        //submitAll();//commentednotsure
    }


private:
    QVector<QString> layerIDVec;
    QVector<QString> checkedVec;
    QVector<QString> layerNameVec;
    QVector<QString> thicknessVec;
    QVector<QString> densityVec;
    QVector<QString> vsVec;
    QVector<QString> materialVec;
    QVector<QString> esizeVec;
    QVector<QString> colorVec;
    QVector<QString> femVec;

    int numRow = 0;

    QStringList  header = {"","","","","","","","","","","","","","",""};


public slots:
    void setActive(const QModelIndex &index)
    {
        qDebug() << "model slot says: activated: " << index.row();
    }
    QList<QVariant> getRowInfo(int row) const;





};

#endif // BONZATABLEMODEL_H
