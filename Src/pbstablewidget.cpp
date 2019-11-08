#include "pbstablewidget.h"
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QSqlError>
#include <QSqlQuery>
#include <PBSDataModel.h>
#include <pbstools.h>
#include <private/qtableview_p.h>

PBSTableCornerButton::PBSTableCornerButton(QWidget *parent)
    : QAbstractButton(parent)
{
    m_align = Qt::AlignCenter;
    m_text = "*";
    setFocusPolicy(Qt::NoFocus);

    setObjectName("PBSTABLWCORNERWIDGET");
}

void PBSTableCornerButton::paintEvent(QPaintEvent*)
{
    QStyleOptionHeader opt;
    opt.initFrom(this);
    QStyle::State state = isDown() ? QStyle::State_Sunken : QStyle::State_Raised;

    if (isEnabled())
    {
        state |= QStyle::State_Enabled;
    }

    if (isActiveWindow())
    {
        state |= QStyle::State_Active;
    }

    opt.state = state;
    opt.text = m_text;
    opt.rect = rect();
    opt.position = QStyleOptionHeader::OnlyOneSection;
    opt.textAlignment = m_align;
    QPainter painter(this);
    style()->drawControl(QStyle::CE_Header, &opt, &painter, this);
}

PBSTableWidget::PBSTableWidget(QWidget *parent) :
    QTableView(parent),
    pVertical(Qt::Vertical, this),
    pHorizontal(Qt::Horizontal, this),
    cornerButton(this),
    actDeleteRecord(QIcon(":/Resource/Images/cancel.png"), PBSTR("Kayıt Sil"), this)
{
    // setSelectionBehavior(QAbstractItemView::SelectItems);

    QList <QKeySequence>keyList;
    keyList << QKeySequence("Ctrl+Del");

    actDeleteRecord.setShortcuts(keyList);
    connect(&actDeleteRecord, &QAction::triggered, this, &PBSTableWidget::onActDeleteRecord);
    addAction(&actDeleteRecord);

    setAutoScroll(true);
    setMouseTracking(true);

    setAlternatingRowColors(true);
    setSortingEnabled(true);

    setNextCellOnEnter(false);
    setNextRowOnLastCol(false);
    setGotoFirstCellOnNewRecord(false);

    Q_D(QTableView);

    disconnect(d->cornerWidget, SIGNAL(clicked()), this, SLOT(selectAll()));
    delete(d->cornerWidget);

    d->cornerWidget = &cornerButton;

    setEnableQuickCustomize(true);

    setVerticalHeader(&pVertical);
    setHorizontalHeader(&pHorizontal);

    connect(&pHorizontal, SIGNAL(filterChanged(int, QString)), SLOT(onFilterChanged(int, const QString)));
    connect(&pHorizontal, SIGNAL(filterButtonPressed(int)), SLOT(onFilterButtonPressed(int)));
}

PBSTableWidget::~PBSTableWidget()
{
}

void PBSTableWidget::showEvent(QShowEvent*)
{
/*
    connect(selectionModel(),
            SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
            SLOT(onSelectionChanged(const QItemSelection &, const QItemSelection &)));
*/
}

void PBSTableWidget::onFilterChanged(int iCol, const QString bFilter)
{
    emit filterChanged(iCol, bFilter);
}

void PBSTableWidget::onFilterButtonPressed(int iCol)
{
    emit filterButtonPressed(iCol);
}

void PBSTableWidget::setModel(QAbstractItemModel *model)
{
   QTableView::setModel(model);
}

bool PBSTableWidget::addNewRecord(const QModelIndex &index)
{
    if(editTriggers() == QAbstractItemView::NoEditTriggers) // read Only
        return false;

    if(!bCanAddRow)
        return false;

    if(model() && model()->inherits("PBSDataModel"))
    {
        PBSDataModel *pModel = static_cast<PBSDataModel*>(model());
        if(pModel)
        {
            int r = currentIndex().row(),
                c = currentIndex().column();

            if(!pModel->haveEmptyRow())
            {
                pModel->insertEmptyRow();
                emit newRecordAdd();

                resizeRowToContents(pModel->rowCount());
                scrollToBottom();

                if(bGotoFirstCellOnNewRec)
                    c = nextVisibleIndex(0);
                moveTo(pModel->rowCount(), c);

                emit newRecordAdd();
            }
        }
    }
    else
    {
        model()->insertRow(model()->rowCount(QModelIndex()));
        model()->submit();        
    }
    return true;
}

