#include "PBSModelExporter.h"
#include <pbstools.h>
#include <pbstableprinter.h>
#include <QPrinter>
#include <QPdfWriter>

#include <xlsxdocument.h>
#include <xlsxformat.h>
#include <xlsxworksheet.h>

#include <QDebug>

PBSTableViewExporter::PBSTableViewExporter(QTableView *table, QIODevice *device, QObject *parent) :
    QObject(parent),
    pDevice(device),
    pSourceTable(table)
{

}

void PBSTableViewExporter::execute()
{
    if(!pSourceTable)
        return;

    if(pSourceTable->model())
    {
        QModelIndex mi(pSourceTable->model()->index(pSourceTable->model()->rowCount(), 0));
        while(pSourceTable->model()->canFetchMore(mi))
        {
            QModelIndex mi(pSourceTable->model()->index(pSourceTable->model()->rowCount(), 0));
            pSourceTable->model()->fetchMore(mi);
        }
    }

    executeExport();
}

void PBSTableViewExporter::executeExport()
{
    if(!pDevice)
        return;

    writeHeader();

    for(int i = 0; i < pSourceTable->model()->rowCount() - 1; i++)
        writeLine(i);

    writeFooter();
}

// CSV Exporter
PBSTableViewCSVExporter::PBSTableViewCSVExporter(QTableView *table, QIODevice *device, QObject *parent) :
    PBSTableViewExporter(table, device, parent)
{
}

void PBSTableViewCSVExporter::writeHeader()
{
    if(!pSourceTable)
        return;

    if(!pDevice)
        return;

    QStringList l;

    for(int i = 0; i < pSourceTable->horizontalHeader()->count(); i++)
    {        
        if (!pSourceTable->horizontalHeader()->isSectionHidden(i))
            l << pSourceTable->model()->headerData(pSourceTable->horizontalHeader()->logicalIndex(i), Qt::Horizontal, Qt::DisplayRole).toString();
    }
    QTextStream d(pDevice);
    d << l.join(";") << endl;
}

void PBSTableViewCSVExporter::writeLine(int iRow)
{
    if(!pSourceTable)
        return;

    if(!pDevice)
        return;

    QStringList l;

    for(int i = 0; i < pSourceTable->horizontalHeader()->count(); i++)
    {
        if (!pSourceTable->horizontalHeader()->isSectionHidden(i))
            l << pSourceTable->model()->index(iRow, pSourceTable->horizontalHeader()->logicalIndex(i)).data(Qt::DisplayRole).toString();
    }
    QTextStream d(pDevice);
    d << l.join(";") << endl;
}

// XML Exporter
PBSTableViewXMLExporter::PBSTableViewXMLExporter(QTableView *table, QIODevice *device, QObject *parent) :
    PBSTableViewExporter(table, device, parent),
    xmlStream(device)
{
    xmlStream.setAutoFormatting(true);
    xmlStream.setAutoFormattingIndent(2);
}

void PBSTableViewXMLExporter::writeHeader()
{
    xmlStream.writeStartDocument("1.0");
    xmlStream.writeStartElement("ROWDATA");
}

void PBSTableViewXMLExporter::writeLine(int iRow)
{
    if(!pSourceTable)
        return;

    if(!pDevice)
        return;

    QStringList l;

    xmlStream.writeStartElement("ROW");

    for(int i = 0; i < pSourceTable->horizontalHeader()->count(); i++)
    {
        if (!pSourceTable->horizontalHeader()->isSectionHidden(i))
        {
            QString sData(pSourceTable->model()->headerData(pSourceTable->horizontalHeader()->logicalIndex(i), Qt::Horizontal, Qt::DisplayRole).toString());
            sData.replace(" ", "_");
            xmlStream.writeTextElement(sData,
                                       pSourceTable->model()->index(iRow, pSourceTable->horizontalHeader()->logicalIndex(i)).data(Qt::DisplayRole).toString());
        }
    }
    xmlStream.writeEndElement();
}

void PBSTableViewXMLExporter::writeFooter()
{
    xmlStream.writeEndElement();
    xmlStream.writeEndElement();
    xmlStream.writeEndElement();
}

// HTML Exporter
PBSTableViewHTMLExporter::PBSTableViewHTMLExporter(QTableView *table, QIODevice *device, QObject *parent) :
    PBSTableViewExporter(table, device, parent)
{
}

