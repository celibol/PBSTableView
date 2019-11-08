#include <pbstools.h>

#include <QApplication>
#include <QDesktopWidget>
#include <QDir>
#include <QBuffer>
#include <QIcon>
#include <QProcess>
#include <QMessageBox>
#include <QOperatingSystemVersion>
#include <QPixmap>
#include <QTextStream>
#include <QTime>
#include <QDebug>
#include <QMetaProperty>

// System
#ifdef Q_OS_WIN
#include <windows.h>
#elif defined(Q_OS_FREEBSD)
#include <sys/types.h>
#include <sys/sysctl.h>
#include <vm/vm_param.h>
#endif

#define CLAMP(x,l,u) (x) < (l) ? (l) :\
    (x) > (u) ? (u) :\
    (x)

QString PBSTR(const char *sourceText, const char *comment, int n)
{
    return QObject::tr(sourceText, comment, n);
}

namespace PBSTools{

QString miliSecToTime(int msecs)
{
    int hours = msecs/(1000*60*60);
    int minutes = (msecs-(hours*1000*60*60))/(1000*60);
    int seconds = (msecs-(minutes*1000*60)-(hours*1000*60*60))/1000;
    int milliseconds = msecs-(seconds*1000)-(minutes*1000*60)-(hours*1000*60*60);

    QString formattedTime;
    formattedTime.append(QString("%1").arg(hours, 2, 10, QLatin1Char('0')) + ":" +
                         QString( "%1" ).arg(minutes, 2, 10, QLatin1Char('0')) + ":" +
                         QString( "%1" ).arg(seconds, 2, 10, QLatin1Char('0')) + ":" +
                         QString( "%1" ).arg(milliseconds, 3, 10, QLatin1Char('0')));

    return formattedTime;
}

QPair<QModelIndex, QModelIndex> selectionEdges(QItemSelection selection)
{
    QPair<QModelIndex, QModelIndex> p;
    Q_FOREACH(QItemSelectionRange range, selection)
    {
        if (!p.first.isValid())
            p.first = range.topLeft();
        else
        {
            if (range.topLeft() < p.first)
                p.first = range.topLeft();
        }
        if (!p.second.isValid())
            p.second = range.bottomRight();
        else if (p.second < range.bottomRight())
            p.second = range.bottomRight();
    }
    return p;
}

static void dump_props(QObject *o)
{
  auto mo = o->metaObject();
  qDebug() << "## Properties of" << o << "##";
  do {
    qDebug() << "### Class" << mo->className() << "###";
    std::vector<std::pair<QString, QVariant> > v;
    v.reserve(mo->propertyCount() - mo->propertyOffset());
    for (int i = mo->propertyOffset(); i < mo->propertyCount();
          ++i)
      v.emplace_back(mo->property(i).name(),
                     mo->property(i).read(o));
    std::sort(v.begin(), v.end());
    for (auto &i : v)
      qDebug() << i.first << "=>" << i.second;
  } while ((mo = mo->superClass()));
}

bool isAscii(const QString& str)
{
    foreach (const QChar c, str)
    {
        if (c.unicode() > 127)
            return false;
    }

    return true;
}

bool isInteger(const QString& str)
{
    foreach (const QChar c, str)
    {
        if (!c.isDigit())
            return false;
    }

    return true;
}

bool isDecimal(const QString& str)
{
    bool success;
    str.toDouble(&success);

    return success;
}

bool isNumber(const QString& str)
{
    return isInteger(str) || isDecimal(str);
}

bool isHexadecimal(const QString& str)
{
    bool success;

    str.toULongLong(&success, 16);

    return success;
}

bool isOctal(const QString& str)
{
    bool success;

    str.toULongLong(&success, 8);

    return success;
}

bool isBinary(const QString& str)
{
    bool success;

    str.toULongLong(&success, 2);

    return success;
}


char *toCharP(QString in)
{
    QByteArray a;
    a.append(in);

    return a.data();
}

QString appPath()
{
    QDir dir(QCoreApplication::applicationDirPath());
    return dir.absolutePath() + QDir::separator();
}

QString appBaseName()
{
    QString appBaseName = QCoreApplication::applicationFilePath();
    QStringList sl = appBaseName.split(QDir::separator());
    appBaseName = sl[sl.count()-1];

    appBaseName.replace( ".exe", "", Qt::CaseInsensitive );

    return appBaseName;
}

QDoubleValidator *PBSCreateDoubleValidator(double min, double max, int dec)
{
    QDoubleValidator *pRes = new QDoubleValidator;
    pRes->setBottom(min);
    pRes->setTop(max);
    pRes->setDecimals(dec);

    return pRes;
}

QIntValidator *PBSCreateIntValidator(int min, int max)
{
    QIntValidator *pRes = new QIntValidator;
    pRes->setBottom(min);
    pRes->setTop(max);

    return pRes;
}

QPixmap screenShot(QWidget *pWidget)
{
    return pWidget->grab(pWidget->rect());
}

void centerWidgetOnScreen(QWidget* w)
{
    const QRect screen = QApplication::desktop()->screenGeometry(w);
    const QRect size = w->geometry();
    w->move((screen.width() - size.width()) / 2, (screen.height() - size.height()) / 2);
}

void centerWidgetToParent(QWidget* w, QWidget* parent)
{
    if (!parent || !w) {
        return;
    }

    QPoint p;
    parent = parent->window();
    QPoint pp = parent->mapToGlobal(QPoint(0, 0));
    p = QPoint(pp.x() + parent->width() / 2, pp.y() + parent->height() / 2);
    p = QPoint(p.x() - w->width() / 2, p.y() - w->height() / 2 - 20);

    w->move(p);
}

QString fileSizeToString(quint64 size)
{
    double _size = size / 1024.0; // KB
    if (_size < 1000) {
        return QString::number(_size > 1 ? _size : 1, 'f', 0) + " " + PBSTR("KB");
    }

    _size /= 1024; // MB
    if (_size < 1000) {
        return QString::number(_size, 'f', 1) + " " + PBSTR("MB");
    }

    _size /= 1024; // GB
    return QString::number(_size, 'f', 2) + " " + PBSTR("GB");
}

QString truncatedText(const QString &text, int size)
{
    if (text.length() > size) {
        return text.left(size) + QLatin1String("..");
    }
    return text;
}

QRegion roundedRect(const QRect &rect, int radius)
{
    QRegion region;

    // middle and borders
    region += rect.adjusted(radius, 0, -radius, 0);
    region += rect.adjusted(0, radius, 0, -radius);

    // top left
    QRect corner(rect.topLeft(), QSize(radius * 2, radius * 2));
    region += QRegion(corner, QRegion::Ellipse);

    // top right
    corner.moveTopRight(rect.topRight());
    region += QRegion(corner, QRegion::Ellipse);

    // bottom left
    corner.moveBottomLeft(rect.bottomLeft());
    region += QRegion(corner, QRegion::Ellipse);

    // bottom right
    corner.moveBottomRight(rect.bottomRight());
    region += QRegion(corner, QRegion::Ellipse);

    return region;
}

bool isQuote(const QChar &c)
{
    return (c == QLatin1Char('"') || c == QLatin1Char('\''));
}

QString operatingSystem()
{
    QOperatingSystemVersion osversion = QOperatingSystemVersion::current();

    QString str = QString("%1 %2.%3").arg(osversion.name()).arg(osversion.majorVersion()).arg(osversion.minorVersion());

    return str;
}

QString cpuArchitecture()
{
    return QSysInfo::currentCpuArchitecture();
}

QString operatingSystemLong()
{
    const QString arch = cpuArchitecture();
    if (arch.isEmpty())
        return operatingSystem();
    return operatingSystem() + QStringLiteral(" ") + arch;
}

QByteArray pixmapToByteArray(const QPixmap &pix)
{
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    if (pix.save(&buffer, "PNG")) {
        return buffer.buffer().toBase64();
    }

    return QByteArray();
}

QPixmap pixmapFromByteArray(const QByteArray &data)
{
    QPixmap image;
    QByteArray bArray = QByteArray::fromBase64(data);
    image.loadFromData(bArray);

    return image;
}

QPixmap dpiAwarePixmap(const QString &path)
{
    const QIcon icon(path);
    if (icon.availableSizes().isEmpty()) {
        return QPixmap(path);
    }
    return icon.pixmap(icon.availableSizes().at(0));
}

qulonglong getTotalMemory()
{
    static qulonglong cachedValue = 0;
    if ( cachedValue )
        return cachedValue;

#if defined(Q_OS_LINUX)
    // if /proc/meminfo doesn't exist, return 128MB
    QFile memFile( QStringLiteral("/proc/meminfo") );
    if ( !memFile.open( QIODevice::ReadOnly ) )
        return (cachedValue = 134217728);

    QTextStream readStream( &memFile );
    while ( true )
    {
        QString entry = readStream.readLine();
        if ( entry.isNull() ) break;
        if ( entry.startsWith( QStringLiteral("MemTotal:") ) )
            return (cachedValue = (Q_UINT64_C(1024) * entry.section( QLatin1Char(' '), -2, -2 ).toULongLong()));
    }
#elif defined(Q_OS_FREEBSD)
    qulonglong physmem;
    int mib[] = {CTL_HW, HW_PHYSMEM};
    size_t len = sizeof( physmem );
    if ( sysctl( mib, 2, &physmem, &len, NULL, 0 ) == 0 )
        return (cachedValue = physmem);
#elif defined(Q_OS_WIN)
    MEMORYSTATUSEX stat;
    stat.dwLength = sizeof(stat);
    GlobalMemoryStatusEx (&stat);

    return ( cachedValue = stat.ullTotalPhys );
#endif
    return (cachedValue = 134217728);
}

qulonglong getFreeMemory()
{
    static QTime lastUpdate = QTime::currentTime().addSecs(-3);
    static qulonglong cachedValue = 0;

    if ( qAbs( lastUpdate.secsTo( QTime::currentTime() ) ) <= 2 )
        return cachedValue;

#if defined(Q_OS_LINUX)
    // if /proc/meminfo doesn't exist, return MEMORY FULL
    QFile memFile( QStringLiteral("/proc/meminfo") );
    if ( !memFile.open( QIODevice::ReadOnly ) )
        return 0;

    // read /proc/meminfo and sum up the contents of 'MemFree', 'Buffers'
    // and 'Cached' fields. consider swapped memory as used memory.
    qulonglong memoryFree = 0;
    QString entry;
    QTextStream readStream( &memFile );
    static const int nElems = 5;
    QString names[nElems] = { QStringLiteral("MemFree:"), QStringLiteral("Buffers:"), QStringLiteral("Cached:"), QStringLiteral("SwapFree:"), QStringLiteral("SwapTotal:") };
    qulonglong values[nElems] = { 0, 0, 0, 0, 0 };
    bool foundValues[nElems] = { false, false, false, false, false };
    while ( true )
    {
        entry = readStream.readLine();
        if ( entry.isNull() ) break;
        for ( int i = 0; i < nElems; ++i )
        {
            if ( entry.startsWith( names[i] ) )
            {
                values[i] = entry.section( QLatin1Char(' '), -2, -2 ).toULongLong( &foundValues[i] );
            }
        }
    }
    memFile.close();
    bool found = true;
    for ( int i = 0; found && i < nElems; ++i )
        found = found && foundValues[i];
    if ( found )
    {
        memoryFree = values[0] + values[1] + values[2] + values[3];
        if ( values[4] > memoryFree )
            memoryFree = 0;
        else
            memoryFree -= values[4];
    }

    lastUpdate = QTime::currentTime();

    return ( cachedValue = (Q_UINT64_C(1024) * memoryFree) );
#elif defined(Q_OS_FREEBSD)
    qulonglong cache, inact, free, psize;
    size_t cachelen, inactlen, freelen, psizelen;
    cachelen = sizeof( cache );
    inactlen = sizeof( inact );
    freelen = sizeof( free );
    psizelen = sizeof( psize );
    // sum up inactive, cached and free memory
    if ( sysctlbyname( "vm.stats.vm.v_cache_count", &cache, &cachelen, NULL, 0 ) == 0 &&
            sysctlbyname( "vm.stats.vm.v_inactive_count", &inact, &inactlen, NULL, 0 ) == 0 &&
            sysctlbyname( "vm.stats.vm.v_free_count", &free, &freelen, NULL, 0 ) == 0 &&
            sysctlbyname( "vm.stats.vm.v_page_size", &psize, &psizelen, NULL, 0 ) == 0 )
    {
        lastUpdate = QTime::currentTime();
        return (cachedValue = (cache + inact + free) * psize);
    }
    else
    {
        return 0;
    }
#elif defined(Q_OS_WIN)
    MEMORYSTATUSEX stat;
    stat.dwLength = sizeof(stat);
    GlobalMemoryStatusEx (&stat);

    lastUpdate = QTime::currentTime();

    return ( cachedValue = stat.ullAvailPhys );
#else
    // tell the memory is full.. will act as in LOW profile
    return 0;
#endif
}

#define XOR_HASH1 "b94a54d87ab421a1d3d5631d1fc04e6c"
#define XOR_HASH2 "66b5e6e290e308e77517d4a1f9871e57"

QString xorString(const QString &source, const QString &hash)
{
    QString res;
    const ushort *cs = source.utf16();
    const ushort *ch = hash.utf16();
    for (int i = 0; i < source.length(); i++)
    {
        int j = i % 32;
        res.append(cs[i] ^ ch[j]);
    }
    return res;
}

QString encrypt(const QString &source)
{
    return xorString(xorString(source, XOR_HASH1), XOR_HASH2);
}

QString decrypt(const QString &source)
{
    return xorString(xorString(source, XOR_HASH2), XOR_HASH1);
}

QImage ImageblendImages(const QImage &image1, double alpha1, const QImage &image2, double alpha2)
{
    QImage resultImage(image1.size(), QImage::Format_ARGB32_Premultiplied);

    QPainter p;
    p.begin(&resultImage);
    //p.fillRect(QRect(0, 0, image1.size().width(), image1.size().height()), Qt::red);

    //p.setCompositionMode(QPainter::CompositionMode_SourceOver);
    if (alpha2 > 0)
    {
        p.setOpacity(alpha2);
        p.drawImage(0, 0, image2);
    }
    if (alpha1 > 0)
    {
        p.setOpacity(alpha1);
        p.drawImage(0, 0, image1);
    }
    p.end();

    if (alpha1 > 0.8)
        resultImage.setAlphaChannel(image1.alphaChannel());

    return resultImage;
}

QImage grayedImage(const QImage &image)
{
    QImage img = image;
    int pixels = img.width() * img.height();
    unsigned int *data = (unsigned int *)img.bits();
    for (int i = 0; i < pixels; ++i)
    {
        int val = qGray(data[i]);
        data[i] = qRgba(val, val, val, qAlpha(data[i]));
    }
    return img;
}

QImage tintedImage(const QImage &image, const QColor &color, QPainter::CompositionMode mode)
{
    QImage resultImage(image.size(), QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&resultImage);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.drawImage(0, 0, grayedImage(image));
    painter.setCompositionMode(mode);
    painter.fillRect(resultImage.rect(), color);
    painter.end();
    resultImage.setAlphaChannel(image.alphaChannel());
    return resultImage;
}

// Stack Blur Algorithm by Mario Klingemann <mario@quasimondo.com>
void blurImage(QImage &img, int radius)
{
    if (radius < 1)
    {
        return ;
    }

    QRgb *pix = (QRgb*)img.bits();
    int w = img.width();
    int h = img.height();
    int wm = w - 1;
    int hm = h - 1;
    int wh = w * h;
    int div = radius + radius + 1;

    int *r = new int[wh];
    int *g = new int[wh];
    int *b = new int[wh];
    int *a = new int[wh];
    int rsum, gsum, bsum, asum, x, y, i, yp, yi, yw;
    QRgb p;
    int *vmin = new int[qMax(w, h)];

    int divsum = (div + 1) >> 1;
    divsum *= divsum;
    int *dv = new int[256 * divsum];
    for (i = 0; i < 256*divsum; ++i)
    {
        dv[i] = (i / divsum);
    }

    yw = yi = 0;

    int **stack = new int * [div];
    for (int i = 0; i < div; ++i)
    {
        stack[i] = new int[4];
    }


    int stackpointer;
    int stackstart;
    int *sir;
    int rbs;
    int r1 = radius + 1;
    int routsum, goutsum, boutsum, aoutsum;
    int rinsum, ginsum, binsum, ainsum;

    for (y = 0; y < h; ++y)
    {
        rinsum = ginsum = binsum = ainsum
                                   = routsum = goutsum = boutsum = aoutsum
                                                                   = rsum = gsum = bsum = asum = 0;
        for (i = - radius; i <= radius; ++i)
        {
            p = pix[yi + qMin(wm, qMax(i, 0))];
            sir = stack[i + radius];
            sir[0] = qRed(p);
            sir[1] = qGreen(p);
            sir[2] = qBlue(p);
            sir[3] = qAlpha(p);

            rbs = r1 - abs(i);
            rsum += sir[0] * rbs;
            gsum += sir[1] * rbs;
            bsum += sir[2] * rbs;
            asum += sir[3] * rbs;

            if (i > 0)
            {
                rinsum += sir[0];
                ginsum += sir[1];
                binsum += sir[2];
                ainsum += sir[3];
            }
            else
            {
                routsum += sir[0];
                goutsum += sir[1];
                boutsum += sir[2];
                aoutsum += sir[3];
            }
        }
        stackpointer = radius;

        for (x = 0; x < w; ++x)
        {

            r[yi] = dv[rsum];
            g[yi] = dv[gsum];
            b[yi] = dv[bsum];
            a[yi] = dv[asum];

            rsum -= routsum;
            gsum -= goutsum;
            bsum -= boutsum;
            asum -= aoutsum;

            stackstart = stackpointer - radius + div;
            sir = stack[stackstart % div];

            routsum -= sir[0];
            goutsum -= sir[1];
            boutsum -= sir[2];
            aoutsum -= sir[3];

            if (y == 0)
            {
                vmin[x] = qMin(x + radius + 1, wm);
            }
            p = pix[yw + vmin[x]];

            sir[0] = qRed(p);
            sir[1] = qGreen(p);
            sir[2] = qBlue(p);
            sir[3] = qAlpha(p);

            rinsum += sir[0];
            ginsum += sir[1];
            binsum += sir[2];
            ainsum += sir[3];

            rsum += rinsum;
            gsum += ginsum;
            bsum += binsum;
            asum += ainsum;

            stackpointer = (stackpointer + 1) % div;
            sir = stack[(stackpointer) % div];

            routsum += sir[0];
            goutsum += sir[1];
            boutsum += sir[2];
            aoutsum += sir[3];

            rinsum -= sir[0];
            ginsum -= sir[1];
            binsum -= sir[2];
            ainsum -= sir[3];

            ++yi;
        }
        yw += w;
    }
    for (x = 0; x < w; ++x)
    {
        rinsum = ginsum = binsum = ainsum
                                   = routsum = goutsum = boutsum = aoutsum
                                                                   = rsum = gsum = bsum = asum = 0;

        yp = - radius * w;

        for (i = -radius; i <= radius; ++i)
        {
            yi = qMax(0, yp) + x;

            sir = stack[i + radius];

            sir[0] = r[yi];
            sir[1] = g[yi];
            sir[2] = b[yi];
            sir[3] = a[yi];

            rbs = r1 - abs(i);

            rsum += r[yi] * rbs;
            gsum += g[yi] * rbs;
            bsum += b[yi] * rbs;
            asum += a[yi] * rbs;

            if (i > 0)
            {
                rinsum += sir[0];
                ginsum += sir[1];
                binsum += sir[2];
                ainsum += sir[3];
            }
            else
            {
                routsum += sir[0];
                goutsum += sir[1];
                boutsum += sir[2];
                aoutsum += sir[3];
            }

            if (i < hm)
            {
                yp += w;
            }
        }

        yi = x;
        stackpointer = radius;

        for (y = 0; y < h; ++y)
        {
            pix[yi] = qRgba(dv[rsum], dv[gsum], dv[bsum], dv[asum]);

            rsum -= routsum;
            gsum -= goutsum;
            bsum -= boutsum;
            asum -= aoutsum;

            stackstart = stackpointer - radius + div;
            sir = stack[stackstart % div];

            routsum -= sir[0];
            goutsum -= sir[1];
            boutsum -= sir[2];
            aoutsum -= sir[3];

            if (x == 0)
            {
                vmin[y] = qMin(y + r1, hm) * w;
            }
            p = x + vmin[y];

            sir[0] = r[p];
            sir[1] = g[p];
            sir[2] = b[p];
            sir[3] = a[p];

            rinsum += sir[0];
            ginsum += sir[1];
            binsum += sir[2];
            ainsum += sir[3];

            rsum += rinsum;
            gsum += ginsum;
            bsum += binsum;
            asum += ainsum;

            stackpointer = (stackpointer + 1) % div;
            sir = stack[stackpointer];

            routsum += sir[0];
            goutsum += sir[1];
            boutsum += sir[2];
            aoutsum += sir[3];

            rinsum -= sir[0];
            ginsum -= sir[1];
            binsum -= sir[2];
            ainsum -= sir[3];

            yi += w;
        }
    }
    delete [] r;
    delete [] g;
    delete [] b;
    delete [] a;
    delete [] vmin;
    delete [] dv;
}

};

