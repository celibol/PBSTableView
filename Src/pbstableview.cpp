#include "pbstableview.h"
#include <pbstools.h>
#include <pbsdatamodel.h>
#include <FilterLineEdit.h>
#include <pbsappsettings.h>
#include <pbsfileutil.h>
#include <pbsmodelexporter.h>
#include <pbstableprinter.h>
#include <QPrintPreviewDialog>
#include <QScreen>
#include <QMessageBox>

PBSTableView::PBSTableView(QString sSettings, QWidget *parent) :
    QWidget(parent),
    sSettingsName(sSettings),
    pTableWidget(this),
    pFooterFrame(this),
    popupMenu(this),
    pFilterFrame(this),
    customizeFrame(this, Qt::Popup | Qt::Window),
    pLayout(this),
    actFooter(QIcon(":/Resource/Images/footer.png"), PBSTR("Bilgi Satırı"), this),
    actFilter(QIcon(":/Resource/Images/filter.png"), tr( "Filtreleme"), this),
    actSortAsc(QIcon(":/Resource/Images/Ascending.png"), PBSTR("A-Z Sıralaması"), this),
    actSortDesc(QIcon(":/Resource/Images/descending.png"), PBSTR("Z-A Sıralaması"), this),
    actStretchColumn(QIcon(":/Resource/Images/stretch.png"), PBSTR("En iyi Uzunluk"), this),
    actStretchColumnAll(QIcon(":/Resource/Images/stretchall.png"), PBSTR("En iyi Uzunluk (Tüm Kolonlar)"), this),
    actStretch(QIcon(":/Resource/Images/sigdir.png"), PBSTR("Kolonları Ekrana Sığdır"), this),
    actSaveExcel(QIcon(":/Resource/Images/xls.png"), PBSTR("Excel Dosyası Olarak"), this),
    actSaveExcelXML(QIcon(":/Resource/Images/xls.png"), PBSTR("Excel (XML) Dosyası Olarak"), this),
    actSaveWordXML(QIcon(":/Resource/Images/word.png"), PBSTR("Word (XML) Dosyası Olarak"), this),
    actSendToPrinter(QIcon(":/Resource/Images/printer.png"), PBSTR("Yazdır..."), this),
    actSavePDF(QIcon(":/Resource/Images/pdf.png"), PBSTR("PDF Dosyası Olarak"), this),
    actSaveCSV(QIcon(":/Resource/Images/csv.png"), PBSTR("Text (CSV) Dosyası Olarak"), this),
    actSaveXML(QIcon(":/Resource/Images/xml.png"), PBSTR("XML Dosyası Olarak"), this),
    actSaveHTML(QIcon(":/Resource/Images/htm.png"), PBSTR("HTML Dosyası Olarak"), this),
    actSaveLayout(QIcon(":/Resource/Images/save.png"), PBSTR("Görünümü Kaydet"), this)
{
    setReadOnly(false);
    setAutoEdit(false);
    setColumnAutoWidth(true);
    setFilterRowVisible(false);
    setFooterRowVisible(false);
    setSortEnabled(true);
    setSelectRow(false);

    customizeFrame.setObjectName("PBSTABLEVIEWCUSTOMIZEFRAME");
    customizeFrame.setFrameShape(QFrame::StyledPanel);
    customizeFrame.raise();
    customizeFrame.setMinimumHeight(200);
    customizeFrame.setMinimumWidth(175);
    customizeFrame.resize(175, 100);

    customizeListWidget.setParent(&customizeFrame);
    customizeListWidget.setObjectName("PBSTABLEVIEWCUSTOMIZEFRAMELISTWIDGET");
    customizeListWidget.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    customizeListWidget.resize(175, 200);
    customizeListWidget.setResizeMode(QListView::Adjust);

    pFilterFrame.hide();
    customizeFrame.hide();

    pTableWidget.setSectionMoveable(true);
    pTableWidget.setAlternateRowColor(true);

    createActions();
    createPopupMenu();

    connect(&pTableWidget, SIGNAL(onCornerButtonPressed()), SLOT(cornerButtonPressed()));
    connect(&pTableWidget, SIGNAL(onPositionChanged(int, int)), SLOT(positionChanged(int, int)));
    connect(&pTableWidget, SIGNAL(newRecordAdd()), SLOT(onNewRecord()));
    connect(&pTableWidget, SIGNAL(onDeleteRecord()), SLOT(onDeleteRecord()));
    connect(&pTableWidget, SIGNAL(filterChanged(int, QString)), SLOT(onFilterChanged(int, const QString)));
    connect(&pTableWidget, SIGNAL(filterButtonPressed(int)), SLOT(onFilterButtonPressed(int)));
    connect(pTableWidget.horizontalHeader(), SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showCustomContextMenu(const QPoint&)));

    connect(&pFooterFrame, SIGNAL(onCreatePopupMenu(QMenu *)), SLOT(onCreateFooterMenu(QMenu *)));
    connect(&pFooterFrame, SIGNAL(onValueChanged()), SLOT(footerValueChanged()));

    pLayout.setSpacing(0);
    pLayout.setMargin(0);
    pLayout.setAlignment(Qt::AlignTop);

    pLayout.addWidget(&pTableWidget);
    pLayout.addWidget(&pFooterFrame);

    setLayout(&pLayout);
}

