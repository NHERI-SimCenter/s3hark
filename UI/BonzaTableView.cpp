#include "BonzaTableView.h"
#include "DatabaseManager.h"

#include <QSqlQueryModel>
#include <QSqlError>
#include <QSqlQuery>
#include <QHeaderView>
#include <QFocusEvent>
#include <QMenu>
#include <QAction>
#include <QSqlField>
#include <QAbstractItemView>
#include <QtMath>



BonzaTableView::BonzaTableView(QWidget *parent) :
    QTableView(parent),
    m_nStartId(0), m_nPageSize(MAXLAYERS), m_nCurPageSize(0), m_nTotal(0),
    m_nCurPage(1), m_nTotalPage(0), m_ntotalHeight(10), m_nGWT(0)
{
    DatabaseManager *dbMgr = new DatabaseManager;
    dbMgr->createConnect();
    dbMgr->createTable();
    dbMgr->setStatus(DatabaseManager::Read);
    QSqlDatabase db = dbMgr->database();

    // initialize view
    m_sqlModel = new BonzaTableModel(nullptr, db);
    m_sqlModel->moveToThread(&m_thread);
    m_thread.start();
    m_sqlModel->setTable(g_dbTableName);
    //m_sqlModel->setEditStrategy(QSqlTableModel::OnFieldChange);
    m_sqlModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    m_sqlModel->select();
    m_sqlModel->setHeaderData(CHECKED, Qt::Horizontal, tr("√"));
    m_sqlModel->setHeaderData(LAYERNAME, Qt::Horizontal, tr("LayerName"));
    m_sqlModel->setHeaderData(THICKNESS, Qt::Horizontal, tr("Thickness"));
    m_sqlModel->setHeaderData(DENSITY, Qt::Horizontal, tr("Density"));
    m_sqlModel->setHeaderData(VS, Qt::Horizontal, tr("Vs"));
    m_sqlModel->setHeaderData(COLOR, Qt::Horizontal, tr("Color"));
    m_sqlModel->setHeaderData(MATERIAL, Qt::Horizontal, tr("Material"));




    m_nTotal = m_sqlModel->rowCount();
    m_nTotalPage = totalPage();
    m_nLastPageSize = lastPageSize();
    this->updateModel();    //update the table on each page
    this->setModel(m_sqlModel);
    this->hideColumn(0);
    this->hideColumn(FEM);
    this->verticalHeader()->hide();

    this->setColumnHidden(1,true);

    //this->setItemDelegate(new TableViewItemDelegate(this));
    this->setItemDelegateForColumn(MATERIAL,new MatItemDelegate(this));
    this->setItemDelegateForColumn(LAYERNAME,new TableViewItemDelegate(this));
    this->setItemDelegateForColumn(THICKNESS,new TableViewItemDelegate(this));
    this->setItemDelegateForColumn(DENSITY,new TableViewItemDelegate(this));
    this->setItemDelegateForColumn(VS,new TableViewItemDelegate(this));
    this->setItemDelegateForColumn(COLOR,new TableViewItemDelegate(this));


    //setEditTriggers(QAbstractItemView::SelectedClicked);
    connect(this, SIGNAL(clicked(const QModelIndex &)), this, SLOT(onCellSingleClicked(const QModelIndex &)));
    connect(this, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(onCellDoubleClicked(const QModelIndex &)));


    QMap<QString,double> m_layerData;
    m_layerData["LayerName"] = 1.;
    m_layerData["Thickness"] = 1.;
    m_LayerVector.push_back(m_layerData);

    QMap<QString,double> m_layerData2;
    m_layerData2["LayerName"] = 1.;
    m_layerData2["Thickness"] = 1.;
    m_LayerVector.push_back(m_layerData2);

    QMap<QString,double> m_layerData3;
    m_layerData3["LayerName"] = 1.;
    m_layerData3["Thickness"] = 1.;
    m_LayerVector.push_back(m_layerData3);

    layerParsName.push_back("Layer1");
    layerParsValue.push_back(1.);

    layerParsName.push_back("Layer2");
    layerParsValue.push_back(2.);

    layerParsName.push_back("Layer3");
    layerParsValue.push_back(3.);

}