void PBSTableViewHTMLExporter::writeHeader()
{
    if(!pSourceTable)
        return;

    if(!pDevice)
        return;

    QStringList l;
    l << "<html>\n" << "<head/>\n" << "<body>\n" << "\t<table border=\"1\" style=\"border-style:none\">\n";
    l << "\t\t<font face=\"MS Shell Dlg 2\">\n\t\t\t<tr style=\"border-style:solid\">\n";

    for(int i = 0; i < pSourceTable->horizontalHeader()->count(); i++)
    {
        if (!pSourceTable->horizontalHeader()->isSectionHidden(i))
            l << "\t\t\t\t<th>" + pSourceTable->model()->headerData(pSourceTable->horizontalHeader()->logicalIndex(i), Qt::Horizontal, Qt::DisplayRole).toString() + " </th>\n";
    }
    l << "\t\t\t</tr>\n\t\t</font>\n";
    QTextStream d(pDevice);
    d << l.join("") << endl;
}

void PBSTableViewHTMLExporter::writeLine(int iRow)
{
    if(!pSourceTable)
        return;

    if(!pDevice)
        return;

    QStringList l;

    l << "\t\t\t<tr>\n";
    for(int i = 0; i < pSourceTable->horizontalHeader()->count(); i++)
    {
        if (!pSourceTable->horizontalHeader()->isSectionHidden(i))
            l << "\t\t\t\t<td>" + pSourceTable->model()->index(iRow, pSourceTable->horizontalHeader()->logicalIndex(i)).data(Qt::DisplayRole).toString() + "</td>\n";
    }
    l << "\t\t\t</tr>\n";
    QTextStream d(pDevice);
    d << l.join("");
}

void PBSTableViewHTMLExporter::writeFooter()
{
    QStringList l;
    l << "  </table>\n</body>\n</html>";
    QTextStream d(pDevice);
    d << l.join("") << endl;
}

// Excel XML Exporter
PBSTableViewExcelXMLExporter::PBSTableViewExcelXMLExporter(QTableView *table, QIODevice *device, QObject *parent) :
    PBSTableViewExporter(table, device, parent),
    xmlStream(device)
{
    xmlStream.setAutoFormatting(true);
    xmlStream.setAutoFormattingIndent(2);
}

void PBSTableViewExcelXMLExporter::writeHeader()
{
    xmlStream.writeStartDocument("1.0");
    xmlStream.writeProcessingInstruction("mso-application progid=\"Excel.Sheet\"");

    xmlStream.writeStartElement("Workbook");
    xmlStream.writeAttribute("xmlns", "urn:schemas-microsoft-com:office:spreadsheet");
    xmlStream.writeAttribute("xmlns:o", "urn:schemas-microsoft-com:office:office");
    xmlStream.writeAttribute("xmlns:x", "urn:schemas-microsoft-com:office:excel");
    xmlStream.writeAttribute("xmlns:ss", "urn:schemas-microsoft-com:office:spreadsheet");
    xmlStream.writeAttribute("xmlns:html", "http://www.w3.org/TR/REC-html40");

    xmlStream.writeStartElement("Styles");
    xmlStream.writeStartElement("Style");
    xmlStream.writeAttribute("ss:ID", "Default");
    xmlStream.writeAttribute("ss:Name", "Normal");
    xmlStream.writeStartElement("Alignment");
    xmlStream.writeAttribute("ss:Vertical", "Bottom");
    xmlStream.writeEndElement();
    xmlStream.writeEmptyElement("Borders");
    xmlStream.writeEmptyElement("Font");
    xmlStream.writeAttribute("ss:FontName", "Calibri");
    xmlStream.writeAttribute("x:Family", "Swiss");
    xmlStream.writeAttribute("ss:Size", "11");
    xmlStream.writeAttribute("ss:Color", "#000000");
    xmlStream.writeEmptyElement("Interior");
    xmlStream.writeEmptyElement("NumberFormat");
    xmlStream.writeEmptyElement("Protection");

    xmlStream.writeEndElement();
    xmlStream.writeEndElement();

    xmlStream.writeStartElement("Worksheet");
    xmlStream.writeAttribute("ss:Name", QObject::tr("Sheet1"));

    xmlStream.writeStartElement("Table");
    xmlStream.writeStartElement("Row");
    for (int c = 0; c < pSourceTable->horizontalHeader()->count(); c++)
    {
        if (!pSourceTable->horizontalHeader()->isSectionHidden(c))
        {
            xmlStream.writeStartElement("Cell");

            xmlStream.writeStartElement("Data");
            xmlStream.writeAttribute("ss:Type", "String");
            xmlStream.writeCharacters(pSourceTable->model()->headerData(pSourceTable->horizontalHeader()->logicalIndex(c), Qt::Horizontal, Qt::DisplayRole).toString());
            xmlStream.writeEndElement();
            xmlStream.writeEndElement();
        }
    }
    xmlStream.writeEndElement();
}

