#ifndef PBSTABLEVIEWTOOL_H
#define PBSTABLEVIEWTOOL_H

#include <QtWidgets>

typedef enum {ifNone, ifCheckBox } modelFlag;
typedef enum {skNone = 0, skCount = 1, skSum = 2, skMin = 3, skMax = 4, skAvg = 5} footerEvent;
typedef enum {etLineEdit, etMemo, etCheckBox, etIntSpin, etDoubleSpin, etDate, etDateTime, etProgress, etButton, etPicture } itemEditType;

struct ItemState
{
    Qt::Alignment aAlignColumn;
    Qt::Alignment aAlignFooter;

    itemEditType editType;

    bool bSortable;
    bool bFiltered;
    bool bHidden;
    bool bVisible;
    bool bEditable;
    bool bSelectable;

    QString sFilter;
    QString sFilterBoxFilter;
    QString sFormat;
    QString sFooterFormat;
    QString sCaption;

    footerEvent fFooterEvent;

    QLabel *footerLabel;
    QVariant footerValue;

    ItemState()
    {
        aAlignColumn = aAlignFooter = Qt::AlignRight | Qt::AlignVCenter;

        editType = etLineEdit;

        bSortable = bFiltered = bVisible = bEditable = bSelectable = true;
        bHidden = false;

        sFilter = "";
        sFilterBoxFilter = "";
        sFormat = "";
        sFooterFormat = "#,#0.#0";

        fFooterEvent = skNone;

        footerLabel = nullptr;
        footerValue = QVariant();
    }

    void copyFrom(ItemState its)
    {
        aAlignColumn = its.aAlignColumn;
        aAlignFooter = its.aAlignFooter;

        editType = its.editType;

        bSortable = its.bSortable;
        bFiltered = its.bFiltered;
        bVisible  = its.bVisible;
        bEditable = its.bEditable;
        bSelectable = its.bSelectable;
        bHidden   = its.bHidden;

        sFilter = its.sFilter;
        sFilterBoxFilter = its.sFilterBoxFilter;
        sFormat = its.sFormat;
        sFooterFormat = sFooterFormat;

        fFooterEvent = its.fFooterEvent;
    }
};

typedef QMap<int, ItemState> itemStateMap;

#endif // PBSTABLEVIEWTOOL_H
