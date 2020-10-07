#include "BonzaTableView.h"
#include <QHeaderView>
#include <QFocusEvent>
#include <QMenu>
#include <QAction>
#include <QAbstractItemView>
#include <QtMath>



BonzaTableView::BonzaTableView(QWidget *parent) :
    QTableView(parent),
    m_nStartId(0), m_nPageSize(PAGESIZE), m_nCurPageSize(0), m_nTotal(0),
    m_nCurPage(1), m_nTotalPage(0), m_ntotalHeight(10), m_nGWT(0)
{




    // initialize view
    m_sqlModel = new BonzaTableModel(nullptr);
    m_sqlModel->moveToThread(&m_thread);
    m_thread.start();

    //m_sqlModel->setEditStrategy(QSqlTableModel::OnFieldChange);

    m_sqlModel->setHeaderData(LayerID, Qt::Horizontal, tr("ID"));
    m_sqlModel->setHeaderData(CHECKED, Qt::Horizontal, tr("√"));
    m_sqlModel->setHeaderData(LAYERNAME, Qt::Horizontal, tr("LayerName"));
    m_sqlModel->setHeaderData(THICKNESS, Qt::Horizontal, tr("Thickness"));
    m_sqlModel->setHeaderData(DENSITY, Qt::Horizontal, tr("Density"));
    m_sqlModel->setHeaderData(VS, Qt::Horizontal, tr("Vs"));
    m_sqlModel->setHeaderData(MATERIAL, Qt::Horizontal, tr("Material"));
    m_sqlModel->setHeaderData(ESIZE, Qt::Horizontal, tr("ElementSize"));
    m_sqlModel->setHeaderData(COLOR, Qt::Horizontal, tr("Color"));





    m_nTotal = m_sqlModel->rowCount();
    m_nTotalPage = totalPage();
    m_nLastPageSize = lastPageSize();
    this->updateModel();    //update the table on each page
    this->setModel(m_sqlModel);
    this->hideColumn(0);
    this->hideColumn(FEM);
    this->hideColumn(COLOR);
    this->verticalHeader()->hide();

    this->setColumnHidden(1,true);


    //this->setItemDelegate(new TableViewItemDelegate(this));
    this->setItemDelegateForColumn(MATERIAL,new MatItemDelegate(this));
    this->setItemDelegateForColumn(LAYERNAME,new TableViewItemDelegate(this));
    this->setItemDelegateForColumn(THICKNESS,new TableViewItemDelegate(this));
    this->setItemDelegateForColumn(DENSITY,new TableViewItemDelegate(this));
    this->setItemDelegateForColumn(VS,new TableViewItemDelegate(this));
    this->setItemDelegateForColumn(COLOR,new TableViewItemDelegate(this));
    this->setItemDelegateForColumn(ESIZE,new TableViewItemDelegate(this));




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
    //m_sqlModel->submitAll();//commentednotsure
    if(m_thread.isRunning())
    {
        m_thread.quit();
        m_thread.wait();
    }
    //m_sqlModel->clear();//commentednotsure
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

    int ir = index.row();
    int ic = index.column();
    int numRows = m_sqlModel->rowCount();
    // if this row is alrady selected, regard the current single click as a double click
    bool cellDoubleClicked = m_sqlModel->isActive(ir);

    //qDebug() << "view says: I feel row " << index.row() << " column " << index.column() << " activated" ;

    if(index.column()==MATERIAL)
        qDebug() << "material clicked";

    setActive(index.row(), index.column());

    // double click simulator

    if ( ir==(numRows-1) && (ic==LAYERNAME || ic==THICKNESS || ic==MATERIAL || ic==ESIZE))
        qDebug() << "Rock layer clicked, do nothing.";
    else
    {

        if (cellDoubleClicked)
            this->edit(m_sqlModel->index(ir, ic));

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

    QModelIndexList idxList = this->selectionModel()->selectedIndexes();

    foreach (QModelIndex index, idxList)
    {
        if(index.column()>=LAYERNAME)
        {
            QString d = m_sqlModel->data(index).toString();
            list.append(d);
        }
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

    for( int i = LAYERNAME; i <= LASTCOL; ++i)//read data starting from LayerName
    {
        QString d = m_sqlModel->data(m_sqlModel->index(r, i)).toString();
        list.append(d);
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


        if(insertPosition==m_nTotal & m_nTotal>0)
            insertPosition -=1;

        //int insertPosition = 1;
        this->model()->insertRows(insertPosition,1);

        QList<QVariant> rowList;
        QString defaultID = QString::number(insertPosition);
        QString defaultLayerName = "Layer " + QString::number(insertPosition+1);
        QString defaultesize = "-";
        if (m_nTotal > 0) defaultesize = QString::number(DefaultESize);
        rowList << defaultID << "0" << defaultLayerName << "3" << DefaultDensity << DefaultVs << DefaultEType << defaultesize << "blue";
        int col = CHECKED;
        for( int i = 0; i < valueList.count(); ++i)
        {
            col++;
            rowList[col] = valueList.at(i).toString();
        }

        for( int i = 0; i < rowList.count(); ++i)
        {
            QString data = rowList.at(i).toString();
            m_sqlModel->setData(m_sqlModel->index(insertPosition, i), data);
        }

        if (valueList.count()!=7)
            m_sqlModel->setData(m_sqlModel->index(insertPosition, COLOR), QColor::fromRgb(QRandomGenerator::global()->generate()).name());
        if(m_nTotal<1)//Rock
            m_sqlModel->setData(m_sqlModel->index(m_nTotal, COLOR), QString("Black"));

        /*
        if(m_nTotal>=1)
        {
            int col = CHECKED;
            int lsize = valueList.count();
            for( int i = 0; i < lsize; ++i)
            {
                col++;
                QString data = valueList.at(i).toString();
                m_sqlModel->setData(m_sqlModel->index(insertPosition, col), data);
            }
            if (valueList.count()!=7)
                m_sqlModel->setData(m_sqlModel->index(insertPosition, COLOR), QColor::fromRgb(QRandomGenerator::global()->generate()).name());
        }else{//Rock
            //QList<QVariant> valueListRock;
            //valueListRock << "Rock" << "-" << DefaultDensity << DefaultVs << DefaultEType << "-";
            int col = CHECKED;
            for( int i = 0; i < valueList.count(); ++i)
            {
                col++;
                QString data = valueList.at(i).toString();
                m_sqlModel->setData(m_sqlModel->index(m_nTotal, col), data);
            }
            //m_sqlModel->setData(m_sqlModel->index(m_nTotal, COLOR), QColor::fromRgb(QRandomGenerator::global()->generate()).name());
            m_sqlModel->setData(m_sqlModel->index(m_nTotal, COLOR), QString("Black"));
        }
        */


        // reordering the IDs
        /*
        bool b;
        for (int j = m_nTotal; j>=insertPosition; j--)
        {
            b = m_sqlModel->setData(m_sqlModel->index(j,0),QString::number(j+2),Qt::EditRole);

        }
        b = m_sqlModel->setData(m_sqlModel->index(m_nTotal,0),QString::number(insertPosition+1),Qt::EditRole);
        */

        m_nTotal++;

        m_nCurPage = qCeil(double(insertPosition+1) / double(m_nPageSize));
        gotoPage(m_nCurPage);

        m_sqlModel->setActive(insertPosition);
        setCurrentIndex(m_sqlModel->index(insertPosition, LAYERNAME));

    }

}


void BonzaTableView::insertAtSilent(const QList<QVariant> &valueList, int insertPosition)
{

    if(totalSize() < (MAXLAYERS))
    {

        if(insertPosition==m_nTotal & m_nTotal>0)
            insertPosition -=1;

        this->model()->insertRow(m_nTotal);// actually added to the end of the table, stupid qt.


        // really put the data on the inserted row
        m_sqlModel->setDataSilent(m_sqlModel->index(m_nTotal, CHECKED), 0);
        m_sqlModel->setDataSilent(m_sqlModel->index(m_nTotal, LAYERNAME), "Layer "+QString::number(insertPosition+1));
        m_sqlModel->setDataSilent(m_sqlModel->index(m_nTotal, THICKNESS), "3");

        if(m_nTotal>=1)
        {
            int col = CHECKED;
            for( int i = 0; i < valueList.count(); ++i)
            {
                col++;
                QString data = valueList.at(i).toString();
                m_sqlModel->setDataSilent(m_sqlModel->index(m_nTotal, col), data);
            }
            //m_sqlModel->setDataSilent(m_sqlModel->index(m_nTotal, COLOR), QColor::fromRgb(QRandomGenerator::global()->generate()).name());
        }else{//Rock
            //QList<QVariant> valueListRock;
            //valueListRock << "Rock" << "-" << DefaultDensity << DefaultVs << DefaultEType << "-";
            int col = CHECKED;
            for( int i = 0; i < valueList.count(); ++i)
            {
                col++;
                QString data = valueList.at(i).toString();
                m_sqlModel->setDataSilent(m_sqlModel->index(m_nTotal, col), data);
            }
            //m_sqlModel->setData(m_sqlModel->index(m_nTotal, COLOR), QColor::fromRgb(QRandomGenerator::global()->generate()).name());
            m_sqlModel->setDataSilent(m_sqlModel->index(m_nTotal, COLOR), "Black");
        }


        bool b;
        for (int j = m_nTotal; j>=insertPosition; j--)
        {
            b = m_sqlModel->setDataSilent(m_sqlModel->index(j,0),QString::number(j+2),Qt::EditRole);
        }
        b = m_sqlModel->setDataSilent(m_sqlModel->index(m_nTotal,0),QString::number(insertPosition+1),Qt::EditRole);

        m_nTotal++;

        m_nCurPage = qCeil(double(insertPosition+1) / double(m_nPageSize));
        gotoPage(m_nCurPage);

        m_sqlModel->setActive(insertPosition);
        setCurrentIndex(m_sqlModel->index(insertPosition, LAYERNAME));

    }
}

void BonzaTableView::insertAbove(const QList<QVariant> &valueList)
{


    QModelIndexList idxList = this->selectionModel()->selectedIndexes();

    QMap<int, int> rowMap;
    foreach (QModelIndex index, idxList)
    {
        rowMap.insert(index.row(), 0);
    }

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

    if (m_sqlModel->data(m_sqlModel->index(rowNum,LAYERNAME)).toString()=="")
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

    m_sqlModel->submit();
    updateModel();

    QModelIndex sel = this->model()->index(m_sqlModel->rowCount(), 1);
    this->setCurrentIndex(sel);
}

void BonzaTableView::removeOneRow(int rowDel)
{


    this->model()->removeRow(rowDel);
    m_nTotal--;
    m_sqlModel->submit();

    // manually update the primary key of the table
    for( int i = rowDel; i < m_nTotal; ++i)
    {
        m_sqlModel->setData(m_sqlModel->index(i,0),QString::number(i+1),Qt::EditRole);
    }
    m_sqlModel->submit();


    int currentRow = qMin(int(rowDel+1),int(m_nTotal));
    m_nCurPage = qCeil(double(currentRow) / double(m_nPageSize));
    m_nLastPageSize = rowDel % m_nPageSize;
    gotoPage(m_nCurPage);

    updateModel();

    emit rowRemoved(rowDel);

    m_sqlModel->setActive(rowDel);
    setCurrentIndex(m_sqlModel->index(rowDel, LAYERNAME));

}

void BonzaTableView::remove()
{



    QModelIndexList idxList = this->selectionModel()->selectedIndexes();

    QMap<int, int> rowMap;
    foreach (QModelIndex index, idxList)
    {
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
            if((rowDel+1)==m_nTotal)
                qDebug() <<"Rock layer can not be removed.";
            else
                removeOneRow(rowDel);
        }
    }else{
        qDebug() <<"Rock layer can not be removed.";

    }


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
    m_sqlModel->submit();

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

    m_sqlModel->submit();
    m_nStartId = (m_nCurPage-1) * m_nPageSize;
    QString strFilter = QString(" 1=1 limit %1,%2").arg(m_nStartId).arg(m_nPageSize);
    m_nCurPageSize = m_sqlModel->rowCount();
}

/**
 * refresh the data on the whole table
 * @brief BonzaTableView::updateModel
 */
void BonzaTableView::updateTableModel()
{
    m_sqlModel->submit();
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
    QAction *insertAction = popMenu->addAction( tr("&Insert  ") + QChar(0x2934) );
    connect( insertAction, SIGNAL(triggered()), this, SIGNAL(insertAct()) );

    QAction *removeAction = popMenu->addAction( tr("&Delete ") + QChar(0x2717) );
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

    QAction *insertBelowAction = popMenu->addAction( tr("&Insert  ") + QChar(0x2935) );
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
