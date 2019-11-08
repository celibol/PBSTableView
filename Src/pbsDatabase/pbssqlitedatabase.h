#ifndef PBSSQLITEDATABASE_H
#define PBSSQLITEDATABASE_H

#include <QObject>
#include <pbsdatabase.h>

class PBSSQLiteDatabase : public PBSDatabase
{
    Q_OBJECT
public:
    explicit PBSSQLiteDatabase(const QString connectName, const QString dbName);
    explicit PBSSQLiteDatabase(const QString connectName);

signals:

public slots:
};

#endif // PBSSQLITEDATABASE_H