BonzaTableView::~BonzaTableView()
{
    m_sqlModel->submitAll();
    if(m_thread.isRunning())
    {
        m_thread.quit();
        m_thread.wait();
    }
    m_sqlModel->clear();
    m_sqlModel->deleteLater();
}




/**
 * when a cell is clicked
 * 1. call setActive to set active row in the model
 * 2. check if this is a double click
 * @brief BonzaTableView::onCellSingleClicked
 * @return
 */
void BonzaTableView::onCellSingleClicked(const QModelIndex &index)
{

    emit cellClicked(index);

    int clickedRow = index.row();
    int clickedCol = index.column();
    // if this row is alrady selected, regard the current single click as a double click
    bool cellDoubleClicked = m_sqlModel->isActive(clickedRow);

    qDebug() << "view says: I feel row " << index.row() << " column " << index.column() << " activated" ;

    if(index.column()==MATERIAL)
        qDebug() << "material clicked";

    setActive(index.row(), index.column());

    // double click simulator
    if (cellDoubleClicked)
    {
        QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier, QString());
        QCoreApplication::sendEvent(this, &keyPress);
    }






}

/**
 * copy data in the current selected row
 * into the dialog of adding new row
 * @brief BonzaTableView::currentRowInfo
 * @return
 */
QList<QVariant> BonzaTableView::currentRowInfo() const
{
    QList<QVariant> list;
    QModelIndex idx = this->currentIndex();
    if( !idx.isValid() ) return list;
    int r = idx.row();
    QSqlRecord record = m_sqlModel->record(r);  //get the data in the current row
    for( int i = LAYERNAME; i < record.count(); ++i)//read data starting from LayerName
    {
        QSqlField field = record.field(i);
        QString data = field.value().toString();
        list.append(data);
    }

    return list;
}

/**
 * get the info of a given row
 * @brief BonzaTableView::getRowInfo
 * @return
 */
QList<QVariant> BonzaTableView::getRowInfo(int r) const
{
    QList<QVariant> list;

    QSqlRecord record = m_sqlModel->record(r);  //get the data in the current row
    for( int i = LAYERNAME; i < record.count(); ++i)//read data starting from LayerName
    {
        QSqlField field = record.field(i);
        QString data = field.value().toString();
        list.append(data);
    }

    return list;
}

/**
 * update the FEM cell
 * @brief BonzaTableView::updateFEMCell
 * @return
 */
void BonzaTableView::updateFEMCell(const QList<QVariant> &valueList)
{
    qDebug() << "updateFEMCell called. ";
}

void BonzaTableView::insertAt(const QList<QVariant> &valueList, int insertPosition)
{

    if(totalSize() < (MAXLAYERS))
    {
    // select all
    m_sqlModel->submit();
    m_sqlModel->setFilter(QString(""));
    m_sqlModel->select();


    //int insertPosition = 1;
    this->model()->insertRow(insertPosition);// actually added to the end of the table, stupid qt.
    m_sqlModel->submitAll();

    // really put the data on the inserted row
    m_sqlModel->setData(m_sqlModel->index(insertPosition, CHECKED), 0);
    m_sqlModel->setData(m_sqlModel->index(insertPosition, LAYERNAME), "Layer "+QString::number(insertPosition+1));
    m_sqlModel->setData(m_sqlModel->index(insertPosition, THICKNESS), "3");


    m_nTotal++;
    updateTableModel();


    for( int i = m_nTotal-1; i > -1+insertPosition; --i)
    {
        m_sqlModel->setData(m_sqlModel->index(i,0),QString::number(i+2),Qt::EditRole);
    }
    m_sqlModel->setData(m_sqlModel->index(m_nTotal-1,0),QString::number(insertPosition+1),Qt::EditRole);

    // 

    if (m_sqlModel->record(insertPosition).value("LAYERNAME")=="")
    {
        m_sqlModel->setData(m_sqlModel->index(insertPosition, CHECKED), 0);
        m_sqlModel->setData(m_sqlModel->index(insertPosition, LAYERNAME), "Layer "+QString::number(insertPosition+1));
        m_sqlModel->setData(m_sqlModel->index(insertPosition, THICKNESS), "3");
    }


    // really put the data on the inserted row
    //m_sqlModel->setData(m_sqlModel->index(insertPosition, CHECKED), "0");
    //m_sqlModel->setData(m_sqlModel->index(insertPosition, LAYERNAME), "Layer "+QString::number(insertPosition+1));
    //m_sqlModel->setData(m_sqlModel->index(insertPosition, COLOR), "red");
    int col = CHECKED;
    for( int i = 0; i < valueList.count(); ++i)
    {
        col++;
        QString data = valueList.at(i).toString();
        m_sqlModel->setData(m_sqlModel->index(insertPosition, col), data);
    }
    m_sqlModel->setData(m_sqlModel->index(insertPosition, COLOR), QColor::fromRgb(QRandomGenerator::global()->generate()).name());

    /*
    if (valueList.length()>THICKNESS)
        m_ntotalHeight += valueList.at(THICKNESS-1).toDouble();
    */

    m_sqlModel->submitAll();

    m_sqlModel->select();

    m_nCurPage = qCeil(double(insertPosition+1) / double(m_nPageSize));
    gotoPage(m_nCurPage);


    m_sqlModel->setActive(insertPosition);
    setCurrentIndex(m_sqlModel->index(insertPosition, LAYERNAME));

    // print table for debug
    for( int i = m_nTotal-1; i > -1; --i)
    {
        QSqlRecord myrecord = m_sqlModel->record(i);
        QString id = myrecord.value("id").toString();
        QString name = myrecord.value("LAYERNAME").toString();
        QString color = myrecord.value("COLOR").toString();
        QString checked = myrecord.value("CHECKED").toString();
        QString thickness = myrecord.value("THICKNESS").toString();
        qDebug() <<"ID: " <<id << "; name: " << name << " ; color:" << color << " ; checked: " << checked << " ;thickness " << thickness;
    }


    }
}