namespace Colors{

const QColor &bg(const QPalette &pal, const QWidget* w)
{
    QPalette::ColorRole role;
    if (!w) {
        role = QPalette::Window;
    }
    else if (w->parentWidget()) {
        role = w->parentWidget()->backgroundRole();
    }
    else {
        role = w->backgroundRole();
    }

//     if (pal.brush(role).style() > 1)
    return pal.color(role);
//     return QApplication::palette().color(role);
}

int contrast(const QColor &a, const QColor &b)
{
    int ar, ag, ab, br, bg, bb;
    a.getRgb(&ar, &ag, &ab);
    b.getRgb(&br, &bg, &bb);

    int diff = 299 * (ar - br) + 587 * (ag - bg) + 114 * (ab - bb);
    diff = (diff < 0) ? -diff : 90 * diff / 100;
    int perc = diff / 2550;

    diff = qMax(ar, br) + qMax(ag, bg) + qMax(ab, bb)
           - (qMin(ar, br) + qMin(ag, bg) + qMin(ab, bb));

    perc += diff / 765;
    perc /= 2;

    return perc;
}

QPalette::ColorRole counterRole(QPalette::ColorRole role)
{
    switch (role) {
    case QPalette::ButtonText: //8
        return QPalette::Button;
    case QPalette::WindowText: //0
        return QPalette::Window;
    case QPalette::HighlightedText: //13
        return QPalette::Highlight;
    case QPalette::Window: //10
        return QPalette::WindowText;
    case QPalette::Base: //9
        return QPalette::Text;
    case QPalette::Text: //6
        return QPalette::Base;
    case QPalette::Highlight: //12
        return QPalette::HighlightedText;
    case QPalette::Button: //1
        return QPalette::ButtonText;
    default:
        return QPalette::Window;
    }
}

bool counterRole(QPalette::ColorRole &from, QPalette::ColorRole &to, QPalette::ColorRole defFrom,
                    QPalette::ColorRole defTo)
{
    switch (from) {
    case QPalette::WindowText: //0
        to = QPalette::Window;
        break;
    case QPalette::Window: //10
        to = QPalette::WindowText;
        break;
    case QPalette::Base: //9
        to = QPalette::Text;
        break;
    case QPalette::Text: //6
        to = QPalette::Base;
        break;
    case QPalette::Button: //1
        to = QPalette::ButtonText;
        break;
    case QPalette::ButtonText: //8
        to = QPalette::Button;
        break;
    case QPalette::Highlight: //12
        to = QPalette::HighlightedText;
        break;
    case QPalette::HighlightedText: //13
        to = QPalette::Highlight;
        break;
    default:
        from = defFrom;
        to = defTo;
        return false;
    }
    return true;
}

QColor emphasize(const QColor &c, int value)
{
    int h, s, v, a;
    QColor ret;
    c.getHsv(&h, &s, &v, &a);
    if (v < 75 + value) {
        ret.setHsv(h, s, CLAMP(85 + value, 85, 255), a);
        return ret;
    }
    if (v > 200) {
        if (s > 30) {
            h -= 5;
            if (h < 0) {
                h = 360 + h;
            }
            s = (s << 3) / 9;
            v += value;
            ret.setHsv(h, CLAMP(s, 30, 255), CLAMP(v, 0, 255), a);
            return ret;
        }
        if (v > 230) {
            ret.setHsv(h, s, CLAMP(v - value, 0, 255), a);
            return ret;
        }
    }
    if (v > 128) {
        ret.setHsv(h, s, CLAMP(v + value, 0, 255), a);
    }
    else {
        ret.setHsv(h, s, CLAMP(v - value, 0, 255), a);
    }
    return ret;
}

bool haveContrast(const QColor &a, const QColor &b)
{
    int ar, ag, ab, br, bg, bb;
    a.getRgb(&ar, &ag, &ab);
    b.getRgb(&br, &bg, &bb);

    int diff = (299 * (ar - br) + 587 * (ag - bg) + 114 * (ab - bb));

    if (qAbs(diff) < 91001) {
        return false;
    }

    diff = qMax(ar, br) + qMax(ag, bg) + qMax(ab, bb)
           - (qMin(ar, br) + qMin(ag, bg) + qMin(ab, bb));

    return (diff > 300);
}

QColor light(const QColor &c, int value)
{
    int h, s, v, a;
    c.getHsv(&h, &s, &v, &a);
    QColor ret;
    if (v < 255 - value) {
        ret.setHsv(h, s, CLAMP(v + value, 0, 255), a); //value could be negative
        return ret;
    }
    // psychovisual uplightning, i.e. shift hue and lower saturation
    if (s > 30) {
        h -= (value * 5 / 20);
        if (h < 0) {
            h = 400 + h;
        }
        s = CLAMP((s << 3) / 9, 30, 255);
        ret.setHsv(h, s, 255, a);
        return ret;
    }
    else { // hue shifting has no sense, half saturation (btw, white won't get brighter :)
        ret.setHsv(h, s >> 1, 255, a);
    }
    return ret;
}

QColor mid(const QColor &c1, const QColor &c2, int w1, int w2)
{
    int sum = (w1 + w2);
    if (!sum) {
        return Qt::black;
    }

    int r, g, b, a;
#if 0
    QColor c1 = oc1;
    b = value(c1);
    if (b < 70) {
        c1.getHsv(&r, &g, &b, &a);
        c1.setHsv(r, g, 70, a);
    }
#endif
    r = (w1 * c1.red() + w2 * c2.red()) / sum;
    r = CLAMP(r, 0, 255);
    g = (w1 * c1.green() + w2 * c2.green()) / sum;
    g = CLAMP(g, 0, 255);
    b = (w1 * c1.blue() + w2 * c2.blue()) / sum;
    b = CLAMP(b, 0, 255);
    a = (w1 * c1.alpha() + w2 * c2.alpha()) / sum;
    a = CLAMP(a, 0, 255);
    return QColor(r, g, b, a);
}

int value(const QColor &c)
{
    int v = c.red();
    if (c.green() > v) {
        v = c.green();
    }
    if (c.blue() > v) {
        v = c.blue();
    }
    return v;
}
};
