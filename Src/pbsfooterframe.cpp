#include <pbscalculatethread.h>
#include "pbsfooterframe.h"

#include "pbsheaderview.h"
#include "PBSDataModel.h"

#include <QDebug>
#include <pbstools.h>

PBSFooterFrame::PBSFooterFrame(QWidget* parent, Qt::WindowFlags f) :
    QFrame(parent)
{
    setObjectName("PBSFOOTERFRAME");
    setFrameShape(QFrame::StyledPanel);

    setContextMenuPolicy(Qt::CustomContextMenu);

    pNone  = new QAction(QIcon(":/Resource/Images/fclear.png"), PBSTR("Yok"), this);
    pNone->setCheckable(true);
    connect(pNone, &QAction::triggered, this, &PBSFooterFrame::onNone);

    pCount = new QAction(QIcon(":/Resource/Images/count.png"), PBSTR("Sayaç"), this);
    pCount->setCheckable(true);
    connect(pCount, &QAction::triggered, this, &PBSFooterFrame::onCount);

    pSum   = new QAction(QIcon(":/Resource/Images/sum.png"), PBSTR("Toplam"), this);
    pSum->setCheckable(true);
    connect(pSum, &QAction::triggered, this, &PBSFooterFrame::onSum);

    pMin   = new QAction(QIcon(":/Resource/Images/min.png"), PBSTR("En Küçük"), this);
    pMin->setCheckable(true);
    connect(pMin, &QAction::triggered, this, &PBSFooterFrame::onMin);

    pMax   = new QAction(QIcon(":/Resource/Images/max.png"), PBSTR("En Büyük"), this);
    pMax->setCheckable(true);
    connect(pMax, &QAction::triggered, this, &PBSFooterFrame::onMax);

    pAvg   = new QAction(QIcon(":/Resource/Images/avg.png"), PBSTR("Averaj"), this);
    pAvg->setCheckable(true);
    connect(pAvg, &QAction::triggered, this, &PBSFooterFrame::onAvg);

    pMenu = new QMenu(this);
    pMenu->addAction(pNone);
    pMenu->addAction(pCount);
    pMenu->addAction(pSum);
    pMenu->addAction(pMin);
    pMenu->addAction(pMax);
    pMenu->addAction(pAvg);

    QTimer::singleShot(50, this, SLOT(sendMenuSignal()));

    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showCustomContextMenu(const QPoint&)));

    PBSTableView *v = static_cast<PBSTableView*>(parent);
    if(v)
    {
        pThread = new PBSCalculateThread(this);
        pThread->pTableView = v;
        // connect(pThread, &PBSCalculateThread::valueChanged, this, &PBSFooterFrame::valueChanged);
        connect(pThread, SIGNAL(valueChanged()), this, SLOT(valueChanged()));
    }
}

void PBSFooterFrame::valueChanged()
{
    emit onValueChanged();
}

void PBSFooterFrame::sendMenuSignal()
{
    emit onCreatePopupMenu(pMenu);
}

QMenu *PBSFooterFrame::menu()
{
    return pMenu;
}

PBSFooterFrame::~PBSFooterFrame()
{
    PBSFreeAndNull(pNone);
    PBSFreeAndNull(pCount)
    PBSFreeAndNull(pSum);
    PBSFreeAndNull(pMin);
    PBSFreeAndNull(pMax);
    PBSFreeAndNull(pMenu)

    if(pThread)
    {
        pThread->quit();
        pThread->wait();
        delete pThread;
    }
}

void PBSFooterFrame::showCustomContextMenu(const QPoint &p)
{
    if(pMenu)
    {
        PBSTableView *v = static_cast<PBSTableView*>(parent());
        PBSHeaderView *h = static_cast<PBSHeaderView*>(v->tableWidget()->horizontalHeader());
        PBSHeaderView *vh = static_cast<PBSHeaderView*>(v->tableWidget()->verticalHeader());

        QPoint mp(p);
        mp.setX(mp.x() - v->tableWidget()->verticalHeader()->width());

        int iCol = h->logicalIndexAt(mp);

        QMap<int, ItemState> states = v->ItemStateMap();

        pNone->setChecked(states[iCol].fFooterEvent == skNone);
        pCount->setChecked(states[iCol].fFooterEvent == skCount);
        pSum->setChecked(states[iCol].fFooterEvent == skSum);
        pMin->setChecked(states[iCol].fFooterEvent == skMin);
        pMax->setChecked(states[iCol].fFooterEvent == skMax);
        pAvg->setChecked(states[iCol].fFooterEvent == skAvg);

        QPoint mP = mapToGlobal(p);
        pMenu->popup(mP);
    }
}

int PBSFooterFrame::findCol()
{
    PBSTableView *v = static_cast<PBSTableView*>(parent());
    PBSHeaderView *h = static_cast<PBSHeaderView*>(v->tableWidget()->horizontalHeader());

    QPoint p = mapFromGlobal(pMenu->pos());
    p.setX(p.x() - v->tableWidget()->verticalHeader()->width());

    int iCol = h->logicalIndexAt(p);
    return iCol;
}

void PBSFooterFrame::calculate()
{
    if(pThread)
        pThread->calculate(-1);
}

void PBSFooterFrame::showEvent(QShowEvent*)
{
    // calculate();
}

QSize PBSFooterFrame::sizeHint() const
{
    QSize s = QFrame::sizeHint();

    QFontMetrics fm(font());

    int h = fm.height() + 10;

    s.setHeight(h);

    return s;
}

void PBSFooterFrame::execCol(footerEvent e)
{
    PBSTableView *v = static_cast<PBSTableView*>(parent());
    if(!v) return;

    PBSHeaderView *h = static_cast<PBSHeaderView*>(v->tableWidget()->horizontalHeader());
    if(!h) return;

    QMap<int, ItemState> states = v->ItemStateMap();

    int iCol = findCol();
    v->setFooterEvent(iCol, e);

    if(states[iCol].footerLabel)
    {
        states[iCol].footerLabel->setText("");
        states[iCol].footerLabel->setAlignment(states[iCol].aAlignFooter);
    }

    if(e != skNone && pThread)
        pThread->calculate(iCol);

    emit menuClicked(iCol);
}

void PBSFooterFrame::onNone()
{
    execCol(skNone);
}

void PBSFooterFrame::onCount()
{
    execCol(skCount);
}

void PBSFooterFrame::onSum()
{
    execCol(skSum);
}

void PBSFooterFrame::onMin()
{
    execCol(skMin);
}

void PBSFooterFrame::onMax()
{
    execCol(skMax);
}

void PBSFooterFrame::onAvg()
{
    execCol(skAvg);
}
