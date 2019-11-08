#ifndef PBSTABLEWIDGET_H
#define PBSTABLEWIDGET_H

#include <QtWidgets>
#include <pbsheaderview.h>

class PBSTableCornerButton : public QAbstractButton
{
    Q_OBJECT
public:
    PBSTableCornerButton(QWidget *parent = 0);
    void paintEvent(QPaintEvent*);
    void setText(const QString &text) {m_text = text;}
    void setAlignment(Qt::Alignment align) {m_align = align;}

private:
    QString m_text;
    Qt::Alignment m_align;
};

class PBSTableWidget : public QTableView
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QTableView)

public:
    explicit PBSTableWidget(QWidget *parent = nullptr);
    ~PBSTableWidget();

    void setModel(QAbstractItemModel *model) override;

    void gotoRow(int);
    void gotoColumn(int);
    void moveTo(int, int);

    int currentRow();
    int currentColumn();
    QModelIndex currentPos();

    bool enableQuickCustomize();
    void setEnableQuickCustomize(bool);

    bool nextCellOnEnter() { return bNextCellOnEnter; }
    void setNextCellOnEnter(bool b) { bNextCellOnEnter = b; }

    bool nextRowOnLastCol() { return bNextRowOnLastCol; }
    void setNextRowOnLastCol(bool b) { bNextRowOnLastCol = b; }

    bool gotoFirstCellOnNewRecord() { return bGotoFirstCellOnNewRec; }
    void setGotoFirstCellOnNewRecord(bool b) { bGotoFirstCellOnNewRec = b; }

    bool columnAutoWidth();
    void setColumnAutoWidth(bool);

    bool sortEnabled();
    void setSortEnabled(bool);

    bool autoEdit();
    void setAutoEdit(bool);

    bool readOnly();
    void setReadOnly(bool);

    bool selectRow();
    void setSelectRow(bool);

    bool alternateRowColor();
    void setAlternateRowColor(bool);

    void setSectionMoveable(bool);
    bool sectionMoveable();

    bool canDeleteRow();
    void setCanDeleteRow(bool);

    bool canAddRow();
    void setCanAddRow(bool);
signals:
    void filterChanged(int, QString);
    void filterButtonPressed(int);
    void newRecordAdd();
    void onPositionChanged(int, int);
    void onCornerButtonPressed();
    void onDeleteRecord();
protected slots:
    void keyPressEvent(QKeyEvent*) override;
    void onFilterChanged(int, QString);
    void onFilterButtonPressed(int);
    void cornerButtonPressed();
public slots:
    void edit(const QModelIndex &index);
    void closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent*) Q_DECL_OVERRIDE;
    void onActDeleteRecord();
protected:
    PBSTableCornerButton cornerButton;

    QAction actDeleteRecord;

    bool bNextCellOnEnter = true,
         bNextRowOnLastCol = false,
         bGotoFirstCellOnNewRec = false,
         bAlternateRowColor = true,
         bSelectRow = false,
         bReadOnly = false,
         bAutoEdit = true,
         bColumnAutoWidth = true,
         bSortEnabled = true,
         bSectionMoveable = true,
         bFilterRowVisible = true,
         bFooterRowVisible = true,
         bQuickCustomize = true,
         bAskForDelete = true,
         bCanDeleteRow = true,
         bCanAddRow = true;

    PBSHeaderView pVertical, pHorizontal;

    bool edit(const QModelIndex &index, EditTrigger trigger, QEvent *event) Q_DECL_OVERRIDE;
    void selectionChanged(const QItemSelection &, const QItemSelection &) Q_DECL_OVERRIDE;

    virtual bool addNewRecord(const QModelIndex &index);
    virtual int nextVisibleIndex(int);

};

#endif // PBSTABLEWIDGET_H
