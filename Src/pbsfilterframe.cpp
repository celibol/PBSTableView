#include <pbsfilterframe.h>
#include <pbstools.h>
#include <pbstableview.h>

PBSFilterFrame::PBSFilterFrame(QWidget *parent) :
    QFrame(parent, Qt::Popup | Qt::Window),
    pTableView(Q_NULLPTR),
    pModel(Q_NULLPTR),
    proxyModel(Q_NULLPTR),
    currentCol(-1)
{
    setObjectName("PBSFILTERFRAME");
    setFrameShape(QFrame::StyledPanel);

    resize(200, 350);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    buildLayout();
}

PBSFilterFrame::~PBSFilterFrame()
{
    if(pModel)
        delete pModel;
}

void PBSFilterFrame::deleteChildren()
{
    QListIterator<QObject *> i(children());
    while(i.hasNext())
    {
        QObject *obj = i.next();

        if(obj)
            delete obj;

        obj = Q_NULLPTR;
    }

    if(proxyModel != nullptr)
        delete proxyModel;

    if(pModel != nullptr)
        delete pModel;
}

void PBSFilterFrame::showEvent(QShowEvent*)
{
    QListIterator<QObject *> i(children());
    while(i.hasNext())
    {
        QObject *obj = i.next();
        if(obj->inherits("QLineEdit"))
        {
            QLineEdit *pEdit = reinterpret_cast<QLineEdit*>(obj);
            if(pEdit)
                pEdit->setText("");
        }
        else if(obj->inherits("QCheckBox"))
        {
            QCheckBox *pCheckBox = reinterpret_cast<QCheckBox*>(obj);
            if(pCheckBox)
                pCheckBox->setChecked(false);
        }
    }
}

void PBSFilterFrame::buildLayout()
{
    qDebug() << "Build Layout 1";
    deleteChildren();

    qDebug() << "Build Layout 2";

    QVBoxLayout *pLayout = new QVBoxLayout();

    pLayout->setAlignment(Qt::AlignTop);
    pLayout->setContentsMargins(-1, -1, -1, -1);
    pLayout->setSpacing(0);

    QPushButton *pClearButton = new QPushButton();
    pClearButton->setIcon(QIcon(":/Resource/clear.png"));
    pClearButton->setFixedHeight(25);
    pClearButton->setText(PBSTR("Hepsi"));
    connect(pClearButton, SIGNAL(clicked()), SLOT(clearFilter()));

    pLayout->addWidget(pClearButton);

    QPushButton *pEmptyButton = new QPushButton();
    pEmptyButton->setIcon(QIcon(":/Resource/bos.png"));
    pEmptyButton->setFixedHeight(25);
    pEmptyButton->setText(PBSTR("Boş Olanlar"));
    connect(pEmptyButton, SIGNAL(clicked()), SLOT(emptyFilter()));
    pLayout->addWidget(pEmptyButton);

    QPushButton *pNotEmptyButton = new QPushButton();
    pNotEmptyButton->setIcon(QIcon(":/Resource/dolu.png"));
    pNotEmptyButton->setFixedHeight(25);
    pNotEmptyButton->setText(PBSTR("Dolu Olanlar"));
    connect(pNotEmptyButton, SIGNAL(clicked()), SLOT(nonEmptyFilter()));
    pLayout->addWidget(pNotEmptyButton);

    pLayout->addItem(new QSpacerItem(5, 5));

    QLineEdit *pFilterEdit = new QLineEdit();
    pFilterEdit->setFixedHeight(20);
    pFilterEdit->setPlaceholderText(PBSTR("Filtre"));
    pLayout->addWidget(pFilterEdit);

    connect(pFilterEdit, SIGNAL(textChanged(const QString&)), this, SLOT(filterEditChanged(const QString)));
    pLayout->addItem(new QSpacerItem(5, 5));

    columnListWidget = new QListView();
    columnListWidget->setFixedHeight(200);
    columnListWidget->setObjectName("PBSColumnListWidget");
    columnListWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    columnListWidget->setResizeMode(QListView::Adjust);
    pLayout->addWidget(columnListWidget);

    selCB = new QCheckBox();
    selCB->setText(PBSTR("Hepsini Seç/Bırak"));
    connect(selCB, SIGNAL(stateChanged(int)), this, SLOT(selectChanged(int)));
    pLayout->addWidget(selCB);

    QHBoxLayout *pHLayout = new QHBoxLayout();
    pHLayout->setAlignment(Qt::AlignRight);
    pHLayout->setContentsMargins(0, 10, 0, 0);

    QPushButton *pOkButton = new QPushButton();
    pOkButton->setIcon(QIcon(":/Resource/ok.png"));
    pOkButton->setFixedHeight(25);
    pOkButton->setText(PBSTR("Uygula"));
    connect(pOkButton, SIGNAL(clicked()), SLOT(applyFilter()));
    pHLayout->addWidget(pOkButton);

    pHLayout->addItem(new QSpacerItem(15, 5));

    QPushButton *pCancelButton = new QPushButton();
    pCancelButton->setIcon(QIcon(":/Resource/cancel.png"));
    pCancelButton->setFixedHeight(25);
    pCancelButton->setText(PBSTR("İptal"));
    connect(pCancelButton, SIGNAL(clicked()), SLOT(hide()));
    pHLayout->addWidget(pCancelButton);

    pLayout->addLayout(pHLayout);

    setLayout(pLayout);
}

