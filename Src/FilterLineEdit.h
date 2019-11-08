#ifndef FILTERLINEEDIT_H
#define FILTERLINEEDIT_H

#include <QLineEdit>
#include <QToolBUtton>
#include <QList>
#include <QWidgetAction>

class QTimer;
class QKeyEvent;

class FilterLineEdit : public QLineEdit
{
    Q_OBJECT

    QWidgetAction *filterAction;
    QToolButton *filterButton;
public:
    explicit FilterLineEdit(QWidget* parent, QList<FilterLineEdit*>* filters, int columnnum);

    // Override methods for programatically changing the value of the line edit
    void clear();
    void setText(const QString& text);

    void setFilterVisible(bool bVisible);
    bool FilterVisible();
private slots:
    void delayedSignalTimerTriggered();
    void FilterBtnClick();
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);
    void focusSlot();
signals:
    void delayedTextChanged(QString text);
    void FilterButtonClicked(int);
protected:
    void keyReleaseEvent(QKeyEvent* event);

private:
    QList<FilterLineEdit*>* filterList;
    int columnNumber;
    QTimer* delaySignalTimer;
    QAction *filterQction;

    void createFilterButton();
};

#endif
