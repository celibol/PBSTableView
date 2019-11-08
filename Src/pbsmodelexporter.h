#ifndef PBSTableViewEXPORTER_H
#define PBSTableViewEXPORTER_H

#include <QObject>
#include <QIODevice>
#include <QTextStream>
#include <QtWidgets>
#include <QPrinter>

class PBSTableViewExporter : public QObject
{
    Q_OBJECT
public:
    explicit PBSTableViewExporter(QTableView *table, QIODevice *device, QObject *parent = nullptr);

    void execute();
signals:    

public slots:

protected:
    QIODevice *pDevice;
    QTableView *pSourceTable;

    virtual void executeExport();
    virtual void writeHeader() {}
    virtual void writeLine(int) {}
    virtual void writeFooter() {}
};

class PBSTableViewPDFExporter : public PBSTableViewExporter
{
    Q_OBJECT
public:
    explicit PBSTableViewPDFExporter(QTableView *table, QString fName, QObject *parent = nullptr);
signals:

public slots:

protected:
    QString pdfName;
    void executeExport() Q_DECL_OVERRIDE;
};

class PBSTableViewXLSXExporter : public PBSTableViewExporter
{
    Q_OBJECT
public:
    explicit PBSTableViewXLSXExporter(QTableView *table, QString fName, QObject *parent = nullptr);
signals:

public slots:

protected:
    QString xlsxName;
    void executeExport() Q_DECL_OVERRIDE;
};

class PBSTableViewCSVExporter : public PBSTableViewExporter
{
    Q_OBJECT
public:
    explicit PBSTableViewCSVExporter(QTableView *table, QIODevice *device, QObject *parent = nullptr);
protected:
    void writeHeader() Q_DECL_OVERRIDE;
    void writeLine(int) Q_DECL_OVERRIDE;
};

class PBSTableViewHTMLExporter : public PBSTableViewExporter
{
    Q_OBJECT
public:
    explicit PBSTableViewHTMLExporter(QTableView *table, QIODevice *device, QObject *parent = nullptr);
protected:
    void writeHeader() Q_DECL_OVERRIDE;
    void writeLine(int) Q_DECL_OVERRIDE;
    void writeFooter() Q_DECL_OVERRIDE;
};

class PBSTableViewXMLExporter : public PBSTableViewExporter
{
    Q_OBJECT
public:
    explicit PBSTableViewXMLExporter(QTableView *table, QIODevice *device, QObject *parent = nullptr);
protected:
    QXmlStreamWriter xmlStream;

    void writeHeader() Q_DECL_OVERRIDE;
    void writeLine(int) Q_DECL_OVERRIDE;
    void writeFooter() Q_DECL_OVERRIDE;
};

class PBSTableViewExcelXMLExporter : public PBSTableViewExporter
{
    Q_OBJECT
public:
    explicit PBSTableViewExcelXMLExporter(QTableView *table, QIODevice *device, QObject *parent = nullptr);
protected:
    QXmlStreamWriter xmlStream;
    void writeHeader() Q_DECL_OVERRIDE;
    void writeLine(int) Q_DECL_OVERRIDE;
    void writeFooter() Q_DECL_OVERRIDE;
};

class PBSTableViewWordXMLExporter : public PBSTableViewExporter
{
    Q_OBJECT
public:
    explicit PBSTableViewWordXMLExporter(QTableView *table, QIODevice *device, QObject *parent = nullptr);
protected:
    QXmlStreamWriter xmlStream;
    QMap<int, int> colWidth;
    int imageId;

    void writeFont(const QFont &);
    void writeBackgroundColor(const QBrush &);
    void writeDecoration(const QVariant &);

    void writeHeader() Q_DECL_OVERRIDE;
    void writeLine(int) Q_DECL_OVERRIDE;
    void writeFooter() Q_DECL_OVERRIDE;
};

#endif // PBSTableViewEXPORTER_H
