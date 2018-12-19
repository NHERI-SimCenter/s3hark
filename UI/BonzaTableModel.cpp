#include "BonzaTableModel.h"
#include <QSqlField>
QList<QVariant> BonzaTableModel::getRowInfo(int r) const
{
    QList<QVariant> list;

    QSqlRecord record = this->record(r);  //get the data in the current row
    for( int i = LAYERNAME; i < record.count(); ++i)//read data starting from LayerName
    {
        QSqlField field = record.field(i);
        QString data = field.value().toString();
        list.append(data);
    }

    return list;
}

