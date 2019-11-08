#include "pbsdatamodel.h"

#include <pbstools.h>
#include <pbstableview.h>
#include <KoOdfNumberStyles.h>

class PBSDataModelPrivate
{
public:
    QModelIndexList lCheckedList;

    PBSTableView *pTableView;

    PBSDataModelPrivate(QObject *parent)
    {
        if(parent->inherits("PBSTableView"))
        {
             pTableView = static_cast<PBSTableView*>(parent);
        }
    }
    ~PBSDataModelPrivate() {}
};

PBSDataModel::PBSDataModel(QObject *parent, const QSqlDatabase &dbc,
                           QString sSelect, QString sWhere, QString sGroupBy, QString sOrderBy) :
    QSqlTableModel(parent),
    sqlDB(dbc),
    qSelect(sSelect),
    qWhere(sWhere),
    qGroupBy(sGroupBy),
    qOrderBy(sOrderBy),
    mfDefaultFlag(ifNone),
    pItemStateMap(Q_NULLPTR),
    d(new PBSDataModelPrivate(parent))
{
    setEditStrategy(QSqlTableModel::OnFieldChange);

    if(parent->inherits("PBSTableView"))
    {
         pItemStateMap = &(static_cast<PBSTableView*>(parent)->ItemStateMap());
    }
}

PBSDataModel::~PBSDataModel()
{
    PBSFreeAndNull(d);
}

QModelIndexList PBSDataModel::checkedList()
{
    return d->lCheckedList;
}

void PBSDataModel::setQuery(const QSqlDatabase &dbc, QString sSelect, QString sWhere, QString sGroupBy, QString sOrderBy)
{
    sqlDB = dbc;
    qSelect = sSelect;
    qWhere = sWhere;
    qGroupBy = sGroupBy;
    qOrderBy = sOrderBy;
}

QString PBSDataModel::buildQuery(bool bAddOrderBy)
{
    if (qSelect.isEmpty() || (!sqlDB.isValid()))
        return "";

    QString query = qSelect;

    QString sW = qWhere;
    if(!qFilter.isEmpty())
    {
        if(sW.isEmpty())
            sW = qFilter;
        else
            sW.append(" AND " + qFilter);
    }

    if(!sW.isEmpty())
        query.append(" WHERE " + sW);

    if(!qGroupBy.isEmpty())
        query.append(" GROUP BY " + qGroupBy);

    if(bAddOrderBy && !qOrderBy.isEmpty())
        query.append(" ORDER BY " + qOrderBy);

    return query;
}

void PBSDataModel::openQuery()
{
    QString q = buildQuery();

    beginResetModel();

    clear();
    QSqlTableModel::query().clear();

    setTable(sUpdateTableName);

// * Signal = modelAboutToBeReset
    PBSTableView *p = static_cast<PBSTableView*>(parent());
    if(p)
        p->tableWidget()->reset();

    QSqlQueryModel::setQuery(q, sqlDB);


// *  Signal = modelReset
    if(p)
        p->tableWidget()->viewport()->update();

    endResetModel();

    emit dataChanged(QModelIndex(), QModelIndex());
}

void PBSDataModel::setSort(int column, Qt::SortOrder order)
{
    QString s(record().fieldName(column));
    setOrderBy(s);

    sortColumn = column;
    sortOrder = order;

    QString sO(orderBy());
    if(!sO.isEmpty())
    {
        if(order == Qt::DescendingOrder)
            sO.append(" DESC");
        setOrderBy(sO);
    }
}

void PBSDataModel::sort(int column, Qt::SortOrder order)
{
    if ((sortColumn != column) || (sortOrder != order))
    {
        setSort(column, order);
        openQuery();
    }
}