PBSTableWidget *PBSTableView::tableWidget()
{
    return &pTableWidget;
}

PBSFooterFrame *PBSTableView::FooterFrame()
{
    return &pFooterFrame;
}

itemStateMap &PBSTableView::ItemStateMap()
{
    return tItemStateMap;
}

ItemState PBSTableView::itemState(int ix)
{
    return tItemStateMap[ix];
}

void PBSTableView::setItemStateMap(int i, ItemState its)
{
    tItemStateMap[i] = its;
}

void PBSTableView::setItemFilter(int ix, QString flt, QString fltFlt)
{
    ItemState its = tItemStateMap[ix];

    its.sFilter = flt;
    its.sFilterBoxFilter = fltFlt;

    tItemStateMap[ix] = its;
}

void PBSTableView::setItemFilter(int ix, QString flt)
{
    ItemState its = tItemStateMap[ix];

    its.sFilter = flt;

    tItemStateMap[ix] = its;
}

void PBSTableView::setItemFilterBoxFilter(int ix, QString flt)
{
    ItemState its = tItemStateMap[ix];

    its.sFilterBoxFilter = flt;

    tItemStateMap[ix] = its;
}

void PBSTableView::reOpenQuery()
{
    if(!pModel && !pModel->inherits("PBSDataModel"))
        return;

    PBSDataModel *pM = qobject_cast<PBSDataModel*>(pTableWidget.model());
    if(pM)
    {
        applyFilter();
        pM->openQuery();
        pFooterFrame.calculate();
    }
}

void PBSTableView::setItemCaption(int ix, QString s)
{
    ItemState its = tItemStateMap[ix];

    its.sCaption = s;

    tItemStateMap[ix] = its;
}

void PBSTableView::showEvent(QShowEvent*)
{
    loadState();

    PBSHeaderView *hdr = static_cast<PBSHeaderView*>(pTableWidget.horizontalHeader());
    for(int i = 0; i < hdr->count(); i++)
    {
        if(itemState(i).bHidden || !itemState(i).bVisible)
            hdr->hideSection(i);
    }
    hdr->generateFilters(hdr->count());

    setFooterRowVisible(bFooterRowVisible);
    actStretch.setChecked(pTableWidget.horizontalHeader()->sectionResizeMode(0) == QHeaderView::Stretch);
}

void PBSTableView::createActions()
{
    actFooter.setCheckable(true);
    actFilter.setCheckable(true);
    actSortAsc.setCheckable(true);
    actSortDesc.setCheckable(true);
    actStretch.setCheckable(true);

    actFooter.setChecked(bFooterRowVisible);
    actFilter.setChecked(bFilterRowVisible);
    actStretch.setChecked(pTableWidget.columnAutoWidth());

    actSortAsc.setVisible(pTableWidget.sortEnabled());
    actSortDesc.setVisible(pTableWidget.sortEnabled());

    connect(&actFooter, &QAction::triggered, this, &PBSTableView::onActFooter);
    connect(&actFilter, &QAction::triggered, this, &PBSTableView::onActFilter);
    connect(&actSortAsc, &QAction::triggered, this, &PBSTableView::onActSortAsc);
    connect(&actSortDesc, &QAction::triggered, this, &PBSTableView::onActSortDesc);
    connect(&actStretchColumn, &QAction::triggered, this, &PBSTableView::onActStretchColumn);
    connect(&actStretchColumnAll, &QAction::triggered, this, &PBSTableView::onActStretchColumnAll);
    connect(&actStretch, &QAction::triggered, this, &PBSTableView::onActStretch);

    connect(&actSaveExcel, &QAction::triggered, this, &PBSTableView::onActSaveExcel);
    connect(&actSaveExcelXML, &QAction::triggered, this, &PBSTableView::onActSaveExcelXML);
    connect(&actSaveWordXML, &QAction::triggered, this, &PBSTableView::onActSaveWordXML);
    connect(&actSavePDF, &QAction::triggered, this, &PBSTableView::onActSavePDF);
    connect(&actSaveCSV, &QAction::triggered, this, &PBSTableView::onActSaveCSV);
    connect(&actSaveXML, &QAction::triggered, this, &PBSTableView::onActSaveXML);
    connect(&actSaveHTML, &QAction::triggered, this, &PBSTableView::onActSaveHTML);

    connect(&actSendToPrinter, &QAction::triggered, this, &PBSTableView::onActSendToPrinter);

    connect(&actSaveLayout, &QAction::triggered, this, &PBSTableView::onActSaveLayout);
}

