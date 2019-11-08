#ifndef PBSTOOLS_H
#define PBSTOOLS_H

#include <QWidget>
#include <QObject>
#include <QValidator>
#include <QFileDialog>
#include <QApplication>
#include <QColor>
#include <QPalette>
#include <QPainter>
#include <QModelIndex>
#include <QItemSelection>

#define PBSFreeAndNull(d) if(d || d != nullptr) {delete d; d = nullptr;}

QString PBSTR(const char *sourceText, const char *comment = Q_NULLPTR, int n = -1);

namespace PBSTools{

QPair<QModelIndex, QModelIndex> selectionEdges(QItemSelection selection);
QString miliSecToTime(int);

static void dump_props(QObject *o);

QString appPath();
QString appBaseName();

bool isInteger(const QString& str);
bool isDecimal(const QString& str);
bool isNumber(const QString& str);
bool isHexadecimal(const QString& str);
bool isOctal(const QString& str);
bool isBinary(const QString& str);

bool isQuote(const QChar &c);
char * toCharP(QString in);

QDoubleValidator *PBSCreateDoubleValidator(double, double, int);
QIntValidator *PBSCreateIntValidator(int, int);

void centerWidgetOnScreen(QWidget* w);
void centerWidgetToParent(QWidget* w, QWidget* parent);
QString fileSizeToString(quint64 size);
QString truncatedText(const QString &text, int size);
QRegion roundedRect(const QRect &rect, int radius);

QString operatingSystem();
QString cpuArchitecture();
QString operatingSystemLong();

qulonglong getTotalMemory();
qulonglong getFreeMemory();

QByteArray pixmapToByteArray(const QPixmap &pix);
QPixmap pixmapFromByteArray(const QByteArray &data);
QPixmap dpiAwarePixmap(const QString &path);
QPixmap screenShot(QWidget *pWidget);

QImage blendImages(const QImage &image1, double alpha1, const QImage &image2, double alpha2);
QImage grayedImage(const QImage &image);
QImage tintedImage(const QImage &image, const QColor &color, QPainter::CompositionMode mode = QPainter::CompositionMode_Screen);
void blurImage(QImage &img, int radius);

QString encrypt(const QString &source);
QString decrypt(const QString &source);
};


namespace Colors
{
    const QColor &bg(const QPalette &pal, const QWidget* w);
    int contrast(const QColor &a, const QColor &b);
    QPalette::ColorRole counterRole(QPalette::ColorRole role);
    bool counterRole(QPalette::ColorRole &from, QPalette::ColorRole &to,
                 QPalette::ColorRole defFrom = QPalette::WindowText,
                 QPalette::ColorRole defTo = QPalette::Window);
    QColor emphasize(const QColor &c, int value = 10);
    bool haveContrast(const QColor &a, const QColor &b);
    QColor light(const QColor &c, int value);
    QColor mid(const QColor &oc1, const QColor &c2, int w1 = 1, int w2 = 1);
    int value(const QColor &c);
};

class PBSWaitCursor
{
public:
    PBSWaitCursor()
    {
        show();
    }

    ~PBSWaitCursor()
    {
        restore();
    }

    void show() const
    {
    #ifndef QT_NO_CURSOR
        QApplication::setOverrideCursor(Qt::WaitCursor);
        QApplication::processEvents();
    #endif
    }

    void restore() const
    {
    #ifndef QT_NO_CURSOR
        QApplication::restoreOverrideCursor();
        QApplication::processEvents();
    #endif
    }
};

#endif // PBSTOOLS_H