Qt::ItemFlags PBSDataModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QSqlQueryModel::flags(index);

    ItemState its  = pItemStateMap ? (*pItemStateMap)[index.column()] : ItemState();

    if(!its.bSelectable)
        flags &= ~Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    else if(its.bEditable)
        flags |=  Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
    else
        flags &= ~Qt::ItemIsSelectable | Qt::ItemIsEnabled;

    if(mfDefaultFlag == ifCheckBox)
        flags |= Qt::ItemIsUserCheckable;
    else
        flags &= ~Qt::ItemIsUserCheckable;

    return flags;
}

QString PBSDataModel::formatValue(QModelIndex idx, QVariant value, QString fmt) const
{
    switch(value.type())
    {
        case QVariant::Bool: break;

        case QVariant::Int:
        case QVariant::UInt:
        case QVariant::LongLong:
        case QVariant::ULongLong:
        case QVariant::Double:
            return KoOdfNumberStyles::formatNumber(value.toDouble(), fmt);

        case QVariant::String: return value.toString();

        case QVariant::Date:
        case QVariant::Time:
        case QVariant::DateTime:
        {            
            QDateTime dt(value.toDateTime());
            return dt.toString(fmt);
        }

        default:
            return value.toString();
    }
    return "";
}

QVariant PBSDataModel::data(const QModelIndex &idx, int role) const
{
    if(!idx.isValid())
        return false;

    if(mfDefaultFlag == ifCheckBox && role == Qt::CheckStateRole)
    {
        return d->lCheckedList.contains(idx) ? Qt::Checked : Qt::Unchecked;
    }

    if(role == Qt::TextAlignmentRole)
    {
        const ItemState its = pItemStateMap ? (*pItemStateMap)[idx.column()] : ItemState();

        return (int) its.aAlignColumn;
    }
    else if(role == Qt::DisplayRole)
    {
        QVariant value = data(idx, Qt::EditRole);
        const ItemState its = pItemStateMap ? (*pItemStateMap)[idx.column()] : ItemState();

        if(its.sFormat.isEmpty())
            return value;

        return QVariant::fromValue(formatValue(idx, value, its.sFormat));
    }

    return QSqlTableModel::data(idx, role);
}

bool PBSDataModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(role == Qt::CheckStateRole)
    {
        if(value.toInt() == Qt::Checked)
            d->lCheckedList.append(index);
        else
            d->lCheckedList.removeAt(d->lCheckedList.indexOf(index));

        return true;
    }

    if(role == Qt::EditRole)
    {
//        if(tableName().isEmpty())
//            setTable(sUpdateTableName);

        bool b = QSqlTableModel::setData(index, value, Qt::EditRole);

        return b;
    }

    return QSqlTableModel::setData(index, value, role);
}

QVariant PBSDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Vertical)
    {
        switch(role)
        {
            case Qt::SizeHintRole  : return QSize(32, 16);
            case Qt::DisplayRole   : return "";
        }
    }

    const ItemState its = pItemStateMap ? (*pItemStateMap)[section]: ItemState();
    if(!its.sCaption.isEmpty())
        return its.sCaption;

    if(section == 1 && role == Qt::DisplayRole)
        qDebug() << section << "Header Data" << QSqlTableModel::headerData(section, orientation, role) << role;
    return QSqlTableModel::headerData(section, orientation, role);
}

QSqlRecord PBSDataModel::newRecord()
{
    QSqlRecord r(record());

    r.clearValues();

    return r;
}

bool PBSDataModel::haveEmptyRow()
{
    QSqlRecord nr(newRecord());
    QSqlRecord lr(record(rowCount() - 1));
    bool bHaveEmpty = true;
    for(int i = 0; i < nr.count() - 1; i++)
    {
        if(nr.field(i).name().compare(sKeyFieldName, Qt::CaseInsensitive) != 0)
        {
            QVariant nv = database().driver()->formatValue(nr.field(i)),
                     lv = database().driver()->formatValue(lr.field(i));

            QString ns = nv.toString().toUpper(),
                    ls = lv.toString().toUpper();

            if(ns == "NULL")
                ns = "";

            if(ls == "NULL")
                ls = "";

            if(ns != ls)
            {
                bHaveEmpty = false;
                break;
            }
        }
    }
    return bHaveEmpty;
}

