#include <pbssqlitedatabase.h>

PBSSQLiteDatabase::PBSSQLiteDatabase(const QString connectName, const QString dbName) :
    PBSDatabase (connectName, "QSQLITE", dbName)
{
}

PBSSQLiteDatabase::PBSSQLiteDatabase(const QString connectName) :
    PBSDatabase (connectName, "QSQLITE")
{
}