int PBSTableWidget::currentRow()
{
    return currentIndex().row();
}

int PBSTableWidget::currentColumn()
{
    QHeaderView *hdr = static_cast<QHeaderView*>(horizontalHeader());
    if(!hdr)
       return currentIndex().column();
    return hdr->visualIndex(currentIndex().column());
}

QModelIndex PBSTableWidget::currentPos()
{
    QHeaderView *hdr = static_cast<QHeaderView*>(horizontalHeader());
    if(hdr && model())
    {
        QModelIndex mi = model()->index(currentIndex().row(), hdr->visualIndex(currentIndex().column()));

        return mi;
    }

    return currentIndex();
}

void PBSTableWidget::gotoRow(int r)
{
    QHeaderView *hdr = static_cast<QHeaderView*>(horizontalHeader());
    if(hdr)
        moveTo(r, hdr->visualIndex(currentIndex().column()));
    else
        moveTo(r, currentIndex().column());
}

void PBSTableWidget::gotoColumn(int c)
{
    QHeaderView *hdr = static_cast<QHeaderView*>(horizontalHeader());
    if(hdr)
        moveTo(currentIndex().row(), hdr->visualIndex(c));
    else
        moveTo(currentIndex().row(), c);
}

void PBSTableWidget::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QTableView::selectionChanged(selected, deselected);

    QHeaderView *hdr = static_cast<QHeaderView*>(horizontalHeader());
    if(hdr)
        emit onPositionChanged(currentIndex().row(), hdr->visualIndex(currentIndex().column()));
    else
        emit onPositionChanged(currentIndex().row(), currentIndex().column());
}

int PBSTableWidget::nextVisibleIndex(int c)
{
    QHeaderView *hdr = static_cast<QHeaderView*>(horizontalHeader());
    if(!hdr)
        return c;

    if(c > hdr->count() - 1)
        return nextVisibleIndex(hdr->count() - 1);

    for(int i = c; i < hdr->count() - 1; i++)
    {
        if(!hdr->isSectionHidden(i))
                return i;
    }
    return c;
}

void PBSTableWidget::moveTo(int r, int c)
{
    QHeaderView *hdr = static_cast<QHeaderView*>(horizontalHeader());
    if(!hdr)
        return;

    if(c > hdr->count() - 1)
        c = hdr->count() - 1;

    if(r > model()->rowCount() - 1)
        r = model()->rowCount() - 1;

    c = nextVisibleIndex(hdr->logicalIndex(c));

    QModelIndex oi = currentIndex(),
                ix = model()->index(r, c);

    qDebug() << "Pos" << r << ":" << c << "MoveTo" << ix << "IsValid ?" << ix.isValid();
    if(ix.isValid())
    {
        QItemSelection os(currentIndex(), currentIndex());

        setCurrentIndex(ix);
        selectionModel()->setCurrentIndex(ix, QItemSelectionModel::ClearAndSelect);

        QItemSelection ns(currentIndex(), currentIndex());

        QTableView::selectionChanged(os, ns);

        qDebug() << "MOveTo" << currentIndex();
        emit onPositionChanged(currentRow(), currentColumn());
    }
}

void PBSTableWidget::keyPressEvent(QKeyEvent *e)
{
    if(bNextCellOnEnter && (e->key() == Qt::Key_Return | e->key() == Qt::Key_Enter))
    {
        qint32  cr = currentIndex().row(),
                cc = currentIndex().column(),
                c  = model()->columnCount();

        QHeaderView *hdr = static_cast<QHeaderView*>(horizontalHeader());
        if(hdr)
           cc = hdr->visualIndex(cc);

        if(cc + 1 < c)
        {
            moveTo(cr, cc + 1);

            if(editTriggers() == QAbstractItemView::AllEditTriggers)
                edit(currentIndex());
        }
        else if(bNextRowOnLastCol)
        {
            if(cr < model()->rowCount() - 1)
                moveTo(cr + 1, 0);
            if(editTriggers() == QAbstractItemView::AllEditTriggers)
                edit(currentIndex());
        }
        else
           QTableView::keyPressEvent(e);
    }
    else if(e->key() == Qt::Key_Down && currentIndex().row() == model()->rowCount() - 1)
    {
        addNewRecord(currentIndex());

        // QTableView::keyPressEvent(e);
    }
    else
    {
        QModelIndex c(currentIndex());

        QTableView::keyPressEvent(e);

        if(c.row() != currentIndex().row() || c.column() != currentIndex().column())
        {
            selectionModel()->select(currentIndex(), QItemSelectionModel::Select);
            selectionModel()->select(c, QItemSelectionModel::Deselect);
        }
    }
}