void PBSDataModel::insertEmptyRow()
{
    if(!haveEmptyRow())
    {
        QSqlTableModel::EditStrategy es = editStrategy();
        setEditStrategy(QSqlTableModel::OnRowChange);

        insertRowIntoTable(newRecord());

        setEditStrategy(es);

        QModelIndex mi = index(rowCount(), 0);
        emit dataChanged(mi, mi);
    }
}

bool PBSDataModel::insertRowIntoTable(const QSqlRecord &values)
{
    setTable(sUpdateTableName);

    QString sql(createInsertQuery(values));

    QSqlQuery q;
    if(!q.prepare(sql))
        return false;

    bool b = q.exec();
    if(b)
    {
        select();
        while(canFetchMore())
            fetchMore();
    }

    return b;
}

int PBSDataModel::rowCount(const QModelIndex &parent) const
{
    return QSqlTableModel::rowCount(parent);
}

bool PBSDataModel::isInsertableField(QString n)
{
    QSqlRecord r(record());

    QSqlField fld = r.field(n);

    return fld.isValid() ? !fld.isReadOnly() : false;
}

bool PBSDataModel::isUpdatableField(QString n)
{
    QSqlRecord r(record());

    QSqlField fld = r.field(n);

    return fld.isValid() ? !fld.isReadOnly() : false;
}

QString PBSDataModel::createInsertQuery(QSqlRecord r)
{
    QString sIns(""), sVal("");

    for(int i = 0; i < r.count(); i++)
    {
        QString f, v;

        QSqlField fld = r.field(i);

        if(fld.isReadOnly())
            continue;

        if(!isInsertableField(fld.name()))
            continue;

        f = fld.name();
        v = database().driver()->formatValue(fld);

        if(sIns.isEmpty())
            sIns = f;
        else
            sIns += ", " + f;

        if(sVal.isEmpty())
            sVal = v;
        else
            sVal += ", " + v;
    }

    return sIns.isEmpty() ? "" : QString("INSERT INTO %1(%2) VALUES(%3)").arg(sUpdateTableName).arg(sIns).arg(sVal);
}

QString PBSDataModel::createUpdateQuery(QSqlRecord)
{
    qDebug() << "Create Update Query";
}

void PBSDataModel::setItemStateMap(itemStateMap *ism)
{
    pItemStateMap = ism;
}

bool PBSDataModel::removeRows(int row, int count, const QModelIndex &parent)
{
    qDebug() << "Remove" << row << count;

    if(sUpdateTableName.isEmpty())
    {
        qDebug() << "PBSDataModel::remove Key Invalid Table Name";
        return false;
    }

    if(sKeyFieldName.isEmpty())
    {
        qDebug() << "PBSDataModel::remove Key Field Name Empty";
        return false;
    }

    if(row > rowCount())
    {
        qDebug() << "PBSDataModel::remove Invalid Row";
        return false;
    }

    for(int i = row; i < row + count; i++)
    {
        if(row <= rowCount())
        {
            QModelIndex mi = index(i, fieldIndex(sKeyFieldName));
            QVariant sKeyData = data(mi, Qt::EditRole);

            QSqlField sqf = record().field(mi.column());
            sqf.setValue(sKeyData);

            QString sSQL(QString("DELETE FROM %1 WHERE %2 = %3").arg(sUpdateTableName).arg(sKeyFieldName).arg(database().driver()->formatValue(sqf)));
            qDebug() << "PBSDataModel::remove Key" << sSQL;

            QSqlQuery q;
            if(!q.prepare(sSQL))
                return false;

            if(!q.exec())
                return false;
        }
    }
    select();
    // while(canFetchMore())
    //    fetchMore();

    QModelIndex mi = index(row, fieldIndex(sKeyFieldName));
    emit dataChanged(mi, mi);

    return true;
}
