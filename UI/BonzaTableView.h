#ifndef BONZATABLEVIEW_H
#define BONZATABLEVIEW_H
#include "GlobalConstances.h"
#include "BonzaTableModel.h"

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



class BonzaTableView : public QTableView
{
    Q_OBJECT
    //Q_PROPERTY(double m_nGWT READ getGWT WRITE setGWT NOTIFY gwtChanged)
    Q_PROPERTY(double m_nGWT MEMBER m_nGWT NOTIFY gwtChanged)


public:
    explicit BonzaTableView(QWidget *parent = nullptr);
    ~BonzaTableView();

    // hidden data can not be obtained from current tableview's model
    // I'm getting it from database's model
    QSqlTableModel* sqlModel() { return m_sqlModel; }

    QList<QVariant> currentRowInfo() const;
    QList<QVariant> getRowInfo(int r) const;
    void updateFEMCell(const QList<QVariant> &valueList);
    int currentPage() const { return m_nCurPage; }
    int totalSize() const { return m_nTotal; }
    int lastPageSize() const { return m_nLastPageSize; }
    double totalHeight() const {return m_sqlModel->getTotalHeight();}

    void setTotalHeight(double h) {m_ntotalHeight = h;}



    BonzaTableModel* m_sqlModel;

    QVector<QMap<QString,double>> m_LayerVector;

    Q_INVOKABLE QMap<QString,double> getLayerVector(int layerNum){return m_LayerVector[layerNum];}

    QVector<QString> layerParsName;
    QVector<double> layerParsValue;
    Q_INVOKABLE QString getLayerParsNames(int index){return layerParsName[index];}
    Q_INVOKABLE double getLayerParsValues(int index){return layerParsValue[index];}


    Q_INVOKABLE double getGWT();

    Q_INVOKABLE void setActive(int row)
    {
        m_sqlModel->setActive(row);// notify the model
        setCurrentIndex(m_sqlModel->index(row, LAYERNAME));
        //setCurrentIndex(m_sqlModel->index(row, THICKNESS));
    }
    Q_INVOKABLE void setActive(int row, int column)
    {
        m_sqlModel->setActive(row);// notify the model
        setCurrentIndex(m_sqlModel->index(row, column));
    }
    Q_INVOKABLE void setActive(const QModelIndex &index)
    {
        m_sqlModel->setActive(index.row());// notify the model
        setCurrentIndex(index);
    }


signals:
    void insertAct();
    void removeAct();
    void insertBelowAct();
    void gwtChanged(double newGWT);
    void rowRemoved(int row);
    void cellClicked(const QModelIndex &index);

public slots:
    void insert(const QList<QVariant> &valueList);
    void insertAtEnd(const QList<QVariant> &valueList);
    void insertBelow(const QList<QVariant> &valueList);
    void insertAbove(const QList<QVariant> &valueList);
    void insertAt(const QList<QVariant> &valueList, int row);
    void remove();
    void removeOneRow(int row);
    void removeOnPage();



    void divideByLayers(double, int);


    void previousPage();
    void nextPage();
    void gotoPage(int index);
    int  totalPage();
    void updateModel();
    void updateTableModel();

    void styleView(bool);

    void setGWT(double GWT);

    void onCellSingleClicked(const QModelIndex &index);

    void onCellDoubleClicked(const QModelIndex &index)
    {
        Q_UNUSED(index);
        qDebug() << "Double click is not working. \n"
                 << "Because the model is repopulated, and on OnManualSubmit mode the selection in the view is losted. \n"
                 <<  "The double click simulator in cellSingleClicked is an alternative. ";
    }



protected:
    void contextMenuEvent(QContextMenuEvent *event);

private:
    int lastPageSize();

private:

    QThread m_thread;

    int m_nStartId;
    int m_nPageSize;
    int m_nCurPageSize;
    int m_nLastPageSize;
    int m_nTotal;

    int m_nCurPage;
    int m_nTotalPage;

    double m_ntotalHeight;
    double m_nGWT = 3.0;

};



/**
 * A class used to customize the appearance of materal cells.
 * @brief TableViewItemDelegate::TableViewItemDelegate
 * @return
 */
class MatItemDelegate: public QItemDelegate
{
    Q_OBJECT
public:
    explicit MatItemDelegate(QWidget* parent):QItemDelegate(parent){}

