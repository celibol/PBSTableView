#include "mainwindow.h"
#include <QApplication>
#include <pbsappsettings.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QDir dir(QCoreApplication::applicationDirPath());
    QString sName = dir.absolutePath() + QDir::separator() + "PBSSettings.xml";

    qDebug() << sName;

    PBSSettings = new PBSAppSettings(sName, XmlFormat);

    MainWindow w;
    w.show();

    return a.exec();
}