void PBSTableView::createPopupMenu()
{
    popupMenu.clear();
    popupMenu.addAction(&actSortAsc);
    popupMenu.addAction(&actSortDesc);
    popupMenu.addSeparator();
    popupMenu.addAction(&actFooter);
    popupMenu.addAction(&actFilter);
    popupMenu.addSeparator();
    popupMenu.addAction(&actSendToPrinter);
    popupMenu.addSeparator();
    popupMenu.addAction(&actStretchColumn);
    popupMenu.addAction(&actStretchColumnAll);
    popupMenu.addAction(&actStretch);

    popupMenu.addSeparator();
    QMenu *saveAsMenu = popupMenu.addMenu(QIcon(":/Resource/Images/save.png"), PBSTR("... Olarak Sakla"));
    saveAsMenu->addAction(&actSaveExcel);
    saveAsMenu->addAction(&actSaveExcelXML);
    saveAsMenu->addAction(&actSaveWordXML);
    saveAsMenu->addSeparator();
    saveAsMenu->addAction(&actSavePDF);
    saveAsMenu->addAction(&actSaveCSV);
    saveAsMenu->addAction(&actSaveXML);
    saveAsMenu->addAction(&actSaveHTML);

    popupMenu.addSeparator();
    popupMenu.addAction(&actSaveLayout);

    emit onCreatePopupMenu(HeaderMenu, &popupMenu);
}

void PBSTableView::onCreateFooterMenu(QMenu *m)
{
    emit onCreatePopupMenu(FooterMenu, m);
}

void PBSTableView::footerValueChanged()
{
    emit onFooterValueChanged();
}

QMenu &PBSTableView::headerMenu()
{
    return popupMenu;
}

QMenu *PBSTableView::footerMenu()
{
    return pFooterFrame.menu();
}

QVariant PBSTableView::footerValue(int i) const
{
    ItemState its = tItemStateMap[i];

    return its.footerValue;
}

void PBSTableView::setItemFormat(int ix, QString sFormat, QString sFooterFormat)
{
    ItemState its = tItemStateMap[ix];

    its.sFormat = sFormat;
    its.sFooterFormat = sFooterFormat;

    tItemStateMap[ix] = its;
}

void PBSTableView::setItemAlignment(int ix, Qt::Alignment col, Qt::Alignment footer)
{
    ItemState its = tItemStateMap[ix];

    its.aAlignColumn = col;
    its.aAlignFooter = footer;

    if(its.footerLabel)
        its.footerLabel->setAlignment(footer);

    tItemStateMap[ix] = its;
}

void PBSTableView::setItemEditor(int ix, itemEditType e)
{
    ItemState itst = tItemStateMap[ix];
    itst.editType = e;
    tItemStateMap[ix] = itst;
}

void PBSTableView::setItemState(int ix, ItemStates its, bool b)
{
    ItemState itst = tItemStateMap[ix];
    switch(its)
    {
        case Sortable  : itst.bSortable = b; break;
        case Filter    : itst.bFiltered = b; break;
        case Hidden    : itst.bHidden = b; break;
        case Visible   : itst.bVisible = b; break;
        case Editable  : itst.bEditable = b; break;
        case Selectable: itst.bSelectable = b; break;
    }
    tItemStateMap[ix] = itst;
}

void PBSTableView::setItemState(int ix, bool select, bool edit, bool hidden, bool visible, bool filter, bool sort)
{
    ItemState its = tItemStateMap[ix];

    its.bSelectable = select;
    its.bEditable   = edit;
    its.bFiltered   = filter;
    its.bSortable   = sort;
    its.bVisible    = visible;
    its.bHidden     = hidden;

    tItemStateMap[ix] = its;
}