void BonzaTableView::insertAbove(const QList<QVariant> &valueList)
{


    QModelIndexList idxList = this->selectionModel()->selectedIndexes();

    QMap<int, int> rowMap;
    foreach (QModelIndex index, idxList)
    {
        rowMap.insert(index.row(), 0);
        qDebug() << index.row();
    }

    qDebug() << rowMap.size();
    int insertPosition;
    QMapIterator<int, int> rowMapIterator(rowMap);
    rowMapIterator.toBack();
    if (rowMapIterator.hasPrevious())
    {
        while (rowMapIterator.hasPrevious())
        {
            rowMapIterator.previous();
            int rowDelOnCurrentPage = rowMapIterator.key();
            insertPosition = rowDelOnCurrentPage + m_nPageSize * ( m_nCurPage - 1);
            insertAt(valueList, insertPosition);
        }
    } else
    {
        if (m_nTotal<1)
            insertAt(valueList, 0);
        else
            insertAt(valueList, (m_nCurPage-1) * m_nPageSize + m_nCurPageSize);
    }

}

void BonzaTableView::insertBelow(const QList<QVariant> &valueList)
{

    QModelIndexList idxList = this->selectionModel()->selectedIndexes();

    QMap<int, int> rowMap;
    foreach (QModelIndex index, idxList)
    {
        rowMap.insert(index.row(), 0);
    }

    qDebug() << rowMap.size();

    int insertPosition;
    QMapIterator<int, int> rowMapIterator(rowMap);
    rowMapIterator.toBack();
    if (rowMapIterator.hasPrevious())
    {
        while (rowMapIterator.hasPrevious())
        {
            rowMapIterator.previous();
            int rowDelOnCurrentPage = rowMapIterator.key();
            insertPosition = rowDelOnCurrentPage + m_nPageSize * ( m_nCurPage - 1);
            insertAt(valueList, insertPosition+1);
        }
    }else{
        insertAt(valueList, m_nTotal);
    }

}

