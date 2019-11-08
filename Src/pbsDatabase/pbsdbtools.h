#ifndef PBSDBTOOLS_H
#define PBSDBTOOLS_H

#include <QtSql>
#include <QIcon>

bool pbsCheckDBDriver(const QString drvName);
void pbsShowLastDBError(QWidget *parent, const QSqlDatabase db, QString headerIcon = "", QString footerIcon = "");
const QSqlDatabase &pbsOpenDB(const QString sConnectionName, const QString sDriver, QString dbName, const QString sUser = "", QString sPass = "", const QString hostName = "", const int iPort = -1);
bool pbsHaveDBError(QSqlDatabase *);

#endif // PBSDBTOOLS_H