void PBSTableView::setFooterEvent(int ix, footerEvent fe)
{
    ItemState its = tItemStateMap[ix];

    if(!its.footerLabel)
    {
        its.footerLabel = new QLabel("", FooterFrame());
        its.footerLabel->setAlignment(its.aAlignFooter);
    }

    if(fe == skNone)
    {
        if(its.footerLabel)
            its.footerLabel->setText("");
        its.footerValue.clear();
    }

    int xOffset = tableWidget()->verticalHeader()->width();

    QHeaderView *p = tableWidget()->horizontalHeader();

    its.footerLabel->move(xOffset + p->sectionPosition(ix) - p->offset(), 0);
    its.footerLabel->resize(p->sectionSize(ix), pFooterFrame.height());
    its.footerLabel->setAlignment(its.aAlignFooter | Qt::AlignVCenter);
    its.footerLabel->setObjectName("PBSTABLEFOOTERLABEL");
    its.fFooterEvent = fe;
    pFooterFrame.setFixedHeight(25);

    tItemStateMap[ix] = its;

    if(its.footerLabel)
        its.footerLabel->show();
}

void PBSTableView::setModel(QAbstractItemModel *model)
{
    model->setParent(this);

    pModel = model;
    pTableWidget.setModel(model);

    connect(pModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)), this, SLOT(dataChanged(QModelIndex, QModelIndex)));
}

void PBSTableView::dataChanged(const QModelIndex &, const QModelIndex &)
{
    pFooterFrame.calculate();
}

bool PBSTableView::enableQuickCustomize()
{
    return pTableWidget.enableQuickCustomize();
}

void PBSTableView::setEnableQuickCustomize(bool cc)
{
    pTableWidget.setEnableQuickCustomize(cc);
}

void PBSTableView::setColumnAutoWidth(bool ba)
{
    pTableWidget.setColumnAutoWidth(ba);
}

bool PBSTableView::columnAutoWidth()
{
    return pTableWidget.columnAutoWidth();
}

void PBSTableView::setSortEnabled(bool ba)
{
    pTableWidget.setSortEnabled(ba);

    actSortAsc.setEnabled(ba);
    actSortDesc.setEnabled(ba);
}

bool PBSTableView::sortEnabled()
{
    return pTableWidget.sortEnabled();
}

void PBSTableView::setAutoEdit(bool b)
{
    pTableWidget.setAutoEdit(b);
}

bool PBSTableView::autoEdit()
{
    return pTableWidget.autoEdit();
}

bool PBSTableView::readOnly()
{
    return pTableWidget.readOnly();
}

void PBSTableView::setReadOnly(bool b)
{
    pTableWidget.setReadOnly(b);
}

bool PBSTableView::selectRow()
{
    return pTableWidget.selectRow();
}

void PBSTableView::setSelectRow(bool b)
{
    pTableWidget.setSelectRow(b);
}

bool PBSTableView::nextCellOnEnter()
{
    return pTableWidget.nextCellOnEnter();
}

void PBSTableView::setNextCellOnEnter(bool b)
{
    pTableWidget.setNextCellOnEnter(b);
}

bool PBSTableView::nextRowOnLastCol()
{
    return pTableWidget.nextRowOnLastCol();
}

void PBSTableView::setNextRowOnLastCol(bool b)
{
    pTableWidget.setNextRowOnLastCol(b);
}

bool PBSTableView::gotoFirstCellOnNewRecord()
{
    return pTableWidget.gotoFirstCellOnNewRecord();
}

void PBSTableView::setGotoFirstCellOnNewRecord(bool b)
{
    pTableWidget.setGotoFirstCellOnNewRecord(b);
}

void PBSTableView::setFilterRowVisible(bool ba)
{
    bFilterRowVisible = ba;
    actFilter.setChecked(ba);

    PBSHeaderView *pHeader = static_cast<PBSHeaderView*>(pTableWidget.horizontalHeader());
    if(pHeader)
        pHeader->setFilterRowVisible(ba);
}

void PBSTableView::setItemDelegate(QAbstractItemDelegate *delegate)
{
    pTableWidget.setItemDelegate(delegate);
}

QAbstractItemDelegate *PBSTableView::itemDelegate() const
{
    return pTableWidget.itemDelegate();
}

bool PBSTableView::filterRowVisible()
{
    return bFilterRowVisible;
}

void PBSTableView::setFooterRowVisible(bool ba)
{
    bFooterRowVisible = ba;
    actFooter.setChecked(ba);

    if(ba)
        pFooterFrame.show();
    else
        pFooterFrame.hide();
}