void BonzaTableView::insert(const QList<QVariant> &valueList)
{
    // select all
    m_sqlModel->submit();
    m_sqlModel->setFilter(QString(""));
    m_sqlModel->select();


    int insertPosition = 1;
    this->model()->insertRow(insertPosition);// actually added to the end of the table, stupid qt.

    // really put the data into the inserted row
    int col = CHECKED;
    for( int i = 0; i < valueList.count(); ++i)
    {
        col++;
        QString data = valueList.at(i).toString();
        m_sqlModel->setData(m_sqlModel->index(insertPosition, col), data);
    }

    m_nTotal++;
    updateTableModel();

    // manually update the primary key of the table
    for( int i = m_nTotal-1; i > -1+insertPosition; --i)
    {
        m_sqlModel->setData(m_sqlModel->index(i,0),QString::number(i+2),Qt::EditRole);
    }
    m_sqlModel->setData(m_sqlModel->index(m_nTotal-1,0),QString::number(insertPosition+1),Qt::EditRole);
    m_sqlModel->submit();
    m_sqlModel->select();

    // print table for debug
    for( int i = m_nTotal-1; i > -1; --i)
    {
        QSqlRecord myrecord = m_sqlModel->record(i);
        QString id = myrecord.value("id").toString();
        QString name = myrecord.value("LAYERNAME").toString();
        qDebug() <<"TableView ID: " <<id << "; name: " << name ;
    }


}

void BonzaTableView::insertAtEnd(const QList<QVariant> &valueList)
{
    int rowNum = m_sqlModel->rowCount();
    if( lastPageSize() >= m_nPageSize )
    {
        m_nTotalPage++;
    }
    m_nCurPage = m_nTotalPage;

    m_sqlModel->insertRow(rowNum);

    if (m_sqlModel->record(rowNum).value("LAYERNAME")=="")
    {
        m_sqlModel->setData(m_sqlModel->index(rowNum, CHECKED), 0);
        m_sqlModel->setData(m_sqlModel->index(rowNum, LAYERNAME), "Layer "+QString::number(rowNum+1));
        m_sqlModel->setData(m_sqlModel->index(rowNum, THICKNESS), "3");
    }

    int col = CHECKED;
    m_sqlModel->setData(m_sqlModel->index(rowNum, col), 0);
    for( int i = 0; i < valueList.count(); ++i)
    {
        col++;
        QString data = valueList.at(i).toString();
        m_sqlModel->setData(m_sqlModel->index(rowNum, col), data);
    }
    m_sqlModel->setData(m_sqlModel->index(rowNum, COLOR), QColor::fromRgb(QRandomGenerator::global()->generate()).name());

    m_nTotal++;

    m_sqlModel->submitAll();
    updateModel();
    qDebug() << m_sqlModel->rowCount();
    QModelIndex sel = this->model()->index(m_sqlModel->rowCount(), 1);
    this->setCurrentIndex(sel);
}

void BonzaTableView::removeOneRow(int rowDel)
{
    // select all
    m_sqlModel->submit();
    m_sqlModel->setFilter(QString(""));
    m_sqlModel->select();

    // debuging
    for( int i = 0; i < m_nTotal; ++i)
    {
        QSqlRecord myrecord = m_sqlModel->record(i);
        QString id = myrecord.value("id").toString();
        QString name = myrecord.value("LAYERNAME").toString();
        qDebug() <<"ID: " <<id << "; name: " << name ;
    }


    this->model()->removeRow(rowDel);
    m_nTotal--;
    m_sqlModel->submitAll();
    m_sqlModel->select();

    // debuging
    for( int i = 0; i < m_nTotal; ++i)
    {
        QSqlRecord myrecord = m_sqlModel->record(i);
        QString id = myrecord.value("id").toString();
        QString name = myrecord.value("LAYERNAME").toString();
        qDebug() <<"ID: " <<id << "; name: " << name ;
    }

    qDebug()<<m_sqlModel->index(0,0).row();
    // manually update the primary key of the table
    for( int i = rowDel; i < m_nTotal; ++i)
    {
        qDebug() <<"old ID: " << m_sqlModel->record(i).value("id").toString() << "; new ID: " << i+1 ;
        m_sqlModel->setData(m_sqlModel->index(i,0),QString::number(i+1),Qt::EditRole);
    }
    m_sqlModel->submitAll();
    m_sqlModel->select();

    // debuging
    for( int i = 0; i < m_nTotal; ++i)
    {
        QSqlRecord myrecord = m_sqlModel->record(i);
        QString id = myrecord.value("id").toString();
        QString name = myrecord.value("LAYERNAME").toString();
        qDebug() <<"ID: " <<id << "; name: " << name ;
    }



    int currentRow = qMin(int(rowDel+1),int(m_nTotal));
    m_nCurPage = qCeil(double(currentRow) / double(m_nPageSize));
    m_nLastPageSize = rowDel % m_nPageSize;
    gotoPage(m_nCurPage);

    updateModel();

    emit rowRemoved(rowDel);

    m_sqlModel->setActive(rowDel);
    setCurrentIndex(m_sqlModel->index(rowDel, LAYERNAME));

    // debuging
    for( int i = 0; i < m_nTotal; ++i)
    {
        QSqlRecord myrecord = m_sqlModel->record(i);
        QString id = myrecord.value("id").toString();
        QString name = myrecord.value("LAYERNAME").toString();
        qDebug() <<"ID: " <<id << "; name: " << name ;
    }

}

