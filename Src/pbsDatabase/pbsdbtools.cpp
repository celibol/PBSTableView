#include "pbsdbtools.h"

#ifndef QT_QML_LIB
// #include <pbstaskdialog.h>
#endif

bool pbsCheckDBDriver(const QString drvName)
{
    return QSqlDatabase::drivers().contains(drvName);
}

void pbsShowLastDBError(QWidget *parent, const QSqlDatabase db, QString headerIcon, QString footerIcon)
{
    QSqlError e = db.lastError();
    qDebug() << "Veritabanı Hatası: " << e.type() << " " << e.text();

    if(e.isValid() && e.type() != QSqlError::NoError)
    {
        qDebug() << "Veritabanı Hatası: " << e.type() << " " << e.text();
#ifndef QT_QML_LIB
        // PBSErrorMessage(parent, "Veritabanı Hatası", "Hata:<br><br>" + e.text(), headerIcon, footerIcon);
#else
        Q_UNUSED(parent)
        Q_UNUSED(headerIcon)
        Q_UNUSED(footerIcon)

        qDebug() << "Veritabanı Hatası: " << e.type() << " " << e.text();
#endif
    }
}

bool pbsHaveDBError(QSqlDatabase *db)
{
    if(!db->isValid())
        return true;
    if(!db->isOpen())
        return true;
    if(db->isOpenError())
        return true;
    return db->lastError().type() != QSqlError::NoError;
}

const QSqlDatabase &pbsOpenDB(const QString sConnectionName, const QString sDriver, QString dbName, const QString sUser, QString sPass, const QString hostName, const int iPort)
{
    static QSqlDatabase retDB; // = new QSqlDatabase;
    retDB = QSqlDatabase::addDatabase(sDriver);

    if(retDB.isValid())
    {
        if(!sConnectionName.trimmed().isEmpty())
            QSqlDatabase::connectionNames().append(sConnectionName);

        if(sDriver.indexOf("QODBC") == 0)
        {
            QString sFmt = QString("DRIVER={SQL Server};SERVER=%1;DATABASE=%2;UID=%3;PWD=%4").arg(hostName).arg(dbName).arg(sUser).arg(sPass);
            dbName = sFmt;
        }
        retDB.setPassword(sPass);
        retDB.setUserName(sUser);
        retDB.setHostName(hostName);
        retDB.setDatabaseName(dbName);

        if(iPort != -1)
            retDB.setPort(iPort);

        retDB.open();
    }
    return retDB;
}
