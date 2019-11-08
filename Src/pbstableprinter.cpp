#include <pbstableprinter.h>

#include <QDebug>
#include <QHeaderView>

#include <pbstools.h>

PBSTablePrinter::PBSTablePrinter(QPainter *painter, QTableView *table, QObject *parent) :
    QObject(parent),
    table(table),
    painter(painter)
{
    printer = Q_NULLPTR;
    writer = Q_NULLPTR;

    topMargin = 5;
    bottomMargin = 5;
    leftMargin = 5;
    rightMargin = 5;

    pen = painter->pen();

    headersFont = painter->font();
    contentFont = painter->font();
}

void PBSTablePrinter::setPrinter(QPrinter *p)
{
    printer = p;
    writer = Q_NULLPTR;
}

void PBSTablePrinter::setWriter(QPdfWriter *w)
{
    printer = Q_NULLPTR;
    writer = w;
}

void PBSTablePrinter::setPageMargin(int left, int right, int top, int bottom)
{
    topMargin = top;
    bottomMargin = bottom;
    leftMargin = left;
    rightMargin = right;
}

void PBSTablePrinter::setHeadersFont(QFont f)
{
    headersFont = f;
}

void PBSTablePrinter::setContentFont(QFont f)
{
    contentFont = f;
}

void PBSTablePrinter::setPen(QPen p)
{
    pen = p;
}

void PBSTablePrinter::calcColumnWidth()
{
    double totalWidth = 0.0;
    for(int i = 0; i < table->horizontalHeader()->count(); i++)
    {
        if (!table->horizontalHeader()->isSectionHidden(i))
            totalWidth += table->horizontalHeader()->sectionSize(i);
    }

    for(int i = 0; i < table->horizontalHeader()->count(); i++)
    {
        if (!table->horizontalHeader()->isSectionHidden(i))
            columnWidth << 100.00 * table->horizontalHeader()->sectionSize(table->horizontalHeader()->logicalIndex(i)) / totalWidth;
    }
}

void PBSTablePrinter::drawHeader()
{
    QFontMetrics fm(headersFont);

    painter->save();

    painter->setFont(headersFont);
    qDebug() << "Header Font" << headersFont;
    qDebug() << "Content Font" << contentFont;

    double x = leftMargin;
    int c = 0, h = fm.height() + 10;

    for(int i = 0; i < table->horizontalHeader()->count(); i++)
    {
        if (!table->horizontalHeader()->isSectionHidden(i))
        {
            QString sText(table->model()->headerData(table->horizontalHeader()->logicalIndex(i), Qt::Horizontal, Qt::DisplayRole).toString());

            double w = painter->viewport().width() * columnWidth[c++] / 100.00;
            if(x + w > painter->viewport().width() - leftMargin - rightMargin)
                w = painter->viewport().width() - leftMargin - rightMargin - x;

            painter->setBrush(table->palette().base());
            QRectF rf(x, topMargin, w, h);
            painter->drawRect(rf);

            QRectF tf(x + 10, topMargin, w - 3, h);
            painter->drawText(tf,  Qt::AlignLeft | Qt::AlignVCenter | Qt::TextWordWrap, sText);
            x += w;
        }
    }

    painter->restore();
}

void PBSTablePrinter::drawData()
{
    QFontMetrics fm(headersFont);
    int currY = topMargin + fm.height() + 10;

    painter->save();
    painter->setFont(contentFont);

    QFontMetrics cf(contentFont);

    int defHeight = cf.height() + 10;
    int defSecSize = table->verticalHeader()->defaultSectionSize();

    for(int r = 0; r < table->model()->rowCount(); r++)
    {
        int h = defHeight * (table->verticalHeader()->sectionSize(r) / defSecSize),
                ic = 0;
        double x = leftMargin;

        if (currY + bottomMargin + h >= painter->viewport().height())
        {
            if(printer)
                printer->newPage();
            else if(writer)
                writer->newPage();

            QBrush br = painter->brush();

            drawHeader();

            painter->setBrush(br);

            currY = topMargin + fm.height() + 10;
        }
        for(int c = 0; c < table->horizontalHeader()->count(); c++)
        {
            if (!table->horizontalHeader()->isSectionHidden(c))
            {
                QModelIndex ix = table->model()->index(r, table->horizontalHeader()->logicalIndex(c));

                QString sText(table->model()->data(ix, Qt::DisplayRole).toString());
                double w = painter->viewport().width() * columnWidth[ic++] / 100.00;
                if(x + w > painter->viewport().width() - leftMargin - rightMargin)
                    w = painter->viewport().width() - leftMargin - rightMargin - x;

                QRectF rf(x, currY, w, h);

                if(table->alternatingRowColors() & r % 2 == 1)
                    painter->setBrush(table->palette().alternateBase());
                else
                    painter->setBrush(table->palette().base());

                painter->drawRect(rf);

                int f = table->model()->data(ix, Qt::TextAlignmentRole).toInt();
                QRectF tf(x + 10, currY, w - 20, h);
                painter->drawText(tf,  f | Qt::TextWordWrap, sText);

                x += w;
            }
        }
        currY += h;
    }
    painter->restore();
}

void PBSTablePrinter::execute()
{
    if(table->model())
    {
        QModelIndex mi(table->model()->index(table->model()->rowCount(), 0));
        while(table->model()->canFetchMore(mi))
        {
            QModelIndex mi(table->model()->index(table->model()->rowCount(), 0));
            table->model()->fetchMore(mi);
        }
    }

    calcColumnWidth();

    painter->save();

    painter->setPen(pen);

    drawHeader();
    drawData();

    painter->restore();
}
