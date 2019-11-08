#ifndef PBSTABLEVIEWITEMDELEGATE_H
#define PBSTABLEVIEWITEMDELEGATE_H

#include <QItemDelegate>

#ifndef PBSTABLEVIEW_H
#include <pbstableview.h>
#endif

#include <pbstableviewtool.h>

class PBSTableViewItemDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    PBSTableViewItemDelegate(PBSTableView *parent = nullptr);

    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
signals:
    void buttonPressed(QModelIndex);
protected:
    itemStateMap &ItemStateMap() const;

    QSpinBox *createIntSpinBox();
    QDoubleSpinBox *createDoubleSpinBox();

    void paintProgress(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paintButton(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // PBSTABLEVIEWITEMDELEGATE_H
