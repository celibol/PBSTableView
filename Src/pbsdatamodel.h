#ifndef PBSDATAMODEL_H
#define PBSDATAMODEL_H

#include <QObject>
#include <QtSql>

#include <pbstableviewtool.h>

class PBSDataModelPrivate;

class PBSDataModel : public QSqlTableModel
{
    Q_OBJECT
public:
    explicit PBSDataModel(QObject *parent = nullptr,
                          const QSqlDatabase &dbc = QSqlDatabase::database(),
                          QString sSelect = "",
                          QString sWhere = "",
                          QString sGroupBy = "",
                          QString sOrderBy = "");

   ~PBSDataModel() override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    // bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool insertRowIntoTable(const QSqlRecord &values) override;

    QSqlDatabase &database() { return sqlDB; }

    void setSelect(QString &sSelect) { qSelect = sSelect; }
    QString selectSQL() { return qSelect; }

    void setWhere(QString &sWhere) { qWhere = sWhere; }
    QString where() { return qWhere; }

    void setGroupBy(QString &sGroupBy) { qGroupBy = sGroupBy; }
    QString groupBy() { return qGroupBy; }

    void setOrderBy(QString &sOrderBy) { qOrderBy = sOrderBy; }
    QString orderBy() { return qOrderBy; }

    void setFilter(QString &sFilter) { qFilter = sFilter; }
    QString Filter() { return qFilter; }

    QString updateTableName() { return sUpdateTableName; }
    void setUpdateTableName(QString s) { sUpdateTableName = s; setTable(s); }

    modelFlag defaultItemFlag() { return mfDefaultFlag; }
    void setDefaultItemFlag(modelFlag f) { mfDefaultFlag = f; }

    QModelIndexList checkedList();

    void setQuery(const QSqlDatabase & dbc = QSqlDatabase::database(),
        QString sSelect = "", QString sWhere = "", QString sGroupBy = "", QString sOrderBy = "");

    QString buildQuery(bool bAddOrderBy = true);
    void openQuery();

    virtual QSqlRecord newRecord();

    virtual bool haveEmptyRow();
    virtual void insertEmptyRow();    

    virtual bool isInsertableField(QString);
    virtual bool isUpdatableField(QString);

    QString createInsertQuery(QSqlRecord);
    QString createUpdateQuery(QSqlRecord);

    void setItemStateMap(itemStateMap *);

    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
public slots:
    virtual void setSort (int column, Qt::SortOrder order) override;
    virtual void sort (int column, Qt::SortOrder order) override;
protected:
    itemStateMap *pItemStateMap;
    QString sKeyFieldName = "",
            sUpdateTableName = "",
            qSelect = "",
            qWhere = "",
            qGroupBy = "",
            qOrderBy = "",
            qFilter = "";

    modelFlag mfDefaultFlag = ifNone;

    int sortColumn = -1;
    Qt::SortOrder sortOrder = Qt::AscendingOrder;

    PBSDataModelPrivate *d;
    QSqlDatabase sqlDB;

    virtual QString formatValue(QModelIndex mi, QVariant value, QString fmt) const;
};

#endif // PBSDATAMODEL_H