    QWidget * createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QComboBox *editor = new QComboBox(parent);
        editor->addItem("Elastic");
        editor->addItem("PM4Sand");
        return editor;
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) const
    {
        QComboBox* cb = qobject_cast<QComboBox*>(editor);
        Q_ASSERT(cb);
        // get the index of the text in the combobox that matches the current value of the itenm
        QString currentText = index.data(Qt::EditRole).toString();
        int cbIndex = cb->findText(currentText);
        // if it is valid, adjust the combobox
        if (cbIndex >= 0)
           cb->setCurrentIndex(cbIndex);
    }


    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
    {
        QComboBox* cb = qobject_cast<QComboBox*>(editor);
        Q_ASSERT(cb);
        model->setData(index, cb->currentText(), Qt::EditRole);
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
                           const QModelIndex &index ) const
    {
        // get the text for index
        QString text = (index.data(Qt::DisplayRole)).toString();
        QStyleOptionViewItem opt = setOptions(index, option);

        qobject_cast<BonzaTableView*>(parent())->horizontalHeader()->resizeSection(CHECKED, 20);

        painter->save();
        QPen pen;

        if( index.column() != CHECKED )
        {
            painter->setPen(Qt::NoPen);
            painter->setRenderHint(QPainter::Antialiasing, true);
            // set the color for active, it seems didn't work
            if( opt.state & QStyle::State_Selected )
            {
                painter->setBrush(option.palette.highlightedText());
                painter->fillRect(opt.rect, QBrush(QColor(0, 0, 0)));
//                QItemDelegate::paint(painter, option, index);
            }
            // draw the background color and text color
            painter->setPen(pen);
            drawBackground(painter, opt, index);

            //painter->drawText(opt.rect, opt.displayAlignment, text);
            painter->drawText(opt.rect, Qt::AlignHCenter | Qt::AlignVCenter, text);
            painter->restore();
        }

    }


};


/**
 * A class used to customize the appearance of the
 * table view.
 * @brief TableViewItemDelegate::TableViewItemDelegate
 * @return
 */
class TableViewItemDelegate: public QItemDelegate
{
    Q_OBJECT
public:
    explicit TableViewItemDelegate(QWidget* parent):QItemDelegate(parent){}




    void paint(QPainter *painter, const QStyleOptionViewItem &option,
                           const QModelIndex &index ) const
    {
        // get the text for index
        QString text = (index.data(Qt::DisplayRole)).toString();
        QStyleOptionViewItem opt = setOptions(index, option);

        qobject_cast<BonzaTableView*>(parent())->horizontalHeader()->resizeSection(CHECKED, 20);

        painter->save();
        QPen pen;

        if( index.column() != CHECKED )
        {
            painter->setPen(Qt::NoPen);
            painter->setRenderHint(QPainter::Antialiasing, true);
            // set the color for active, it seems didn't work
            if( opt.state & QStyle::State_Selected )
            {
                painter->setBrush(option.palette.highlightedText());
                painter->fillRect(opt.rect, QBrush(QColor(0, 0, 0)));
//                QItemDelegate::paint(painter, option, index);
            }
            // draw the background color and text color
            painter->setPen(pen);
            drawBackground(painter, opt, index);

            //painter->drawText(opt.rect, opt.displayAlignment, text);
            painter->drawText(opt.rect, Qt::AlignHCenter | Qt::AlignVCenter, text);
            painter->restore();
        }



    }