void PBSTableViewExcelXMLExporter::writeLine(int r)
{
    xmlStream.writeStartElement("Row");
    for (int c = 0; c < pSourceTable->horizontalHeader()->count(); c++)
    {
        if (!pSourceTable->horizontalHeader()->isSectionHidden(c))
        {
            xmlStream.writeStartElement("Cell");

            xmlStream.writeStartElement("Data");
            xmlStream.writeAttribute("ss:Type", "String");
            xmlStream.writeCharacters(pSourceTable->model()->index(r, pSourceTable->horizontalHeader()->logicalIndex(c)).data(Qt::DisplayRole).toString());
            xmlStream.writeEndElement();
            xmlStream.writeEndElement();
        }
    }
    xmlStream.writeEndElement();
}

void PBSTableViewExcelXMLExporter::writeFooter()
{
    xmlStream.writeEndElement();
    xmlStream.writeEndElement();
    xmlStream.writeEndElement();
}

// Word XML Exporter
PBSTableViewWordXMLExporter::PBSTableViewWordXMLExporter(QTableView *table, QIODevice *device, QObject *parent) :
    PBSTableViewExporter(table, device, parent),
    xmlStream(device),
    imageId(1)
{
}

void PBSTableViewWordXMLExporter::writeHeader()
{
    xmlStream.setAutoFormatting(true);
    xmlStream.setAutoFormattingIndent(2);

    xmlStream.writeStartDocument("1.0", true);
    xmlStream.writeProcessingInstruction("mso-application progid=\"Word.Document\"");
    xmlStream.writeStartElement("w:wordDocument");
    //
    xmlStream.writeAttribute("xmlns:w", "http://schemas.microsoft.com/office/word/2003/wordml");
    xmlStream.writeAttribute("xmlns:v", "urn:schemas-microsoft-com:vml");
    xmlStream.writeAttribute("xmlns:wx", "http://schemas.microsoft.com/office/word/2003/auxHint");
    xmlStream.writeAttribute("xmlns:o", "urn:schemas-microsoft-com:office:office");
    xmlStream.writeStartElement("o:DocumentProperties");
    xmlStream.writeTextElement("o:Author", "PBS");
    xmlStream.writeTextElement("o:LastAuthor", "PBS");
    xmlStream.writeTextElement("o:Version", "14");
    xmlStream.writeEndElement();
    xmlStream.writeStartElement("w:body");
    xmlStream.writeStartElement("w:tbl");
    xmlStream.writeStartElement("w:tblPr");
    xmlStream.writeStartElement("w:tblBorders");
    xmlStream.writeEmptyElement("w:top");
    xmlStream.writeAttribute("w:val", "single");
    xmlStream.writeAttribute("w:sz", "4");
    xmlStream.writeAttribute("wx:bdrwidth", "10");
    xmlStream.writeAttribute("w:space", "0");
    xmlStream.writeAttribute("w:color", "auto");
    xmlStream.writeEmptyElement("w:left");
    xmlStream.writeAttribute("w:val", "single");
    xmlStream.writeAttribute("w:sz", "4");
    xmlStream.writeAttribute("wx:bdrwidth", "10");
    xmlStream.writeAttribute("w:space", "0");
    xmlStream.writeAttribute("w:color", "auto");
    xmlStream.writeEmptyElement("w:bottom");
    xmlStream.writeAttribute("w:val", "single");
    xmlStream.writeAttribute("w:sz", "4");
    xmlStream.writeAttribute("wx:bdrwidth", "10");
    xmlStream.writeAttribute("w:space", "0");
    xmlStream.writeAttribute("w:color", "auto");
    xmlStream.writeEmptyElement("w:right");
    xmlStream.writeAttribute("w:val", "single");
    xmlStream.writeAttribute("w:sz", "4");
    xmlStream.writeAttribute("wx:bdrwidth", "10");
    xmlStream.writeAttribute("w:space", "0");
    xmlStream.writeAttribute("w:color", "auto");
    xmlStream.writeEmptyElement("w:insideH");
    xmlStream.writeAttribute("w:val", "single");
    xmlStream.writeAttribute("w:sz", "4");
    xmlStream.writeAttribute("wx:bdrwidth", "10");
    xmlStream.writeAttribute("w:space", "0");
    xmlStream.writeAttribute("w:color", "auto");
    xmlStream.writeEmptyElement("w:insideV");
    xmlStream.writeAttribute("w:val", "single");
    xmlStream.writeAttribute("w:sz", "4");
    xmlStream.writeAttribute("wx:bdrwidth", "10");
    xmlStream.writeAttribute("w:space", "0");
    xmlStream.writeAttribute("w:color", "auto");

    xmlStream.writeEndElement();
    xmlStream.writeEmptyElement("w:tblStyle");
    xmlStream.writeAttribute("w:val", "TableGrid");
    xmlStream.writeEmptyElement("w:tblW");
    xmlStream.writeAttribute("w:w", "0");
    xmlStream.writeAttribute("w:type", "auto");
    xmlStream.writeEmptyElement("w:tblLook");
    xmlStream.writeAttribute("w:val", "01E0");
    xmlStream.writeEndElement();
    xmlStream.writeStartElement("w:tblGrid");

    for (int c = 0; c < pSourceTable->horizontalHeader()->count(); c++)
    {
        if (!pSourceTable->horizontalHeader()->isSectionHidden(c))
        {
            int w = 1440 / pSourceTable->horizontalHeader()->logicalDpiX() * pSourceTable->horizontalHeader()->sectionSize(c);
            xmlStream.writeEmptyElement("w:gridCol");
            xmlStream.writeAttribute("w:w", QString("%1").arg(w));
            colWidth[c] = w;
        }
    }
    xmlStream.writeEndElement();
    xmlStream.writeStartElement("w:tr");

    for (int c = 0; c < pSourceTable->horizontalHeader()->count(); c++)
    {
        if (!pSourceTable->horizontalHeader()->isSectionHidden(c))
        {
            xmlStream.writeStartElement("w:trPr");
            xmlStream.writeEmptyElement("w:tblHeader");
            xmlStream.writeEndElement();
            xmlStream.writeStartElement("w:tc");

            xmlStream.writeStartElement("w:tcPr");
            xmlStream.writeEmptyElement("w:tcW");
            xmlStream.writeAttribute("w:w", QString("%1").arg(colWidth.value(c)));
            xmlStream.writeAttribute("w:type", "dxa");
            xmlStream.writeEmptyElement("w:shd");
            xmlStream.writeAttribute("w:val", "clear");
            xmlStream.writeAttribute("w:color", "auto");

            writeBackgroundColor(qvariant_cast<QBrush>(pSourceTable->model()->headerData(pSourceTable->horizontalHeader()->logicalIndex(c), Qt::Horizontal, Qt::BackgroundRole)));

            xmlStream.writeEndElement();

            xmlStream.writeStartElement("w:p");
            xmlStream.writeStartElement("w:r");
            xmlStream.writeStartElement("w:rPr");
            writeFont(qvariant_cast<QFont>(pSourceTable->model()->headerData(pSourceTable->horizontalHeader()->logicalIndex(c), Qt::Horizontal, Qt::FontRole)));
            xmlStream.writeEndElement();
            xmlStream.writeTextElement("w:t", pSourceTable->model()->headerData(pSourceTable->horizontalHeader()->logicalIndex(c), Qt::Horizontal, Qt::DisplayRole).toString());
            xmlStream.writeEndElement();
            xmlStream.writeEndElement();
            xmlStream.writeEndElement();
        }
    }
    xmlStream.writeEndElement();
}