bool PBSTableView::footerRowVisible()
{
    return bFooterRowVisible;
}

void PBSTableView::onActFooter()
{
    setFooterRowVisible(!footerRowVisible());
}

void PBSTableView::onActFilter()
{
    setFilterRowVisible(!filterRowVisible());
}

void PBSTableView::onActSortAsc()
{
    if(!pTableWidget.sortEnabled()) return;

    QPoint mP = mapFromGlobal(popupMenu.pos());
    int iCol = pTableWidget.horizontalHeader()->logicalIndexAt(mP);

    actSortAsc.setChecked(true);
    actSortDesc.setChecked(false);

    pTableWidget.sortByColumn(iCol, Qt::AscendingOrder);
}

void PBSTableView::onActSortDesc()
{
    if(!pTableWidget.sortEnabled()) return;

    QPoint mP = mapFromGlobal(popupMenu.pos());
    int iCol = pTableWidget.horizontalHeader()->logicalIndexAt(mP);

    actSortAsc.setChecked(false);
    actSortDesc.setChecked(true);

    pTableWidget.sortByColumn(iCol, Qt::DescendingOrder);
}

void PBSTableView::onActStretchColumn()
{
    QPoint mP = mapFromGlobal(popupMenu.pos());
    int iCol = pTableWidget.horizontalHeader()->logicalIndexAt(mP);

    actStretch.setChecked(false);

    pTableWidget.resizeColumnToContents(iCol);
    pTableWidget.horizontalHeader()->update();
}

void PBSTableView::onActStretchColumnAll()
{
    pTableWidget.resizeColumnsToContents();

    PBSHeaderView *hdr = static_cast<PBSHeaderView*>(pTableWidget.horizontalHeader());
    if(hdr)
        hdr->adjustPositions();

    actStretch.setChecked(false);
    hdr->update();
}

void PBSTableView::onActStretch()
{
    pTableWidget.horizontalHeader()->setSectionResizeMode(actStretch.isChecked() ? QHeaderView::Stretch : QHeaderView::Interactive);
    PBSHeaderView *hdr = static_cast<PBSHeaderView*>(pTableWidget.horizontalHeader());
    if(hdr)
        hdr->adjustPositions();
   hdr->update();
}

void PBSTableView::showCustomContextMenu(const QPoint &p)
{
    QPoint mP = mapToGlobal(p);

    int iCol = pTableWidget.horizontalHeader()->logicalIndexAt(p),
    iSort = pTableWidget.horizontalHeader()->sortIndicatorSection();

    actSortAsc.setChecked(iCol == iSort && pTableWidget.horizontalHeader()->sortIndicatorOrder() == Qt::AscendingOrder);
    actSortDesc.setChecked(iCol == iSort && pTableWidget.horizontalHeader()->sortIndicatorOrder() == Qt::DescendingOrder);

    popupMenu.popup(mP);
}

void PBSTableView::onActSaveLayout()
{
    saveState();
}

int PBSTableView::currentRow()
{
    return pTableWidget.currentRow();
}

int PBSTableView::currentColumn()
{
    QHeaderView *hdr = static_cast<QHeaderView*>(pTableWidget.horizontalHeader());
    if(hdr)
        hdr->visualIndex(pTableWidget.currentColumn());

    return pTableWidget.currentColumn();
}

QModelIndex PBSTableView::currentPos()
{
    return pTableWidget.currentPos();
}

void PBSTableView::gotoRow(int r)
{
    pTableWidget.gotoRow(r);
    emit onPositionChanged(currentRow(), currentColumn());
}

void PBSTableView::gotoColumn(int c)
{
    pTableWidget.gotoColumn(c);
    emit onPositionChanged(currentRow(), currentColumn());
}

void PBSTableView::moveTo(int r, int c)
{
    pTableWidget.moveTo(r, c);
    emit onPositionChanged(r, c);
}

void PBSTableView::positionChanged(int r, int c)
{
    emit onPositionChanged(r, c);
}

void PBSTableView::onNewRecord()
{
    PBSHeaderView *hdr = static_cast<PBSHeaderView*>(pTableWidget.horizontalHeader());

    for(int i = 0; i < hdr->count(); i++)
    {
        if(!tItemStateMap[i].bVisible | tItemStateMap[i].bHidden)
        {
            qDebug() << i << "Column Hidden";
            hdr->hideSection(i);
        }
    }

    if(gotoFirstCellOnNewRecord())
    {        
        int c = 0;
        for(int i = 0; i < hdr->count(); i++)
        {
            if(tItemStateMap[i].bVisible && tItemStateMap[i].bEditable && tItemStateMap[i].bSelectable)
            {
                c = i;
                break;
            }
        }
        gotoColumn(c);
    }
    emit newRecordAdd();
}