void BonzaTableView::remove()
{

    // debuging
    for( int i = 0; i < m_nTotal; ++i)
    {
        QSqlRecord myrecord = m_sqlModel->record(i);
        QString id = myrecord.value("id").toString();
        QString name = myrecord.value("LAYERNAME").toString();
        qDebug() <<"ID: " <<id << "; name: " << name ;
    }


    QModelIndexList idxList = this->selectionModel()->selectedIndexes();

    QMap<int, int> rowMap;
    foreach (QModelIndex index, idxList)
    {
        qDebug()<<index.row();
        rowMap.insert(index.row(), 0);
    }

    int rowDel;
    QMapIterator<int, int> rowMapIterator(rowMap);
    rowMapIterator.toBack();
    if (rowMapIterator.hasPrevious())
    {
        while (rowMapIterator.hasPrevious())
        {
            rowMapIterator.previous();
            int rowDelOnCurrentPage = rowMapIterator.key();
            rowDel = rowDelOnCurrentPage + m_nPageSize * ( m_nCurPage - 1);
            //this->model()->removeRow(rowDel);
            removeOneRow(rowDel);
        }
    }else{
        if (m_nTotal>0)
            removeOneRow(m_nTotal-1);
    }

    /*
    if( lastPageSize() <= 1 )
    {
        m_nLastPageSize = m_nPageSize;
        m_nTotalPage--;
        m_nCurPage--;
    }
    m_nTotal--;
    m_sqlModel->submitAll();

    updateModel();

    // debuging
    for( int i = 0; i < m_nTotal; ++i)
    {
        QSqlRecord myrecord = m_sqlModel->record(i);
        QString id = myrecord.value("id").toString();
        QString name = myrecord.value("LAYERNAME").toString();
        qDebug() <<"ID: " <<id << "; name: " << name ;
    }
    */

}



void BonzaTableView::removeOnPage()
{


    QModelIndexList idxList = this->selectionModel()->selectedIndexes();

    QMap<int, int> rowMap;
    foreach (QModelIndex index, idxList)
        rowMap.insert(index.row(), 0);

    int rowDel;
    QMapIterator<int, int> rowMapIterator(rowMap);
    rowMapIterator.toBack();
    while (rowMapIterator.hasPrevious())
    {
        rowMapIterator.previous();
        rowDel = rowMapIterator.key();
        this->model()->removeRow(rowDel);
    }
    if( lastPageSize() <= 1 )
    {
        m_nLastPageSize = m_nPageSize;
        m_nTotalPage--;
        m_nCurPage--;
    }
    m_nTotal--;
    m_sqlModel->submitAll();

    updateModel();


}

void BonzaTableView::previousPage()
{
    if(m_nStartId <= 0 || m_nTotal <= 0)
        return;

    m_nCurPage--;
    this->updateModel();
}

void BonzaTableView::nextPage()
{
    if( m_nCurPage >= m_nTotalPage || m_nTotal <= 0)
        return;

    m_nCurPage++;
    this->updateModel();
}

/**
 * jump to the 'index' page
 * @brief BonzaTableView::gotoPage
 * @param index
 */
void BonzaTableView::gotoPage(int index)
{
    //if(  index <= 0 || index > m_nTotalPage || m_nTotalPage <= 0 || index == m_nCurPage)
    //    return;

    m_nCurPage = index;
    this->updateModel();
}

/**
 * num of total pages
 * @brief BonzaTableView::totalPage
 * @return
 */
int BonzaTableView::totalPage()
{
    m_nTotalPage = m_nTotal / m_nPageSize;
    m_nTotalPage = (m_nTotal % m_nPageSize) > 0 ? (m_nTotalPage + 1) : m_nTotalPage;

    return m_nTotalPage;
}

