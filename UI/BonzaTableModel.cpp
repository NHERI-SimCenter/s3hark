#include "BonzaTableModel.h"


Qt::ItemFlags BonzaTableModel::flags( const QModelIndex &index ) const
{
    if(!index.isValid())
        return nullptr;

    if ( index.column() == CHECKED )
        return (QAbstractTableModel::flags(index) & Qt::ItemIsEditable);

    return  (QAbstractTableModel::flags(index) | Qt::ItemIsEditable) ;
}

int BonzaTableModel::columnCount(const QModelIndex &parent) const
{
    return LASTCOL+1;
}

int BonzaTableModel::rowCount(const QModelIndex &parent) const
{
    return numRow;
}

bool BonzaTableModel::insertRows(int position, int rows, const QModelIndex &index)
 {
    Q_UNUSED(index);
    beginInsertRows(QModelIndex(), position, position+rows-1);

    for (int row=0; row < rows; row++) {
        layerIDVec.insert(position, "");
        checkedVec.insert(position, "");
        layerNameVec.insert(position, "");
        thicknessVec.insert(position, "");
        densityVec.insert(position, "");
        vsVec.insert(position, "");
        materialVec.insert(position, "");
        esizeVec.insert(position, "");
        colorVec.insert(position, "");
        femVec.insert(position, "");
    }

    numRow += rows;
    endInsertRows();

    return true;
 }

bool BonzaTableModel::removeRows(int position, int rows, const QModelIndex &index)
 {
     Q_UNUSED(index);
     beginRemoveRows(QModelIndex(), position, position+rows-1);

     for (int row=0; row < rows; ++row) {
         layerIDVec.removeAt(position);
         checkedVec.removeAt(position);
         layerNameVec.removeAt(position);
         thicknessVec.removeAt(position);
         densityVec.removeAt(position);
         vsVec.removeAt(position);
         materialVec.removeAt(position);
         esizeVec.removeAt(position);
         colorVec.removeAt(position);
         femVec.removeAt(position);
     }

     numRow -= rows;
     endRemoveRows();

     return true;
 }

bool BonzaTableModel::setData( const QModelIndex &index, const QVariant &value, int role)
{

    if(!index.isValid())
        return false;

    int ir = index.row();
    int ic = index.column();
    QModelIndex ind = createIndex(ir, ic);
    int numLayers = this->rowCount();

    if (abs(ir - (numLayers-1)) < 1e-5 && numLayers==1) // First time to add Rock layer
    {
        bool rockEditted = editData(ir, ic, value, Qt::EditRole);
        emit dataChanged(ind, ind);
        return rockEditted;
    }

    //if ( ic != CHECKED )
    {
        bool dataSetted = editData(ir, ic, value);
        emit dataChanged(ind, ind);
        if(ic == THICKNESS || ic == ESIZE || ic == COLOR)
            emit thicknessEdited();

        return dataSetted;
    }

}

bool BonzaTableModel::setData( int ir, int ic, const QVariant &value, int role)
{


    int numLayers = this->rowCount();

    QModelIndex ind = createIndex(ir, ic);

    if (abs(ir - (numLayers-1)) < 1e-5 && numLayers==1) // First time to add Rock layer
    {
        bool rockEdited = editData(ir, ic, value, Qt::EditRole);
        emit dataChanged(ind, ind);
        return rockEdited;
    }

    //if ( ic != CHECKED )
    {
        bool dataSetted = editData(ir, ic, value);
        emit dataChanged(ind, ind);
        //emit thicknessEdited();
        return dataSetted;
    }

}

bool BonzaTableModel::editData( int row, int col, const QVariant &value, int role)
{
    switch (col) {
    case CHECKED:
        checkedVec[row] = value.toString();
        break;
    case LayerID:
        layerIDVec[row] = value.toString();
        break;
    case LAYERNAME   :
        layerNameVec[row] = value.toString();
        break;
    case THICKNESS:
        thicknessVec[row] = value.toString();
        break;
    case DENSITY:
        densityVec[row] = value.toString();
        break;
    case VS:
        vsVec[row] = value.toString();
        break;
    case MATERIAL:
        materialVec[row] = value.toString();
        break;
    case ESIZE:
        esizeVec[row] = value.toString();
        break;
    case COLOR:
        colorVec[row] = value.toString();
        break;
    case FEM:
        femVec[row] = value.toString();
        break;
    }
    return true;
}

bool BonzaTableModel::addData( int row, int col, const QVariant &value, int role)
{
    switch (col) {
    case CHECKED:
        checkedVec.push_back(value.toString());
        break;
    case LayerID:
        layerIDVec.push_back(value.toString());
        break;
    case LAYERNAME   :
        layerNameVec.push_back(value.toString());
        break;
    case THICKNESS:
        thicknessVec.push_back(value.toString());
        break;
    case DENSITY:
        densityVec.push_back(value.toString());
        break;
    case VS:
        vsVec.push_back(value.toString());
        break;
    case MATERIAL:
        materialVec.push_back(value.toString());
        break;
    case ESIZE:
        esizeVec.push_back(value.toString());
        break;
    case COLOR:
        colorVec.push_back(value.toString());
        break;
    case FEM:
        femVec.push_back(value.toString());
        break;
    }
    return true;
}


bool BonzaTableModel::setDataSilent( const QModelIndex &index, const QVariant &value, int role )
{
    if(!index.isValid())
        return false;

    int ir = index.row();
    int ic = index.column();
    int numLayers = this->rowCount();

    if (abs(ir - (numLayers-1)) < 1e-5 && numLayers==1) // First time to add Rock layer
    {
        return QAbstractTableModel::setData(index, value, Qt::EditRole);
    }

    if ( ic != CHECKED )
    {
        return QAbstractTableModel::setData(index, value, Qt::EditRole);
    }

    return QAbstractTableModel::setData(index, value, Qt::EditRole);
}


QVariant BonzaTableModel::data( const QModelIndex &index, int role ) const
{
    if(!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole | role == Qt::EditRole)
    {
        switch (index.column()) {
        case CHECKED:
            return checkedVec[index.row()];
        case LayerID:
            return layerIDVec[index.row()];
        case LAYERNAME   :
            return layerNameVec[index.row()];
        case THICKNESS:
            return thicknessVec[index.row()];
        case DENSITY:
            return densityVec[index.row()];
        case VS:
            return vsVec[index.row()];
        case MATERIAL:
            return materialVec[index.row()];
        case ESIZE:
            return esizeVec[index.row()];
        case COLOR:
            return colorVec[index.row()];
        case FEM:
            return femVec[index.row()];
        }
    }
    return QVariant();

}

bool BonzaTableModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{

    if(role==Qt::EditRole && orientation==Qt::Horizontal)
    {
        if(header.count() < section+1)
            header.append(value.toString());
        else
            header[section]=value.toString();
        return true;
    } else {
        return true;
    }

}
QVariant BonzaTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role==Qt::DisplayRole && orientation==Qt::Horizontal)
        return header[section];
    return QAbstractTableModel::headerData(section,orientation,role);
}

QList<QVariant> BonzaTableModel::getRowInfo(int r) const
{
    QList<QVariant> list;

    //QSqlRecord record = this->record(r);  //get the data in the current row
    for( int i = LAYERNAME; i <= LASTCOL; ++i)//read data starting from LayerName
    {
        QString d = data(createIndex(r, i)).toString();
        list.append(d);
    }

    return list;
}