void PBSFilterFrame::build(PBSTableView *p, int iCol)
{
    if(!p)
        return;
    if(!p->tableWidget()->horizontalHeader())
        return;

    if(iCol > p->tableWidget()->horizontalHeader()->count() - 1)
        return;

    if(!p->tableWidget()->model())
        return;

    if(!p->tableWidget()->model()->inherits("PBSDataModel"))
        return;

    pTableView = p;
    PBSDataModel *pSourceModel = static_cast<PBSDataModel*>(p->tableWidget()->model());

    QString fldName(pSourceModel->record().field(iCol).name()),
            srcSql(pSourceModel->buildQuery(false)),
            orderBy(pSourceModel->orderBy().isEmpty() ? "" : QString("ORDER BY %1").arg(pSourceModel->orderBy()));

    QString sql(QString("Select DISTINCT(%1) FROM(%2) AS _PBS_DISTINCT_QUERY___ %3").arg(fldName).arg(srcSql).arg("")); // arg(orderBy));

    currentFieldName = fldName;
    currentCol = iCol;

    if(pModel)
        delete pModel;

    pModel = new PBSDataModel(pSourceModel->parent(), pSourceModel->database(),
             sql, pSourceModel->where(), pSourceModel->groupBy(), fldName);

    pModel->setDefaultItemFlag(ifCheckBox);
    pModel->openQuery();

    tFilterItemStateMap[0].copyFrom(p->ItemStateMap()[iCol]);
    pModel->setItemStateMap(&tFilterItemStateMap);

    proxyModel = new QSortFilterProxyModel(pModel->parent());
    proxyModel->setDynamicSortFilter(false);
    proxyModel->setSourceModel(pModel);

    columnListWidget->setModel(proxyModel);
}

void PBSFilterFrame::clearFilter()
{
    if(pTableView)
    {
        pTableView->setItemFilterBoxFilter(currentCol, "");
        pTableView->reOpenQuery();
    }

    hide();
}

void PBSFilterFrame::emptyFilter()
{
    if(pTableView)
    {
        QString f = QString("COALESCE(%1, '') = ''").arg(currentFieldName);

        pTableView->setItemFilterBoxFilter(currentCol, f);
        pTableView->reOpenQuery();
    }

    hide();
}

void PBSFilterFrame::nonEmptyFilter()
{
    if(pTableView)
    {
        QString f = QString("COALESCE(%1, '') <> ''").arg(currentFieldName);

        pTableView->setItemFilterBoxFilter(currentCol, f);
        pTableView->reOpenQuery();
    }
    hide();
}

void PBSFilterFrame::filterEditChanged(const QString &f)
{
    QRegExp regExp(f, Qt::CaseInsensitive, QRegExp::RegExp);
    proxyModel->setFilterRegExp(regExp);
}

void PBSFilterFrame::selectChanged(int)
{
    proxyModel->blockSignals(true);

    QModelIndexList chL = pModel->checkedList();
    for(int i = 0; i < proxyModel->rowCount(); i++)
    {
        QModelIndex mi = proxyModel->index(i, 0);
        proxyModel->setData(mi, selCB->checkState(), Qt::CheckStateRole);
        if(selCB->checkState() == Qt::Checked && !chL.contains(mi))
            chL.append(mi);
        else if(selCB->checkState() == Qt::Unchecked && chL.contains(mi))
            chL.removeAt(chL.indexOf(mi));

    }
    proxyModel->blockSignals(false);

    proxyModel->invalidate();
}

void PBSFilterFrame::applyFilter()
{
    if(!pModel)
        return;

    if(!pTableView)
        return;

    QSqlDriver *driver = pModel->database().driver();

    QSqlField field;
    field.setType(pModel->record().field(0).type());

    QModelIndexList chL = pModel->checkedList();

    QString sFilter = "";
    for (int i = 0; i < chL.size(); i++)
    {
        field.setValue(chL[i].data());
        QString f = driver->formatValue(field);
        if(sFilter.isEmpty())
            sFilter = f;
        else
            sFilter.append(QString(", %1").arg(f));

    }
    QString sResFilter(QString("%1 IN (%2)").arg(currentFieldName).arg(sFilter));

    pTableView->setItemFilterBoxFilter(currentCol, sResFilter);
    pTableView->reOpenQuery();

    hide();
}