/**
 * refresh the data on every page
 * @brief BonzaTableView::updateModel
 */
void BonzaTableView::updateModel()
{

    m_sqlModel->submitAll();

    //m_sqlModel->setSort(BOTOMPOS,Qt::AscendingOrder);

    m_nStartId = (m_nCurPage-1) * m_nPageSize;
    QString strFilter = QString(" 1=1 limit %1,%2").arg(m_nStartId).arg(m_nPageSize);
    //QString strFilter = QString("LayerName > '1' AND LayerName < '5'");
    //QString strFilter = QString("");
    m_sqlModel->setFilter(strFilter);
    m_sqlModel->select();

    m_nCurPageSize = m_sqlModel->rowCount();



}

/**
 * refresh the data on the whole table
 * @brief BonzaTableView::updateModel
 */
void BonzaTableView::updateTableModel()
{
    m_sqlModel->submit();
    QString strFilter = QString("");
    m_sqlModel->setFilter(strFilter);
    m_sqlModel->select();
}

void BonzaTableView::styleView(bool enabled)
{
    if( enabled )
    {
        //this->setItemDelegate(new TableViewItemDelegate(this));
        this->setItemDelegateForColumn(MATERIAL,new MatItemDelegate(this));
        this->setItemDelegateForColumn(LAYERNAME,new TableViewItemDelegate(this));
        this->setItemDelegateForColumn(THICKNESS,new TableViewItemDelegate(this));
        this->setItemDelegateForColumn(DENSITY,new TableViewItemDelegate(this));
        this->setItemDelegateForColumn(VS,new TableViewItemDelegate(this));
        this->setItemDelegateForColumn(COLOR,new TableViewItemDelegate(this));
    }
    else
        this->setItemDelegate(new QItemDelegate(this));
}

/**
 * num of layers on the last page
 * @brief BonzaTableView::lastPageSize
 * @return
 */
int BonzaTableView::lastPageSize()
{
    m_nLastPageSize = (m_nTotal % m_nPageSize) == 0 ? m_nPageSize : (m_nTotal % m_nPageSize);

    return m_nLastPageSize;
}

void BonzaTableView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu* popMenu = new QMenu(this);
    QAction *insertAction = popMenu->addAction( tr("&Insert  ⤴") );
    connect( insertAction, SIGNAL(triggered()), this, SIGNAL(insertAct()) );

    QAction *removeAction = popMenu->addAction( tr("&Delete ×") );
    removeAction->setEnabled(false);

    QModelIndex idx = this->indexAt( mapFromGlobal( QPoint( event->globalPos().x(), \
            event->globalPos().y()-this->horizontalHeader()->height())) );
    if( idx.isValid() )
    {
        removeAction->setEnabled(true);
        connect( removeAction, SIGNAL(triggered()), this, SIGNAL(removeAct()) );
    }
    else
    {
        this->clearSelection();
        this->selectionModel()->clearSelection();
    }

    QAction *insertBelowAction = popMenu->addAction( tr("&Insert  ⤵") );
    insertBelowAction->setEnabled(false);

    QModelIndex idx2 = this->indexAt( mapFromGlobal( QPoint( event->globalPos().x(), \
            event->globalPos().y()-this->horizontalHeader()->height())) );
    if( idx2.isValid() )
    {
        insertBelowAction->setEnabled(true);
        connect( insertBelowAction, SIGNAL(triggered()), this, SIGNAL(insertBelowAct()) );
    }
    else
    {
        this->clearSelection();
        this->selectionModel()->clearSelection();
    }

    popMenu->exec(event->globalPos());
    event->accept();
}



/**
 * divide the total evenly by num of layers
 * @brief BonzaTableView::lastPageSize
 * @return
 */
void BonzaTableView::divideByLayers(double previousHeight, int numLayers)
{
    double layerthickness = previousHeight / numLayers;// totalHeight() / m_nTotal;
    for (int i=0;i<m_nTotal;i++)
        m_sqlModel->setData(m_sqlModel->index(i, THICKNESS), QString::number(layerthickness));

}


double BonzaTableView::getGWT()
{
    return m_nGWT;
}

void BonzaTableView::setGWT(double GWT)
{
    m_nGWT = GWT;
    emit gwtChanged(m_nGWT);
}
