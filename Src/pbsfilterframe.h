#ifndef PBSFILTERFRAME_H
#define PBSFILTERFRAME_H

#include <QtWidgets>
#include <pbstableviewtool.h>
#include <pbsdatamodel.h>

class PBSTableView;

class PBSFilterFrame : public QFrame
{
    Q_OBJECT
public:
    PBSFilterFrame(QWidget *parent = Q_NULLPTR);
    ~PBSFilterFrame();

    void build(PBSTableView *, int);
protected slots:
    void clearFilter();
    void emptyFilter();
    void nonEmptyFilter();
    void applyFilter();
    void filterEditChanged(const QString&);
    void showEvent(QShowEvent*);
    void selectChanged(int);
protected:
    int currentCol;
    QString currentFieldName;

    PBSTableView *pTableView;
    PBSDataModel *pModel;
    QSortFilterProxyModel *proxyModel;
    QListView *columnListWidget;
    QCheckBox *selCB;
    itemStateMap tFilterItemStateMap;

    virtual void buildLayout();
    virtual void deleteChildren();
};

#endif // PBSFILTERFRAME_H
