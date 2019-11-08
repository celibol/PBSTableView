#ifndef PBSHEADERVIEW_H
#define PBSHEADERVIEW_H

#include <QtWidgets>

struct ItemState;

class PBSFooterFrame;
class FilterLineEdit;

class PBSHeaderView : public QHeaderView
{
    Q_OBJECT
public:
    enum state { browse, edit };
    Q_ENUM(state)

    state headerState;

    explicit PBSHeaderView(Qt::Orientation orientation, QWidget *parent = Q_NULLPTR);
    ~PBSHeaderView();

    bool filterRowVisible();
    void setFilterRowVisible(bool);

    int sectionHeight();
    virtual QSize sizeHint() const override;
    QRect sectionRect(int logicalIndex) const;

    virtual void generateFilters(int number, bool showFirst = false);

    ItemState getItemState(int);

    FilterLineEdit *filterWidget(int iCol) const
    {
        return iCol < filterWidgets.size() ? filterWidgets[iCol] : nullptr;
    }
signals:
    void filterChanged(int, QString);
    void filterButtonPressed(int);
public slots:
    void adjustPositions();
    void sectionCountChanged(int oldCount, int newCount);
    void inputChanged(const QString &);
    void filterButtonClicked(int);
protected:
    QList<FilterLineEdit*> filterWidgets;

    PBSFooterFrame *footerFrame();
    int footerX();

    virtual void updateGeometries();    

    virtual void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const Q_DECL_OVERRIDE;
private:
    bool bFilterRow;
};

#endif // PBSHEADERVIEW_H