    void paintMore(QPainter *painter, const QStyleOptionViewItem &option,
                           const QModelIndex &index ) const
    {
        // get the text for index
        QString text = (index.data(Qt::DisplayRole)).toString();
        QStyleOptionViewItem opt = setOptions(index, option);
        // opt.displayAlignment = Qt::AlignHCenter | Qt::AlignVCenter;;

        qobject_cast<BonzaTableView*>(parent())->horizontalHeader()->resizeSection(CHECKED, 20);

        painter->save();
        QPen pen;

        if( index.column() == CHECKED )
        {
            painter->restore();

            bool checked = index.model()->data(index, Qt::DisplayRole).toBool();
            //select style for btn
            QStyleOptionButton *checkOpt = new QStyleOptionButton();
            checkOpt->state |= QStyle::State_Enabled;
            //decide on/off
            if(checked)
            {
                checkOpt->state |= QStyle::State_On;
            }
            else
            {
                checkOpt->state |= QStyle::State_Off;
            }
            //get shape of QCheckBox
            checkOpt->rect = CheckBoxRect(option);
            //draw QCheckBox
            QApplication::style()->drawControl(QStyle::CE_CheckBox, checkOpt, painter);
        }
        //  coloring
        else if(index.column() == LAYERNAME)
        {
            // background color of the 1st column
            painter->fillRect(opt.rect, QBrush(QColor(22, 83, 134)));

            // text color
            pen.setColor(QColor(200,106,123));
        }
        else if(index.column() == THICKNESS)
        {
            // getting data from database
            //QString thickness = qobject_cast<BonzaTableView*>(parent())->
            //        sqlModel()->index(index.row(), THICKNESS).data().toString();
            pen.setColor(QColor(200, 100, 140));
        }
        else if(index.column() == COLOR)
        {
            pen.setColor(QColor(140,200,25));
        }
        else if(index.column() == VS)
        {
            pen.setColor(QColor(40,240,25));

        }
        else if(index.column() == DENSITY)
        {
            // background color of even rows
            if( index.row()%2 == 0)
                painter->fillRect(opt.rect, QBrush(QColor(52, 113, 34)));

            pen.setColor(QColor(140,200,125));
        }
        else if(index.column() == MATERIAL)
        {
            // background color of even rows
            if( index.row()%2 == 0)
                painter->fillRect(opt.rect, QBrush(QColor(52, 113, 34)));

            pen.setColor(QColor(140,200,125));
        }

        if( index.column() != CHECKED )
        {
            painter->setPen(Qt::NoPen);
            painter->setRenderHint(QPainter::Antialiasing, true);
            // set the color for active, it seems didn't work
            if( opt.state & QStyle::State_Selected )
            {
                painter->setBrush(option.palette.highlightedText());
                painter->fillRect(opt.rect, QBrush(QColor(22, 183, 134)));
//                QItemDelegate::paint(painter, option, index);
            }
            // draw the background color and text color
            painter->setPen(pen);
            drawBackground(painter, opt, index);
            painter->drawText(opt.rect, opt.displayAlignment, text);
            painter->restore();
        }
    }

    bool editorEvent(QEvent *event, QAbstractItemModel *model,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index)
    {
        if( index.column() != CHECKED )
            return false;

        if( (event->type() == QEvent::MouseButtonRelease) ||
                (event->type() == QEvent::MouseButtonDblClick) )
        {
            QMouseEvent *mouse_event = static_cast<QMouseEvent*>(event);
            if( mouse_event->button() == Qt::LeftButton &&
                    CheckBoxRect(option).contains(mouse_event->pos()) )
            {
                int checked = model->data(index, Qt::DisplayRole).toInt();
                return model->setData(index, checked == 1 ? 0 : 1, Qt::CheckStateRole);
            }
        }


        return false;
    }

    QRect CheckBoxRect(const QStyleOptionViewItem &viewItemStyleOptions)const
    {
        //paramaters needed for drawing button
        QStyleOptionButton checkBoxStyleOption;
        //return the sub-region of the element accroding to the style specified
        QRect checkBoxRect = QApplication::style()->subElementRect( QStyle::SE_CheckBoxIndicator, &checkBoxStyleOption);
        //return the coords of the QCheckBox
        QPoint checkBoxPoint(viewItemStyleOptions.rect.x() + viewItemStyleOptions.rect.width() / 2 - checkBoxRect.width() / 2,
                             viewItemStyleOptions.rect.y() + viewItemStyleOptions.rect.height() / 2 - checkBoxRect.height() / 2);
        //return the shape of QCheckBox
        return QRect(checkBoxPoint, checkBoxRect.size());
    }

    QRect ComboBoxRect(const QStyleOptionViewItem &viewItemStyleOptions)const
    {
        //paramaters needed for drawing button
        QStyleOptionComplex comboBoxStyleOption;
        //return the sub-region of the element accroding to the style specified
        QRect comboBoxRect = QApplication::style()->subControlRect(QStyle::CC_ComboBox, &comboBoxStyleOption, QStyle::SC_ComboBoxEditField);
        //return the coords of the QComboBox
        QPoint comboBoxPoint(viewItemStyleOptions.rect.x() + viewItemStyleOptions.rect.width()  + comboBoxRect.width() ,
                             viewItemStyleOptions.rect.y() + viewItemStyleOptions.rect.height() / 2 - comboBoxRect.height() / 2);
        //return the shape of QComboBox
        return QRect(comboBoxPoint, comboBoxRect.size());
    }
};

#endif // BONZATABLEVIEW_H