void PBSTableViewWordXMLExporter::writeLine(int r)
{
    xmlStream.writeStartElement("w:tr");
    for (int c = 0; c < pSourceTable->horizontalHeader()->count(); c++)
    {
        if (!pSourceTable->horizontalHeader()->isSectionHidden(c))
        {
            xmlStream.writeStartElement("w:tc");

            xmlStream.writeStartElement("w:tcPr");
            xmlStream.writeEmptyElement("w:tcW");
            xmlStream.writeAttribute("w:w", QString("%1").arg(colWidth.value(c)));
            xmlStream.writeAttribute("w:type", "dxa");
            xmlStream.writeEmptyElement("w:shd");
            xmlStream.writeAttribute("w:val", "clear");
            xmlStream.writeAttribute("w:color", "auto");

            writeBackgroundColor(qvariant_cast<QBrush>(pSourceTable->model()->index(r, pSourceTable->horizontalHeader()->logicalIndex(c)).data(Qt::BackgroundRole)));
            xmlStream.writeEndElement();

            xmlStream.writeStartElement("w:p");
            xmlStream.writeStartElement("w:r");
            xmlStream.writeStartElement("w:rPr");
            writeFont(qvariant_cast<QFont>(pSourceTable->model()->index(r, pSourceTable->horizontalHeader()->logicalIndex(c)).data(Qt::FontRole)));
            xmlStream.writeEndElement();

            writeDecoration(pSourceTable->model()->index(r, pSourceTable->horizontalHeader()->logicalIndex(c)).data(Qt::DecorationRole));
            xmlStream.writeTextElement("w:t", pSourceTable->model()->index(r, pSourceTable->horizontalHeader()->logicalIndex(c)).data(Qt::DisplayRole).toString());
            xmlStream.writeEndElement();
            xmlStream.writeEndElement();
            xmlStream.writeEndElement();
        }
    }
    xmlStream.writeEndElement();
}

