/* This file is part of the KDE project
   Copyright (C) 2004-2006 David Faure <faure@kde.org>
   Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoOdfNumberStyles.h"

#include <QLocale>
#include <QBuffer>
#include <QDateTime>
#include <QTime>
#include <math.h>

namespace KoOdfNumberStyles
{

QString format(const QString &value, const NumericStyleFormat &format)
{
    switch (format.type) {
        case Number: {
            bool ok;
            qreal v = value.toDouble(&ok);
            return ok ? formatNumber(v, format.formatStr, format.precision) : value;
        } break;
        case Boolean: {
            return formatBoolean(value, format.formatStr);
        } break;
        case Date: {
            bool ok;
            int v = value.toInt(&ok);
            return ok ? formatDate(v, format.formatStr) : value;
        } break;
        case Time: {
            bool ok;
            qreal v = value.toDouble(&ok);
            return ok ? formatTime(v, format.formatStr) : value;
        } break;
        case DateTime: {
            bool ok;
            qreal v = value.toDouble(&ok);
            return ok ? formatDateTime(v, format.formatStr) : value;
        } break;
        case Percentage: {
            return formatPercent(value, format.formatStr, format.precision);
        } break;
        case Currency: {
            bool ok;
            qreal v = value.toDouble(&ok);
            return ok ? formatCurrency(v, format.formatStr, format.currencySymbol, format.precision) : value;
        } break;
        case Scientific: {
            bool ok;
            qreal v = value.toDouble(&ok);
            return ok ? formatScientific(v, format.formatStr, format.precision) : value;
        } break;
        case Fraction: {
            bool ok;
            qreal v = value.toDouble(&ok);
            return ok ? formatFraction(v, format.formatStr) : value;
        } break;
        case Text: {
            return value;
        } break;
    }
    return value;
}

QString formatNumber(qreal value, const QString &format, int precision)
{
    QString result;
    int start = 0;
    QString fmt(format);
    if(value < 0.00 && !fmt.startsWith("-"))
        fmt.insert(0, '-');

    bool showNegative = fmt.startsWith('-');
    if (showNegative)
        start = 1;
    for (int i = start; i < fmt.length(); ++i) {
        QChar c = fmt[ i ];
        switch (c.unicode()) {
            case '.':
            case ',':
            case '#':
            case '0':
            case '?': {
                bool grouping = false;
                bool gotDot = false;
                bool gotE = false;
                bool gotFraction = false;
                int decimalPlaces = 0;
                int integerDigits = 0;
                int optionalDecimalPlaces = 0;
                int optionalIntegerDigits = 0;
                int exponentDigits = 0;
                int numeratorDigits = 0;
                int denominatorDigits = 0;
                char ch = fmt[ i ].toLatin1();
                do {
                    if (ch == '.') {
                        gotDot = true;
                    } else if (ch == ',') {
                        grouping = true;
                    } else if (ch == 'E' || ch == 'e') {
                        //SET_TYPE_OR_RETURN(KoGenStyle::NumericScientificStyle);

                        if (i >= fmt.length() - 1) break;
                        const char chN = fmt[ i + 1 ].toLatin1();
                        if (chN == '-' || chN == '+') {
                            gotE = true;
                            ++i;
                        }
                    } else if (ch == '0' && gotE) {
                        ++exponentDigits;
                    } else if (ch == '0' && !gotDot && !gotFraction) {
                        ++integerDigits;
                    } else if (ch == '#' && !gotDot && !gotFraction) {
                        ++optionalIntegerDigits;
                    } else if (ch == '0' && gotDot && !gotFraction) {
                        ++decimalPlaces;
                    } else if (ch == '#' && gotDot && !gotFraction) {
                        ++optionalDecimalPlaces;
                    } else if (ch == '?' && !gotFraction) {
                        ++numeratorDigits;
                    } else if (ch == '?' && gotFraction) {
                        ++denominatorDigits;
                    } else if (ch == '/') {
                        //SET_TYPE_OR_RETURN(KoGenStyle::NumericFractionStyle);
                        if (gotDot) return QString(); // invalid
                        gotFraction = true;
                    }

                    if (i >= fmt.length() - 1) break;
                    ch = fmt[ ++i ].toLatin1();

                    if (ch == ' ') {
                        // spaces are not allowed - but there's an exception: if this is a fraction. Let's check for '?' or '/'
                        const char c = fmt[ i + 1 ].toLatin1();
                        if (c == '?' || c == '/')
                            ch = fmt[ ++i ].toLatin1();
                    }
                } while (i < fmt.length() && (ch == '.' || ch == ',' || ch == '#' || ch == '0' || ch == 'E' || ch == 'e' || ch == '?' || ch == '/'));
                if (!(ch == '.' || ch == ',' || ch == '#' || ch == '0' || ch == 'E' || ch == 'e' || ch == '?' || ch == '/')) {
                    --i;
                }
                QString v(QString::number(qAbs(value), 'f', precision >= 0 ? precision : (optionalDecimalPlaces + decimalPlaces)));
                int p = v.indexOf('.');
                QString integerValue = p >= 0 ? v.left(p) : v;
                if (integerValue.length() < integerDigits)
                    integerValue.prepend(QString().fill('0', integerDigits - integerValue.length()));
                QString decimalValue =  p >= 0 ? v.mid(p + 1) : QString();
                if (decimalValue.length() < decimalPlaces)
                    decimalValue.append(QString().fill('0', decimalPlaces - decimalValue.length()));

                if(grouping && integerValue.size() > 3)
                {
                    QLocale lc;

                    int y = -1, sz = integerValue.size();
                    for(int i = sz; i > 0; i--)
                    {
                        y++;
                        if(y == 3)
                        {
                            integerValue.insert(i, lc.groupSeparator());
                            y = 0;
                        }
                    }
                }

                if (showNegative && value < 0)
                    result.append('-');
                result.append(integerValue);
                if (!decimalValue.isEmpty())
                    result.append('.' + decimalValue);                                
            }
            break;
            case '\\': { // backslash escapes the next char
                if (i < fmt.length() - 1) {
                    result.append(fmt[ ++i ]);
                }
            } break;
            default:
                result.append(c);
                break;
        }
    }

    return result;
}

QString formatBoolean(const QString &value, const QString &format)
{
    Q_UNUSED(format);
    bool ok = false;
    int v = value.toInt(&ok);
    return ok && v != 0 ? "TRUE" : "FALSE";
}

QString formatDate(int value, const QString &format)
{
    QDateTime dt(QDate(1899, 12, 30)); // reference date
    dt = dt.addDays(value);
    return dt.toString(format);
}

QString formatTime(qreal value, const QString &format)
{
    QTime t(0,0,0);
    t = t.addSecs(qRound(value * 86400.0)); // 24 hours
    return t.toString(format);
}

QString formatDateTime(qreal value, const QString &format)
{
    QString v(QString::number(qAbs(value), 'f', 8));
    int p = v.indexOf('.');
    QString d = p >= 0 ? v.left(p) : "0";
    QString t = p >= 0 ? v.right(v.length() - p) : "0";

    bool b;

    int id = d.toInt(&b);
    if(!b) id = 0;

    qreal dt = t.toDouble(&b);
    if(!b) dt = 0;

    int it = dt * 86400.0;

    QDateTime qdt(QDate(1899, 12, 30)); // reference date

    qdt = qdt.addDays(id);
    qdt = qdt.addSecs(it);

    return qdt.toString(format);
}

QString formatCurrency(qreal value, const QString &format, const QString& currencySymbol, int precision)
{
    if (currencySymbol == "CCC") // undocumented hack, see doc attached to comment 6 at bug 282972
        return QLocale().toCurrencyString(value, "USD");
    if (format.isEmpty()) // no format means use locale format
        return QLocale().toCurrencyString(value, currencySymbol.isEmpty() ? QLocale().currencySymbol(QLocale::CurrencySymbol)
                                                                          : currencySymbol);
    return formatNumber(value, format, precision);
}

QString formatScientific(qreal value, const QString &format, int precision)
{
    Q_UNUSED(format);
    QString v(QString::number(value, 'E', precision));
    int pos = v.indexOf('.');
    if (pos != -1) {
        v.replace(pos, 1, QLocale().decimalPoint());
    }
    return v;
}

QString formatFraction(qreal value, const QString &format)
{
    QString prefix = value < 0 ? "-" : "";
    value = fabs(value);
    qreal result = value - floor(value);

    if (result == 0) // return w/o fraction part if not necessary
        return prefix + QString::number(value);

    int index = 0;
    int limit = 0;
    if (format.endsWith(QLatin1String("/2"))) {
        index = 2;
    } else if (format.endsWith(QLatin1String("/4"))) {
        index = 4;
    } else if (format.endsWith(QLatin1String("/8"))) {
        index = 8;
    } else if (format.endsWith(QLatin1String("/16"))) {
        index = 16;
    } else if (format.endsWith(QLatin1String("/10"))) {
        index = 10;
    } else if (format.endsWith(QLatin1String("/100"))) {
        index = 100;
    } else if (format.endsWith(QLatin1String("/?"))) {
        index = 3;
        limit = 9;
    } else if (format.endsWith(QLatin1String("/??"))) {
        index = 4;
        limit = 99;
    } else if (format.endsWith(QLatin1String("/???"))) {
        index = 5;
        limit = 999;
    } else { // fallback
        return prefix + QString::number(value);
    }

    // handle halves, quarters, tenths, ...
    if (!format.endsWith(QLatin1String("/?")) &&
        !format.endsWith(QLatin1String("/??")) &&
        !format.endsWith(QLatin1String("/???"))) {
        qreal calc = 0;
        int index1 = 0;
        qreal diff = result;
        for (int i = 1; i <= index; i++) {
            calc = i * 1.0 / index;
            if (fabs(result - calc) < diff) {
                index1 = i;
                diff = fabs(result - calc);
            }
        }
        if (index1 == 0)
            return prefix + QString("%1").arg(floor(value));
        if (index1 == index)
            return prefix + QString("%1").arg(floor(value) + 1);
        if (floor(value) == 0)
            return prefix + QString("%1/%2").arg(index1).arg(index);
        return prefix + QString("%1 %2/%3").arg(floor(value)).arg(index1).arg(index);
    }

    // handle Format::fraction_one_digit, Format::fraction_two_digit and Format::fraction_three_digit style
    qreal target = result;
    qreal numerator = 1;
    qreal denominator = 1;
    qreal bestNumerator = 0;
    qreal bestDenominator = 1;
    qreal bestDist = target;

    // as soon as either numerator or denominator gets above the limit, we're done
    while (numerator <= limit && denominator <= limit) {
        qreal dist = fabs((numerator / denominator) - target);
        if (dist < bestDist) {
            bestDist = dist;
            bestNumerator = numerator;
            bestDenominator = denominator;
        }
        if (numerator / denominator > target) {
            ++denominator;
        } else {
            ++numerator;
        }
    }

    if (bestNumerator == 0)
        return prefix + QString().setNum(floor(value));
    if (bestDenominator == bestNumerator)
        return prefix + QString().setNum(floor(value + 1));
    if (floor(value) == 0)
        return prefix + QString("%1/%2").arg(bestNumerator).arg(bestDenominator);
    return prefix + QString("%1 %2/%3").arg(floor(value)).arg(bestNumerator).arg(bestDenominator);

}

QString formatPercent(const QString &value, const QString &/*format*/, int precision)
{
    if (value.contains('.')) {
        bool ok;
        qreal v = value.toDouble(&ok);
        if (ok)
            return QString::number(v * 100., 'f', precision) + QLatin1String("%");
    }
    return value;
}

}
