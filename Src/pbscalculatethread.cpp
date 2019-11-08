#include "pbscalculatethread.h"
#include <QSqlRecord>
#include <QSqlQuery>
#include <KoOdfNumberStyles.h>

#include <PBSDataModel.h>
#include <pbstools.h>
#include <pbsdbtools.h>

PBSCalculateThread::PBSCalculateThread(QObject *p) :
    QThread(p)
{

}

QString PBSCalculateThread::buildCol(int iCol)
{
    if(!pTableView) return "";
    PBSDataModel *pModel = static_cast<PBSDataModel*>(pTableView->model());
    if(!pModel) return "";

    QString sQuery = "";

    QMap<int, ItemState> ism = pTableView->ItemStateMap();

    ItemState st = ism[iCol];
    QString sCol = pModel->record().fieldName(iCol);
    if(!sCol.isEmpty())
    {
        if(st.footerLabel)
        {
            if(st.fFooterEvent == skCount)
                return QString("COUNT(*) AS PBS_COUNT_%2").arg(iCol);
            else if(st.fFooterEvent == skSum)
                return QString("SUM(%1) AS PBS_SUM_%2").arg(sCol).arg(iCol);
            else if(st.fFooterEvent == skMin)
                return QString("MIN(%1) AS PBS_MIN_%2").arg(sCol).arg(iCol);
            else if(st.fFooterEvent == skMax)
                return QString("MAX(%1) AS PBS_MAX_%2").arg(sCol).arg(iCol);
            else if(st.fFooterEvent == skAvg)
                return QString("AVG(%1) AS PBS_AVG_%2").arg(sCol).arg(iCol);
        }
    }

    return "";
}

void PBSCalculateThread::calculate(int iCol)
{
    sRunQuery = "";

    if(!pTableView) return;
    PBSDataModel *pModel = static_cast<PBSDataModel*>(pTableView->model());
    if(!pModel) return;

    QString sQuery = "";

    QMap<int, ItemState> ism = pTableView->ItemStateMap();
    if(iCol > -1)
    {
        QString Q(buildCol(iCol));
        if(ism[iCol].footerLabel)
        {
            ism[iCol].footerLabel->setText(PBSTR("Hesaplanıyor"));
        }
        sQuery = buildCol(iCol);
    }
    else
    {
        for(int i = 0; i < ism.count(); i++)
        {
            QString Q(buildCol(i));
            if(!Q.isEmpty())
            {
                if(ism[i].footerLabel)
                {
                    ism[i].footerLabel->setText(PBSTR("Hesaplanıyor"));                    
                }

                if(sQuery.isEmpty())
                    sQuery = Q;
                else
                    sQuery += QString(", %1").arg(Q);
            }
        }
    }
    if(!sQuery.isEmpty())
    {
        sRunQuery = QString("SELECT %1 FROM(%2) AS A").arg(sQuery).arg(pModel->buildQuery(false));
        // qDebug() << "Col:" << iCol << sRunQuery;
        run();
    }
    else
        qDebug() << "Calc Thread Query Bos";
}

void PBSCalculateThread::run()
{
    qDebug() << "Calc SQL" << sRunQuery;

    if(sRunQuery.isEmpty()) return;
    if(!pTableView) return;
    PBSDataModel *pModel = static_cast<PBSDataModel*>(pTableView->model());
    if(!pModel) return;

    QSqlQuery q(sRunQuery, pModel->database());

    pbsShowLastDBError(pTableView, pModel->database());

    if(q.next())
    {
        QSqlRecord rec = q.record();

        QMap<int, ItemState> ism = pTableView->ItemStateMap();

        for(int i = 0; i < rec.count(); i++)
        {           
            QString sFld(rec.fieldName(i));
            if(sFld.contains("PBS_COUNT_"))
                sFld.remove(0, 10);
            else if(sFld.contains("PBS_SUM_"))
                sFld.remove(0, 8);
            else if(sFld.contains("PBS_MIN_"))
                sFld.remove(0, 8);
            else if(sFld.contains("PBS_MAX_"))
                sFld.remove(0, 8);
            else if(sFld.contains("PBS_AVG_"))
                sFld.remove(0, 8);

            bool b;
            int iCol = sFld.toInt(&b);

            // qDebug() << iCol << rec.fieldName(i) << rec.value(i);

            if(b)
            {
                ItemState its = ism[iCol];
                its.footerValue =  rec.value(i);
                if(its.footerLabel)
                {
                    double d = rec.value(i).toDouble();
                    QString s;
                    // ism[iCol].footerLabel->setText(s.sprintf("%.02f", d));  // rec.value(i).toString());
                    its.footerLabel->setAlignment(ism[iCol].aAlignFooter);
                    its.footerLabel->setText(KoOdfNumberStyles::formatNumber(d, ism[iCol].sFooterFormat));
                }
                pTableView->setItemStateMap(iCol, its);
            }
        }
        emit valueChanged();
    }
}
