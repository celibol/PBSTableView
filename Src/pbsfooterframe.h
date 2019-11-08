#ifndef PBSFOOTERFRAME_H
#define PBSFOOTERFRAME_H

#include <QObject>
#include <QFrame>
#include <QMenu>
#include <QAction>

#include <pbstableviewtool.h>

class PBSCalculateThread;

class PBSFooterFrame : public QFrame
{
    Q_OBJECT

    QMenu *pMenu;

    QAction *pNone;
    QAction *pCount;
    QAction *pSum;
    QAction *pMin;
    QAction *pMax;
    QAction *pAvg;

    void onNone();
    void onCount();
    void onSum();
    void onMin();
    void onMax();
    void onAvg();
public:
    explicit PBSFooterFrame(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    ~PBSFooterFrame();

    void calculate();

    virtual QSize sizeHint() const override;

    QMenu *menu();
signals:
    void menuClicked(int);
    void onCreatePopupMenu(QMenu *);
    void onValueChanged();
public slots:
    void showCustomContextMenu(const QPoint&);
    void showEvent(QShowEvent*) Q_DECL_OVERRIDE;
    void sendMenuSignal();
    void valueChanged();
protected:
    PBSCalculateThread *pThread;

    int findCol();
    void execCol(footerEvent);
};

#endif // PBSFOOTERFRAME_H
