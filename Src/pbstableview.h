#ifndef PBSTABLEVIEW_H
#define PBSTABLEVIEW_H

#include <QWidget>
#include <QPrinter>

#include <pbstableviewtool.h>
#include <pbstablewidget.h>
#include <pbsfooterframe.h>
#include <pbsfilterframe.h>

#define PBSTABLEVIEWCOLINDEXROLE Qt::UserRole + 1

class PBSTableView : public QWidget
{
    Q_OBJECT    
public:
    enum MenuTypes{
        HeaderMenu = 0,
        FooterMenu = 1
    };

    enum ItemStates{
        Sortable = 0,
        Filter = 1,
        Hidden = 2,
        Visible = 3,
        Editable = 4,
        Selectable = 5
    };

    explicit PBSTableView(QString sSettings, QWidget *parent = nullptr);

    void setModel(QAbstractItemModel *model);
    QAbstractItemModel *model() { return pModel; }

    void setItemDelegate(QAbstractItemDelegate *delegate);
    QAbstractItemDelegate *itemDelegate() const;

    ItemState itemState(int);
    itemStateMap &ItemStateMap();
    void setItemStateMap(int, ItemState);

    void setItemCaption(int, QString);

    void setItemFilter(int, QString, QString);
    void setItemFilter(int, QString);
    void setItemFilterBoxFilter(int, QString);

    void applyFilter();
    void reOpenQuery();

    PBSFooterFrame *FooterFrame();
    PBSTableWidget *tableWidget();

    void gotoRow(int);
    void gotoColumn(int);
    void moveTo(int, int);

    int currentRow();
    int currentColumn();
    QModelIndex currentPos();

    void setItemState(int, ItemStates, bool);
    void setItemState(int, bool select = true, bool edit = true, bool hidden = false, bool visible = true, bool filter = true, bool sort = true);
    void setItemFormat(int, QString = "", QString = "");

    void setItemAlignment(int, Qt::Alignment = Qt::AlignLeft, Qt::Alignment = Qt::AlignLeft);
    void setItemEditor(int, itemEditType);

    QVariant footerValue(int) const;

    void setFooterEvent(int, footerEvent);

    void saveState();
    void loadState();

    bool enableQuickCustomize();
    void setEnableQuickCustomize(bool);

    bool columnAutoWidth();
    void setColumnAutoWidth(bool);

    bool sortEnabled();
    void setSortEnabled(bool);

    void setFilterRowVisible(bool);
    bool filterRowVisible();

    void setFooterRowVisible(bool);
    bool footerRowVisible();

    bool autoEdit();
    void setAutoEdit(bool);

    bool readOnly();
    void setReadOnly(bool);

    bool selectRow();
    void setSelectRow(bool);

    bool nextCellOnEnter();
    void setNextCellOnEnter(bool b);

    bool nextRowOnLastCol();
    void setNextRowOnLastCol(bool b);

    bool gotoFirstCellOnNewRecord();
    void setGotoFirstCellOnNewRecord(bool b);

    QMenu &headerMenu();
    QMenu *footerMenu();

    bool canAddRow();
    void setCanAddRow(bool);

    bool canDeleteRow();
    void setCanDeleteRow(bool);

    bool askForDelete();
    void setAskForDelete(bool);

    virtual bool AskForDelete();
signals:    
    void onCreatePopupMenu(MenuTypes, QMenu *);
    void filterChanged(int, QString);
    void filterButtonPressed(int);
    void newRecordAdd();    
    void onPositionChanged(int, int);
    void onFooterValueChanged();
    void onCornerButtonPressed();
public slots:
    void onFilterChanged(int, QString);
    void onFilterButtonPressed(int);
    void showCustomContextMenu(const QPoint&);
    void showEvent(QShowEvent*) Q_DECL_OVERRIDE;
    void dataChanged(const QModelIndex &, const QModelIndex &);
    void onNewRecord();
    void positionChanged(int, int);
    void onCreateFooterMenu(QMenu *);
    void footerValueChanged();
    void cornerButtonPressed();
    void changeColumnVisible(QListWidgetItem*);
    void print(QPrinter *);
    void onDeleteRecord();
protected:    
    QString sSettingsName;
    itemStateMap tItemStateMap;
    PBSFilterFrame pFilterFrame;

    QVBoxLayout pLayout;

    PBSTableWidget pTableWidget;
    PBSFooterFrame pFooterFrame;
    QFrame customizeFrame;
    QListWidget customizeListWidget;

    QAbstractItemModel *pModel;

    bool bFilterRowVisible = true,
         bFooterRowVisible = true,
         bAskForDelete = true;

    QMenu popupMenu;
    QAction actFooter,
            actFilter,
            actSortAsc,
            actSortDesc,
            actStretchColumn,
            actStretchColumnAll,
            actStretch,
            actSaveLayout,
            actSaveExcel,
            actSaveExcelXML,
            actSaveWordXML,
            actSaveXML,
            actSavePDF,
            actSaveCSV,
            actSaveHTML,
            actSendToPrinter;

    virtual void createActions();
    virtual void createPopupMenu();

    void onActFooter();
    void onActFilter();
    void onActSortAsc();
    void onActSortDesc();
    void onActStretchColumn();
    void onActStretchColumnAll();
    void onActStretch();
    void onActSaveLayout();
    void onActSaveExcel();
    void onActSaveExcelXML();
    void onActSaveWordXML();
    void onActSaveXML();
    void onActSavePDF();
    void onActSaveCSV();
    void onActSaveHTML();
    void onActSendToPrinter();
};

#endif // PBSTABLEVIEW_H
