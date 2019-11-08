#ifndef PBSDATABASE_H
#define PBSDATABASE_H

#include <QObject>
#include <QtSql>

#include "pbsdbtools.h"

class PBSDatabase : public QObject
{
    Q_OBJECT
public:
    explicit PBSDatabase(const QString connectName, const QString driver, const QString dbName = "", const QString userName = "", QString password = "", const QString hostName = "", const int port = -1);
    ~PBSDatabase();

    const QString connectName() { return sConnectionName; }
    void setConnectName(const QString& s);

    const QString driver() { return sDriver; }
    void setDriver(const QString& s);

    const QString dbName() { return sDBName; }
    void setDBName(const QString& s);

    const QString userName() { return sUserName; }
    void setUserName(const QString& s);

    const QString password() { return sPassword; }
    void setPassword(const QString& s);

    const QString hostName() { return sHostName; }
    void setHostName(const QString& s);

    int port() { return iPort; }
    void setPort(const int i);

    void setNumericalPrecisionPolicy(QSql::NumericalPrecisionPolicy p) { pPrecisionPolicy = p; }
    QSql::NumericalPrecisionPolicy	numericalPrecisionPolicy() { return pPrecisionPolicy; }

    QSqlDatabase &SQLDatabase() { return pDatabase; }

    QString lastError() const;

    bool checkDriver();

    Q_INVOKABLE bool open();
    Q_INVOKABLE void close();

    Q_INVOKABLE bool execSQL(const QString &, bool = false);
signals:

public slots:

private slots:
protected:
    QSql::NumericalPrecisionPolicy pPrecisionPolicy;
    QSqlDatabase pDatabase;
    QString sConnectionName, sDriver, sDBName, sUserName, sPassword, sHostName;
    int iPort;
};

#endif // PBSDATABASE_H