void PBSTableView::onFilterChanged(int iCol, const QString bFilter)
{
    ItemState its = tItemStateMap[iCol];

    // its.fFooterFilterType = fltText;
    its.sFilter = bFilter;

    tItemStateMap[iCol] = its;

    emit filterChanged(iCol, bFilter);

    reOpenQuery();
}

void PBSTableView::applyFilter()
{
    PBSDataModel *pM = qobject_cast<PBSDataModel*>(pTableWidget.model());
    PBSHeaderView *hdr = static_cast<PBSHeaderView*>(pTableWidget.horizontalHeader());
    if(hdr && pM)
    {
        QString sFilter = "";

        QSqlDriver *driver = pM->database().driver();

        for(int i = 0; i < hdr->count(); i++)
        {
            QString s(tItemStateMap[i].sFilter);

            qDebug() << i << "Filter Box" << tItemStateMap[i].sFilterBoxFilter;

            if(!s.isEmpty())
            {
                QSqlField field(pM->record().field(i));

                if(s.startsWith("%") || s.startsWith("*"))
                    s.remove(0, 1);

                if(s.endsWith("%") ||s.endsWith("*"))
                    s.chop(1);

                switch(field.type())
                {
                    case QVariant::Bool:
                        if(s[0] == '1' || s.toUpper()[0] == 'T' || s.toUpper()[0] == 'E')
                            s = QString("%1 = %2").arg(field.name()).arg(1);
                        else
                            s = QString("%1 = %2").arg(field.name()).arg(0);
                    break;

                    case QVariant::Int:
                    case QVariant::UInt:
                    case QVariant::LongLong:
                    case QVariant::ULongLong:
                    case QVariant::Double:
                    {
                        bool b;
                        double d = s.toDouble(&b);
                        if(b)
                            s = QString("%1 = %2").arg(field.name()).arg(s);
                        else
                            s = "";
                    }
                    break;

                    case QVariant::String:
                        s = QString("%1 LIKE '%%%2%%'").arg(field.name()).arg(s);
                    break;

                    case QVariant::Date:
                    case QVariant::Time:
                    case QVariant::DateTime:
                    {
                        QDateTime dt(QDateTime::fromString(s));
                        if(dt.isValid())
                        {
                            field.setValue(dt);
                            s = QString("%1 = '%2'").arg(field.name()).arg(driver->formatValue(field));
                        }
                        else
                            s = "";
                    }
                    break;

                    default:
                        s = "";
                    break;
                }
            }

            if(!tItemStateMap[i].sFilterBoxFilter.isEmpty())
            {
                if(s.isEmpty())
                    s = tItemStateMap[i].sFilterBoxFilter;
                else
                    s = QString("%1 AND %2").arg(s).arg(tItemStateMap[i].sFilterBoxFilter);
            }

            if(!s.isEmpty())
            {
                if(sFilter.isEmpty())
                    sFilter = s;
                else
                sFilter = QString("%1 AND %2").arg(sFilter).arg(s);
            }
        }
        qDebug() << "Filtre" << sFilter;

        pM->setFilter(sFilter);        
    }
}

void PBSTableView::onFilterButtonPressed(int iCol)
{
    PBSHeaderView *hdr = static_cast<PBSHeaderView*>(tableWidget()->horizontalHeader());
    if(!hdr)
        return;

    FilterLineEdit *pWidget = hdr->filterWidget(iCol);

    QPoint P(pWidget->geometry().x() + pWidget->geometry().width(), hdr->sectionHeight() + pWidget->geometry().height());

    if(P.x() + pFilterFrame.width() > width())
        P.setX(width() - pFilterFrame.width());

    if(P.y() + pFilterFrame.height() > height())
        P.setY(height() - pFilterFrame.height());

    pFilterFrame.build(this, iCol);

    pFilterFrame.move(mapToGlobal(P));
    pFilterFrame.show();
    pFilterFrame.raise();

    emit filterButtonPressed(iCol);
}

