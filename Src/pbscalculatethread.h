#ifndef PBSCALCULATETHREAD_H
#define PBSCALCULATETHREAD_H

#include <QObject>
#include <QThread>

#include <pbstableview.h>

class PBSCalculateThread : public QThread
{
    Q_OBJECT
public:
    PBSTableView *pTableView = Q_NULLPTR;

    PBSCalculateThread(QObject * = Q_NULLPTR);

    void calculate(int = -1);
signals:
    void valueChanged();
protected:
    QString sRunQuery;
    void run() override;

    QString buildCol(int);
};

#endif // PBSCALCULATETHREAD_H