void PBSTableViewWordXMLExporter::writeFooter()
{
    xmlStream.writeEndElement();
    xmlStream.writeEndElement();
    xmlStream.writeEndElement();
}

void PBSTableViewWordXMLExporter::writeFont(const QFont &font)
{
    QFont f(font);
    if (f.family() == "MS Shell Dlg 2")
        f.setFamily("Arial");

    xmlStream.writeEmptyElement("w:rFonts");
    xmlStream.writeAttribute("w:ascii", f.family());
    xmlStream.writeAttribute("w:h-ansi", f.family());
    xmlStream.writeAttribute("w:cs", f.family());
    xmlStream.writeEmptyElement("wx:font");
    xmlStream.writeAttribute("wx:val", f.family());

    xmlStream.writeEmptyElement("w:sz");
    xmlStream.writeAttribute("w:val", QString("%1").arg(f.pointSize() * 2));
    xmlStream.writeEmptyElement("w:sz-cs");
    xmlStream.writeAttribute("w:val", QString("%1").arg(f.pointSize() * 2));
}

void PBSTableViewWordXMLExporter::writeBackgroundColor(const QBrush &br)
{
    if(br.style() != Qt::NoBrush)
        xmlStream.writeAttribute("w:fill", br.color().name());
}

void PBSTableViewWordXMLExporter::writeDecoration(const QVariant &decoration)
{
    QIcon icon = qvariant_cast<QIcon>(decoration);
    QPixmap pixmap;
    if (icon.isNull())
    {
        pixmap = qvariant_cast<QPixmap>(decoration);
        if (pixmap.isNull())
            return;
    }
    else
        pixmap = icon.pixmap(16, 16);

    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    pixmap.save(&buffer, "PNG");
    xmlStream.writeStartElement("w:pict");
    xmlStream.writeStartElement("w:binData");
    xmlStream.writeAttribute("w:name", QString("wordml://%1.png").arg(imageId, 8, 10, QLatin1Char('0')));
    xmlStream.writeAttribute("xml:space", "preserve");
    xmlStream.writeCharacters(bytes.toBase64());
    xmlStream.writeEndElement();
    //
    xmlStream.writeStartElement("v:shape");
    xmlStream.writeAttribute("style", QString("width:%1px;height:%2px").arg(pixmap.size().width()).arg(pixmap.size().height()));
    xmlStream.writeEmptyElement("v:imageData");
    xmlStream.writeAttribute("src", QString("wordml://%1.png").arg(imageId, 8, 10, QLatin1Char('0')));

    xmlStream.writeEndElement();
    xmlStream.writeEndElement();

    imageId++;
}

// PDF Exporter
PBSTableViewPDFExporter::PBSTableViewPDFExporter(QTableView *table, QString fName, QObject *parent) :
    PBSTableViewExporter(table, nullptr, parent),
    pdfName(fName)
{

}