void PBSTableWidget::setColumnAutoWidth(bool ba)
{
    bColumnAutoWidth = ba;
    horizontalHeader()->setSectionResizeMode(ba ? QHeaderView::Stretch : QHeaderView::Interactive);
}

bool PBSTableWidget::columnAutoWidth()
{
    return bColumnAutoWidth;
}

void PBSTableWidget::setSortEnabled(bool ba)
{
    bSortEnabled = ba;
    setSortingEnabled(ba);
}

bool PBSTableWidget::sortEnabled()
{
    return bSortEnabled;
}

void PBSTableWidget::setAutoEdit(bool b)
{
    bAutoEdit = b;
    setEditTriggers(b ? QAbstractItemView::AllEditTriggers :
                        QAbstractItemView::AnyKeyPressed | QAbstractItemView::DoubleClicked);
}

bool PBSTableWidget::autoEdit()
{
    return bAutoEdit;
}

bool PBSTableWidget::readOnly()
{
    return bReadOnly;
}

void PBSTableWidget::setReadOnly(bool b)
{
    bReadOnly = b;
    if(b)
        setEditTriggers(QAbstractItemView::NoEditTriggers);
    else
        setAutoEdit(bAutoEdit);
}

bool PBSTableWidget::selectRow()
{
    return bSelectRow;
}

bool PBSTableWidget::enableQuickCustomize()
{
    return bQuickCustomize;
}

void PBSTableWidget::setEnableQuickCustomize(bool cc)
{
    bQuickCustomize = cc;

    if(bQuickCustomize)
    {
        cornerButton.setText("*");

        disconnect(&cornerButton, SIGNAL(clicked()), this, SLOT(selectAll()));
        connect(&cornerButton , SIGNAL(clicked()), this, SLOT(cornerButtonPressed()));
    }
    else
    {
        cornerButton.setText("");

        disconnect(&cornerButton, SIGNAL(clicked()), this, SLOT(cornerButtonPressed()));
        connect(&cornerButton, SIGNAL(clicked()), this, SLOT(selectAll()));
    }
    cornerButton.update();
}

void PBSTableWidget::cornerButtonPressed()
{
    emit onCornerButtonPressed();
}

void PBSTableWidget::setSelectRow(bool b)
{
    bSelectRow = b;
    setSelectionBehavior(b ? QAbstractItemView::SelectRows : QAbstractItemView::SelectItems);
}

bool PBSTableWidget::alternateRowColor()
{
    return bSelectRow;
}

void PBSTableWidget::setAlternateRowColor(bool b)
{
    bAlternateRowColor = b;
    setAlternatingRowColors(b);
}

bool PBSTableWidget::edit(const QModelIndex &index, EditTrigger trigger, QEvent *event)
{
    bool b =QTableView::edit(index, trigger, event);

    PBSHeaderView *vhdr = static_cast<PBSHeaderView*>(verticalHeader());
    if(vhdr)
        vhdr->headerState = b ? PBSHeaderView::edit : PBSHeaderView::browse;

    return b;
}

void PBSTableWidget::edit(const QModelIndex &index)
{
    PBSHeaderView *vhdr = static_cast<PBSHeaderView*>(verticalHeader());
    if(vhdr)
        vhdr->headerState = PBSHeaderView::edit;

   QTableView::edit(index);
}

void PBSTableWidget::closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint)
{
    PBSHeaderView *vhdr = static_cast<PBSHeaderView*>(verticalHeader());
    if(vhdr)
    {
        vhdr->headerState = PBSHeaderView::browse;
        vhdr->update(selectionModel()->currentIndex());
    }
   QTableView::closeEditor(editor, hint);
}

void PBSTableWidget::setSectionMoveable(bool b)
{
    horizontalHeader()->setSectionsMovable(b);
}

bool PBSTableWidget::sectionMoveable()
{
    return horizontalHeader()->sectionsMovable();
}

bool PBSTableWidget::canDeleteRow()
{
    return bCanDeleteRow;
}

void PBSTableWidget::setCanDeleteRow(bool b)
{
    bCanDeleteRow = b;
}

void PBSTableWidget::onActDeleteRecord()
{
    emit onDeleteRecord();
}

bool PBSTableWidget::canAddRow()
{
    return bCanAddRow;
}

void PBSTableWidget::setCanAddRow(bool b)
{
    bCanAddRow = b;
}