void PBSTableView::cornerButtonPressed()
{
    if(pTableWidget.horizontalHeader()->count() < 1)
        return;

    if(!pTableWidget.model())
        return;

    QObject::disconnect(&customizeListWidget, SIGNAL(itemChanged(QListWidgetItem*)),
                         this, SLOT(changeColumnVisible(QListWidgetItem*)));

    customizeListWidget.clear();
    PBSHeaderView *hdr = static_cast<PBSHeaderView*>(pTableWidget.horizontalHeader());

    QVector<QString> headers(model()->columnCount());
    QVector<bool> visibles(model()->columnCount());

    for(int ix = 0; ix < hdr->count(); ix++)
    {
        if(tItemStateMap[ix].bVisible)
        {
            int x = hdr->visualIndex(ix);
            headers[x] = model()->headerData(ix, Qt::Horizontal).toString();
            visibles[x] = hdr->isSectionHidden(ix);
        }
    }

    for(int ix = 0; ix < hdr->count(); ix++)
    {
        if(tItemStateMap[ix].bVisible)
            customizeListWidget.addItem(headers[ix]);
    }

    int x = 0;
    for(int ix = 0; ix < hdr->count(); ix++)
    {
        if(tItemStateMap[ix].bVisible)
        {
            QListWidgetItem *item = customizeListWidget.item(x++);
            item->setData(PBSTABLEVIEWCOLINDEXROLE, ix);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(visibles[ix] ? Qt::Unchecked : Qt::Checked);
        }
    }

    QObject::connect(&customizeListWidget, SIGNAL(itemChanged(QListWidgetItem*)),
                         this, SLOT(changeColumnVisible(QListWidgetItem*)));

    QPoint P(10, rect().y() + hdr->sectionHeight());

    customizeFrame.move(mapToGlobal(P));
    customizeFrame.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    customizeFrame.show();

    emit onCornerButtonPressed();
}

void PBSTableView::changeColumnVisible(QListWidgetItem *item)
{
    PBSHeaderView *hdr = static_cast<PBSHeaderView*>(pTableWidget.horizontalHeader());

    int ix = item->data(PBSTABLEVIEWCOLINDEXROLE).toInt();
    int i = hdr->logicalIndex(ix); // customizeListWidget.row(item));

    if(i < 0)
        return;

    if(item->checkState() == Qt::Checked)
    {
        tItemStateMap[ix].bHidden = true;
        if(hdr->isSectionHidden(i))
        {
            hdr->showSection(i);
        }
    }
    else
    {
        tItemStateMap[ix].bHidden = false;
        if(!hdr->isSectionHidden(i))
        {
            hdr->hideSection(i);
        }
    }

    hdr->adjustPositions();
}

void PBSTableView::loadState()
{
    QString ss = PBSSettings->value(sSettingsName).toString();
    if(ss.isEmpty())
        return;

    QByteArray state = QByteArray::fromHex(ss.toLatin1());
    QDataStream d(&state, QIODevice::ReadOnly);

    QByteArray ba;

    d >> ba;

    pTableWidget.horizontalHeader()->restoreState(ba);

    bool b;

    d >> b;
    setFilterRowVisible(b);

    d >> b;
    setFooterRowVisible(b);

    PBSHeaderView *hdr = static_cast<PBSHeaderView*>(pTableWidget.horizontalHeader());
    QMap<int, ItemState> states = tItemStateMap;
    for(int i = 0; i < hdr->count(); i++)
    {
        int f;
        QString sFmt;

        d >> f;

        if(f > -1)
           setFooterEvent(i, (footerEvent)f);
    }
    PBSDataModel *pM = qobject_cast<PBSDataModel*>(pTableWidget.model());

    hdr->adjustPositions();

    pFooterFrame.calculate();
}

void PBSTableView::saveState()
{
    QByteArray state;
    QDataStream d(&state, QIODevice::WriteOnly);

    d << pTableWidget.horizontalHeader()->saveState();
    d << filterRowVisible();
    d << footerRowVisible();

    PBSHeaderView *hdr = static_cast<PBSHeaderView*>(pTableWidget.horizontalHeader());
    QMap<int, ItemState> states = tItemStateMap;
    for(int i = 0; i < hdr->count(); i++)
    {
        d << states[i].fFooterEvent;
    }

    PBSSettings->setValue(sSettingsName, state.toHex());
}

void PBSTableView::onActSaveExcelXML()
{
    QString sName = PBSFileUtil::getSaveFileName(this, PBSTR("Saklanacak Dosyayı Seçin"));
    if(!sName.isEmpty())
    {
        QFile f(sName);
        f.open(QIODevice::ReadWrite);
        PBSTableViewExcelXMLExporter e(tableWidget(), &f, this);
        e.execute();
        f.close();
    }
}

