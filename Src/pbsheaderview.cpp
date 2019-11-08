#include "pbsheaderview.h"
#include <pbstablewidget.h>
#include <pbstableview.h>
#include <pbstools.h>
#include <FilterLineEdit.h>
#include <pbsfooterframe.h>

PBSHeaderView::PBSHeaderView(Qt::Orientation orientation, QWidget *parent) :
    QHeaderView (orientation, parent),
    headerState(browse)
{
    setAttribute(Qt::WA_Hover, true);

    setSectionResizeMode(QHeaderView::Interactive); // Sona Kadar Göster
    setSectionsClickable(true);
    setSortIndicatorShown(orientation == Qt::Horizontal);
    setSectionsClickable(true);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setSectionsMovable(orientation == Qt::Horizontal);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setStretchLastSection(orientation == Qt::Horizontal);
    setDragEnabled(orientation == Qt::Horizontal);
    setDragDropMode(orientation == Qt::Horizontal ? QAbstractItemView::DragDrop : QAbstractItemView::NoDragDrop);

    bFilterRow = true;
    setFilterRowVisible(false);

    if(parent && parent->inherits("QTableView"))
    {
        QTableView *v = static_cast<QTableView*>(parent);
        connect(v->horizontalScrollBar(), SIGNAL(valueChanged(int)), SLOT(adjustPositions()));
    }

    connect(this, SIGNAL(sectionResized(int,int,int)), SLOT(adjustPositions()));
    connect(this, SIGNAL(sectionCountChanged(int, int)), SLOT(sectionCountChanged(int, int)));
    connect(this, SIGNAL(sectionMoved(int, int, int)), SLOT(adjustPositions()));
}

PBSHeaderView::~PBSHeaderView()
{
    qDeleteAll(filterWidgets);
    filterWidgets.clear();
}

QRect PBSHeaderView::sectionRect(int logicalIndex) const
{
    return QRect(sectionViewportPosition(logicalIndex), 0, sectionSize(logicalIndex), height());
}

int PBSHeaderView::sectionHeight()
{
    if(orientation() == Qt::Horizontal && bFilterRow && filterWidgets.size())
        return height() - filterWidgets.at(0)->sizeHint().height();

    return height();
}

QSize PBSHeaderView::sizeHint() const
{
    QSize s = QHeaderView::sizeHint();
    if(model() && orientation() == Qt::Horizontal)
    {
        const QString str = model()->headerData(0, orientation()).toString();
        QSize sm = QSize(fontMetrics().boundingRect(str).width(),
                    fontMetrics().boundingRect(str).height());

        QStringList sl = str.split("\n");
        sm.setHeight(sm.height() * sl.count());

        if( bFilterRow && filterWidgets.size())
            s.setHeight(sm.height() + filterWidgets.at(0)->sizeHint().height() + 6);
        else
            s.setHeight(sm.height() + 6);
    }

    return s;
}

ItemState PBSHeaderView::getItemState(int ix)
{
    PBSTableWidget *pWidget = parent() ? (static_cast<PBSTableWidget*>(parent())) : nullptr;
    PBSTableView *pView = (pWidget && pWidget->inherits("PBSTableWidget")) ? (static_cast<PBSTableView*>(pWidget->parent())) : nullptr;

    return (pView && pView->inherits("PBSTableView")) ? pView->itemState(ix) : ItemState();
}

int PBSHeaderView::footerX()
{
    PBSTableWidget *pWidget = parent() ? (static_cast<PBSTableWidget*>(parent())) : nullptr;

    return pWidget ? (pWidget->verticalHeader() ? pWidget->verticalHeader()->width() : 0) : 0;
}

void PBSHeaderView::generateFilters(int number, bool showFirst)
{
    qDeleteAll(filterWidgets);
    filterWidgets.clear();

    if(!bFilterRow)
        return;

    for(int i = 0; i < number; ++i)
    {
        ItemState its = getItemState(i);

        FilterLineEdit* l = new FilterLineEdit(this, &filterWidgets, i);

        l->setVisible(showFirst && i == 0);
        l->setFilterVisible(its.bFiltered);

        l->setReadOnly(!its.bFiltered);
        l->setFocusPolicy(its.bFiltered ? Qt::WheelFocus : Qt::NoFocus);
        l->setPlaceholderText(its.bFiltered ? PBSTR("Filtre") : "");

        connect(l, SIGNAL(delayedTextChanged(QString)), this, SLOT(inputChanged(QString)));
        connect(l, SIGNAL(FilterButtonClicked(int)), this, SLOT(filterButtonClicked(int)));

        filterWidgets.push_back(l);
    }

    updateGeometries();
}

