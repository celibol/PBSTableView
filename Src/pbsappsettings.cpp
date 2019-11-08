#include "pbsappsettings.h"

#include <QDomDocument>
#include <QTextStream>
#include <QDebug>

bool PBSReadXmlFileForSettings(QIODevice &device, QSettings::SettingsMap &map);
bool PBSWriteXmlFileForSettings(QIODevice &device, const QSettings::SettingsMap &map);

PBSAppSettings *PBSSettings = Q_NULLPTR;
QSettings::Format XmlFormat = QSettings::registerFormat("xml", PBSReadXmlFileForSettings, PBSWriteXmlFileForSettings, Qt::CaseInsensitive);

void PBSProcessWriteKeyForSettings(QDomDocument& doc, QDomElement& domElement, QString key, const QVariant& value )
{
    int slashPos = key.indexOf('/');

    if (slashPos < 0)
    {
        QDomNode node = domElement.appendChild(doc.createElement(key));
        node.appendChild(doc.createTextNode(value.toString()));
//        node.toElement().setAttribute("type", "0");

        return;
    }

    QString groupName = key.left( slashPos );
    QDomElement groupElement;
    QDomNode findedGroupNode = domElement.namedItem( groupName );

    if ( findedGroupNode.isNull() )
    {
        groupElement = doc.createElement( groupName );
        domElement.appendChild( groupElement );        
    }
    else
        groupElement = findedGroupNode.toElement();

    groupElement.setAttribute("type", "1");

    key = key.right( key.size() - slashPos - 1 );

    PBSProcessWriteKeyForSettings( doc, groupElement, key, value );
}

void PBSProcessReadKeyForSettings(QString key, QSettings::SettingsMap &map, QDomElement& domElement )
{
    for (int i = 0; i < domElement.childNodes().count(); ++i)
    {
        QDomElement e = domElement.childNodes().item(i).toElement();
        QString tag = e.tagName();
        if(!e.isNull() && !tag.isEmpty() && e.nodeType() == QDomNode::ElementNode && !e.text().isEmpty())
        {
            if(!e.text().isEmpty() && e.attribute("type", "0") != "1")
            {
                // qDebug() << "Key " << key << " Tag " << qPrintable(e.tagName()) << " Element Text: " << qPrintable(e.text());

                map.insert(key + tag, e.text());
            }

            QString subKey = key + tag + "/";
            QDomElement subElement = domElement.childNodes().item(i).toElement();
            PBSProcessReadKeyForSettings( subKey, map, subElement );
        }
    }
}

bool PBSReadXmlFileForSettings(QIODevice &device, QSettings::SettingsMap &map)
{
    QDomDocument doc("");
    if (!doc.setContent(&device))
        return false;

    QDomNode findedGroupNode = doc.namedItem("PBS");
    if (!findedGroupNode.isNull())
    {
        QDomElement root = findedGroupNode.toElement();
        PBSProcessReadKeyForSettings("", map, root);
    }

    return true;
}

bool PBSWriteXmlFileForSettings(QIODevice &device, const QSettings::SettingsMap &map)
{
    QDomDocument doc("");
    QDomElement root;

    QDomNode findedGroupNode = doc.namedItem("PBS");
    if (!findedGroupNode.isNull())
    {
        root = findedGroupNode.toElement();
    }
    else
    {
        root = doc.createElement("PBS");
        doc.appendChild(root);
    }

    QMapIterator<QString, QVariant> i(map);
    while ( i.hasNext() )
    {
        i.next();

        QString  sKey = i.key();
        QVariant value = i.value();
        PBSProcessWriteKeyForSettings(doc, root, sKey, i.value());
    }

    QDomNode xmlNode = doc.createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-8\"");
    doc.insertBefore(xmlNode, doc.firstChild());

    QTextStream out(&device);
    doc.save(out, QDomNode::EncodingFromDocument);

    return true;
}

PBSAppSettings::PBSAppSettings(const QString &fileName, Format format, QObject *parent) :
    QSettings(fileName, format, parent)
{
}