void PBSTableView::onActSaveWordXML()
{
    QString sName = PBSFileUtil::getSaveFileName(this, PBSTR("Saklanacak Dosyayı Seçin"));
    if(!sName.isEmpty())
    {
        QFile f(sName);
        f.open(QIODevice::ReadWrite);
        PBSTableViewWordXMLExporter e(tableWidget(), &f, this);
        e.execute();
        f.close();
    }
}

void PBSTableView::onActSaveCSV()
{
    QString sName = PBSFileUtil::getSaveFileName(this, PBSTR("Saklanacak Dosyayı Seçin"));
    if(!sName.isEmpty())
    {
        QFile f(sName);
        f.open(QIODevice::ReadWrite);
        PBSTableViewCSVExporter e(tableWidget(), &f, this);
        e.execute();
        f.close();
    }
}

void PBSTableView::onActSaveHTML()
{
    QString sName = PBSFileUtil::getSaveFileName(this, PBSTR("Saklanacak Dosyayı Seçin"));
    if(!sName.isEmpty())
    {
        QFile f(sName);
        f.open(QIODevice::ReadWrite);
        PBSTableViewHTMLExporter e(tableWidget(), &f, this);
        e.execute();
        f.close();
    }
}

void PBSTableView::onActSaveExcel()
{
    QString sName = PBSFileUtil::getSaveFileName(this, PBSTR("Saklanacak Dosyayı Seçin"));
    if(!sName.isEmpty())
    {
        PBSTableViewXLSXExporter e(tableWidget(), sName, this);
        e.execute();
    }
}

void PBSTableView::onActSaveXML()
{
    QString sName = PBSFileUtil::getSaveFileName(this, PBSTR("Saklanacak Dosyayı Seçin"));
    if(!sName.isEmpty())
    {
        QFile f(sName);
        f.open(QIODevice::ReadWrite);
        PBSTableViewXMLExporter e(tableWidget(), &f, this);
        e.execute();
        f.close();
    }
}

void PBSTableView::onActSavePDF()
{
    QString sName = PBSFileUtil::getSaveFileName(this, PBSTR("Saklanacak Dosyayı Seçin"));
    if(!sName.isEmpty())
    {
        PBSTableViewPDFExporter e(tableWidget(), sName, this);
        e.execute();
    }
}

void PBSTableView::onActSendToPrinter()
{
    QPrinter printer(QPrinter::ScreenResolution);
    printer.setPaperSize(QPrinter::A4);
    printer.setFullPage(false);

    printer.setOrientation(QPrinter::Portrait);
    printer.setPageMargins(0, 0, 0, 0, QPrinter::Unit::Millimeter);
    printer.setResolution(96); // QApplication::screens().at(0)->logicalDotsPerInch());

    QPrintPreviewDialog d(&printer, reinterpret_cast<QWidget*>(parent()));    
    d.setWindowFlags(Qt::Window | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);

    connect(&d, SIGNAL(paintRequested(QPrinter *)), this, SLOT(print(QPrinter *)));

    d.exec();
}

void PBSTableView::print(QPrinter *prn)
{
    QPainter painter;

    painter.begin(prn);

    PBSTablePrinter p(&painter, tableWidget(), this);
    p.setPrinter(prn);

    p.setHeadersFont(tableWidget()->horizontalHeader()->font());
    p.setContentFont(font());

    p.execute();
}

bool PBSTableView::askForDelete()
{
    return bAskForDelete;
}

bool PBSTableView::canAddRow()
{
    return pTableWidget.canAddRow();
}

void PBSTableView::setCanAddRow(bool b)
{
    pTableWidget.setCanAddRow(b);
}

void PBSTableView::setAskForDelete(bool b)
{
    bAskForDelete = b;
}

bool PBSTableView::canDeleteRow()
{
    return pTableWidget.canDeleteRow();
}

void PBSTableView::setCanDeleteRow(bool b)
{
    pTableWidget.setCanDeleteRow(b);
}

bool PBSTableView::AskForDelete()
{
    if(!bAskForDelete)
        return true;

    return QMessageBox::question(this, PBSTR("Danışma"), PBSTR("Kayıt Silinsin mi?"),
                              QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes;
}

void PBSTableView::onDeleteRecord()
{
    qDebug() << "Deleting" << canDeleteRow();
    if(pModel && canDeleteRow() && AskForDelete())
    {
        qDebug() << "Deleting";
        int r = currentRow(), c = currentColumn();
        pModel->removeRow(r);
        moveTo(r, c);
    }
}
