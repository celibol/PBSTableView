#ifndef PBSTABLEPRINTER_H
#define PBSTABLEPRINTER_H

#include <QObject>
#include <QTableView>
#include <QPen>
#include <QFont>
#include <QAbstractItemModel>
#include <QPainter>
#include <QPrinter>
#include <QPdfWriter>

class PBSTablePrinter : public QObject
{
    Q_OBJECT
public:
    explicit PBSTablePrinter(QPainter *painter, QTableView *table, QObject *parent = nullptr);

    void setPageMargin(int left = 50, int right = 20, int top = 20, int bottom = 20);

    void setHeadersFont(QFont font);
    void setContentFont(QFont font);

    void setPen(QPen p);

    void setPrinter(QPrinter *);
    void setWriter(QPdfWriter *);

    void execute();
signals:

public slots:
protected:
    QTableView *table;

    QPainter *painter;
    QPrinter *printer;
    QPdfWriter *writer;

    QFont headersFont;
    QFont contentFont;

    QPen pen;

    int topMargin;
    int bottomMargin;
    int leftMargin;
    int rightMargin;    

    QString error;

    QVector<double> columnWidth;

    void calcColumnWidth();
    void drawHeader();
    void drawData();
};

#endif // PBSTABLEPRINTER_H
