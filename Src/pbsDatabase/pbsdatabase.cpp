#include "pbsdatabase.h"

PBSDatabase::PBSDatabase(const QString connectName, const QString driver, const QString dbName,
                         const QString userName, QString password, const QString hostName, const int port) :
    QObject(Q_NULLPTR),
    pPrecisionPolicy(QSql::LowPrecisionDouble),
    sConnectionName(connectName),
    sDriver(driver),
    sDBName(dbName),
    sUserName(userName),
    sPassword(password),
    sHostName(hostName),
    iPort(port)
{
    if(!pbsCheckDBDriver(sDriver))
        qDebug() << "Database Driver " << sDriver << " Bulunamadı";

    pDatabase.setDatabaseName(sDBName);
}

PBSDatabase::~PBSDatabase()
{
    close();
}

void PBSDatabase::setConnectName(const QString& s)
{
    sConnectionName = s;
}

void PBSDatabase::setDriver(const QString& s)
{
    sDriver = s;

    if(!pbsCheckDBDriver(sDriver))
        qDebug() << "Database Driver " << sDriver << " Bulunamadı";
}

void PBSDatabase::setDBName(const QString& s)
{
    sDBName = s;
}

void PBSDatabase::setUserName(const QString& s)
{
    sUserName = s;
}

void PBSDatabase::setPassword(const QString& s)
{
    sPassword= s;
}

void PBSDatabase::setHostName(const QString& s)
{
    sHostName = s;
}

void PBSDatabase::setPort(const int i)
{
    iPort = i;
}

bool PBSDatabase::checkDriver()
{
    return pbsCheckDBDriver(sDriver);
}

QString PBSDatabase::lastError() const
{
    if(!pDatabase.isValid())
        return "Data Açma Hatası";

    QSqlError e = pDatabase.lastError();
    if(e.isValid() && e.type() != QSqlError::NoError)
    {
        return e.text();
    }
    return "";
}

bool PBSDatabase::open()
{
    close();

    pDatabase = pbsOpenDB(sConnectionName, sDriver, sDBName, sUserName, sPassword, sHostName, iPort);

    if(!pDatabase.isValid())
    {
        qDebug() << "Database Oluşturulamadı";
        return false;
    }

    QSqlError e = pDatabase.lastError();

    if(!e.isValid() && e.type() != QSqlError::NoError)
    {
        pbsShowLastDBError(Q_NULLPTR, pDatabase);
        return false;
    }

    pDatabase.setNumericalPrecisionPolicy(pPrecisionPolicy);

    return true;
}

void PBSDatabase::close()
{
    if(pDatabase.isValid())
    {
        if(pDatabase.isOpen())
            pDatabase.close();

        if(!sConnectionName.trimmed().isEmpty())
            QSqlDatabase::removeDatabase(sConnectionName);
    }
}

bool PBSDatabase::execSQL(const QString &sql, bool bUseTransact)
{
    if(!pDatabase.isValid())
        return false;

    if(bUseTransact)
        pDatabase.transaction();

    QSqlQuery q(pDatabase);
    bool b = q.prepare(sql) ? q.exec() : false;

    if(b && bUseTransact)
        b = b ? pDatabase.commit() : pDatabase.rollback();

    if(!b)
    {
        QSqlError e = q.lastError();
        if(e.isValid() && e.type() != QSqlError::NoError)
        {
            qApp->processEvents();
            qDebug() << "Hata " << q.lastError().text();
            return false;
        }
    }

    qApp->processEvents();
    return true;
}
