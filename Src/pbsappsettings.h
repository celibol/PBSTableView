#ifndef PBSAPPSETTINGS_H
#define PBSAPPSETTINGS_H

#include <QSettings>

class PBSAppSettings : public QSettings
{
public:


    PBSAppSettings(const QString &fileName, Format format, QObject *parent = Q_NULLPTR);
};

extern QSettings::Format XmlFormat;
extern PBSAppSettings *PBSSettings;

#endif // PBSAPPSETTINGS_H