void PBSTableViewPDFExporter::executeExport()
{
    QPdfWriter printer(pdfName);
    printer.setPageSize(QPagedPaintDevice::A4);
    printer.setPageMargins(QMargins(0, 0, 0, 0));
    printer.setCreator("PBS Library - http://www.elibolyazilim.com");
    printer.setPdfVersion(QPagedPaintDevice::PdfVersion_A1b);

/*
    QPrinter printer(QPrinter::HighResolution);
    printer.setPaperSize(QPrinter::A4);
    printer.setFullPage(true);
    printer.setPageMargins(0, 0, 0, 0, QPrinter::Unit::Millimeter);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(pdfName);
    printer.setOrientation(QPrinter::Portrait);
*/
     printer.setResolution(150);

    QPainter painter(&printer);

    QFont hdrFont(pSourceTable->horizontalHeader()->font());
    hdrFont.setPointSize(hdrFont.pointSize());

    QFont cntFont(pSourceTable->font());
    cntFont.setPointSize(cntFont.pointSize());

    PBSTablePrinter tablePrinter(&painter, pSourceTable);    
    // tablePrinter.setPrinter(&printer);
    tablePrinter.setWriter(&printer);

    tablePrinter.setHeadersFont(hdrFont);
    tablePrinter.setContentFont(cntFont);

    tablePrinter.setPageMargin(40, 40, 40, 40);

    tablePrinter.execute();

    painter.end();
}

// XLSX Exporter
PBSTableViewXLSXExporter::PBSTableViewXLSXExporter(QTableView *table, QString fName, QObject *parent) :
    PBSTableViewExporter(table, nullptr, parent),
    xlsxName(fName)
{

}

void PBSTableViewXLSXExporter::executeExport()
{
    QXlsx::Document xlsx;

    xlsx.setDocumentProperty("title", "PBSTableViewXLSXExporter Export Table");
    xlsx.setDocumentProperty("subject", "Table Data");
    xlsx.setDocumentProperty("creator", "Cüneyt ELİBOL");
    xlsx.setDocumentProperty("company", "ELİBOL Yazılım");
    xlsx.setDocumentProperty("category", "Table spreadsheets");
    xlsx.setDocumentProperty("keywords", "PBSTableViewXLSXExporter, Table, Data");
    xlsx.setDocumentProperty("description", "Created with PBSTableViewXLSXExporter Part Of PBSLib");

    xlsx.addSheet("PBSTableViewXLSXExporter");

using namespace QXlsx;

    // Worksheet *sheet = xlsx.currentWorksheet();

    Format hdrFmt;

    hdrFmt.setFont(pSourceTable->horizontalHeader()->font());
    hdrFmt.setHorizontalAlignment(Format::AlignHCenter);
    hdrFmt.setVerticalAlignment(Format::AlignVCenter);

    Format cntFmt;

    cntFmt.setFont(pSourceTable->font());
    cntFmt.setVerticalAlignment(Format::AlignVCenter);

    int ic = 1;

    for(int i = 0; i < pSourceTable->horizontalHeader()->count(); i++)
    {
        if (!pSourceTable->horizontalHeader()->isSectionHidden(i))
        {
            QString sText(pSourceTable->model()->headerData(pSourceTable->horizontalHeader()->logicalIndex(i), Qt::Horizontal, Qt::DisplayRole).toString());

            xlsx.write(1, ic++, sText, hdrFmt);
        }
    }

    for(int r = 0; r < pSourceTable->model()->rowCount(); r++)
    {
        ic = 1;
        for(int c = 0; c < pSourceTable->horizontalHeader()->count(); c++)
        {
            if (!pSourceTable->horizontalHeader()->isSectionHidden(c))
            {
                QModelIndex ix = pSourceTable->model()->index(r, pSourceTable->horizontalHeader()->logicalIndex(c));

                int f = pSourceTable->model()->data(ix, Qt::TextAlignmentRole).toInt();
                switch(f)
                {
                    case Qt::AlignLeft: cntFmt.setHorizontalAlignment(Format::AlignLeft); break;
                    case Qt::AlignRight: cntFmt.setHorizontalAlignment(Format::AlignRight); break;

                    case Qt::AlignCenter:
                    case Qt::AlignHCenter: cntFmt.setHorizontalAlignment(Format::AlignHCenter); break;
                }

                QVariant v(pSourceTable->model()->data(ix, Qt::EditRole));

                xlsx.write(r + 2, ic++, v, cntFmt);
            }
        }
    }

    xlsx.saveAs(xlsxName);
}