void PBSHeaderView::setFilterRowVisible(bool ba)
{
    bFilterRow = ba;

    if(bFilterRow)
        generateFilters(QHeaderView::count(), true);
    else
        generateFilters(0);

    updateGeometries();
}

bool PBSHeaderView::filterRowVisible()
{
    return bFilterRow;
}

void PBSHeaderView::updateGeometries()
{
    if(model() && orientation() == Qt::Horizontal)
    {
        const QString str = model()->headerData(0, orientation()).toString();
        QSize sm = QSize(fontMetrics().boundingRect(str).width(),
                    fontMetrics().boundingRect(str).height());

        QStringList sl = str.split("\n");
        sm.setHeight(sm.height() * sl.count());

        if(bFilterRow && filterWidgets.size())
        {
            setFixedHeight(sm.height() + filterWidgets.at(0)->sizeHint().height() + 6);
            setViewportMargins(0, 0, 0, filterWidgets.at(0)->sizeHint().height());
        }
        else
        {
            setFixedHeight(sm.height() + 6);
            setViewportMargins(0, 0, 0, 0);
        }

        QHeaderView::updateGeometries();
        adjustPositions();
        emit geometriesChanged();
    }
}

void PBSHeaderView::adjustPositions()
{
    QSize sm = sizeHint();

    for(int i = 0; i < filterWidgets.size(); ++i)
    {
        ItemState its = getItemState(i);

        QWidget *w = filterWidgets.at(i);
        w->hide();
        if(its.footerLabel)
            its.footerLabel->hide();

        if(isSectionHidden(i))
        {
            continue;
        }

        if(!its.bVisible)
        {
            continue;
        }

        if(bFilterRow)
        {
            w->show();

            if (QApplication::layoutDirection() == Qt::RightToLeft)
                w->move(width() - (sectionPosition(i) + sectionSize(i) - offset()), sm.height() - w->sizeHint().height());
            else
                w->move(sectionPosition(i) - offset(), sm.height() - w->sizeHint().height());
            w->resize(sectionSize(i), w->sizeHint().height());
        }
    }

    int xOffset = footerX();

    for(int i = 0; i < count(); i++)
    {
        ItemState its = getItemState(i);

        if(footerFrame() && its.fFooterEvent != skNone && its.footerLabel)
        {
            its.footerLabel->move(xOffset + sectionPosition(i) - offset(), 0);
            its.footerLabel->resize(sectionSize(i), footerFrame()->height());
            its.footerLabel->show();
        }
    }
}

void PBSHeaderView::sectionCountChanged(int oldCount, int newCount)
{
    Q_UNUSED(oldCount)

    generateFilters(newCount, true);
}

void PBSHeaderView::inputChanged(const QString &bFilter)
{
    int iCol = sender()->property("column").toInt();
    ItemState its = getItemState(iCol);

    // its.fFooterFilterType = fltText;
    its.sFilter = bFilter;

    emit filterChanged(iCol, bFilter);
}

void PBSHeaderView::filterButtonClicked(int iCol)
{
    emit filterButtonPressed(iCol);
}

void PBSHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    painter->save();
    QHeaderView::paintSection(painter, rect, logicalIndex);
    painter->restore();

    if (orientation() == Qt::Vertical)
    {
        if(selectionModel()->currentIndex().row() == logicalIndex)
        {
            QPixmap px(headerState == browse ? ":/Resource/Images/browse.png" : ":/Resource/Images/edit.png");
            painter->drawPixmap(rect.center() - QPoint(px.width() / 2, px.height() / 2), px);
        }
    }
}

PBSFooterFrame *PBSHeaderView::footerFrame()
{
    PBSTableWidget *pWidget = parent() ? (static_cast<PBSTableWidget*>(parent())) : nullptr;
    PBSTableView *pView = (pWidget && pWidget->inherits("PBSTableWidget")) ? (static_cast<PBSTableView*>(pWidget->parent())) : nullptr;

    return (pView && pView->inherits("PBSTableView")) ? pView->FooterFrame() : nullptr;
}
