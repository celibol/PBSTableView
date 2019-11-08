#include "FilterLineEdit.h"

#include <QTimer>
#include <QKeyEvent>
#include <QAction>
#include <pbstools.h>

FilterLineEdit::FilterLineEdit(QWidget* parent, QList<FilterLineEdit*>* filters, int columnnum)
    : QLineEdit(parent),
      filterList(filters),
      columnNumber(columnnum),
      filterAction(0),
      filterButton(0)
{
    setObjectName("FILTEREDIT");
    setPlaceholderText(PBSTR("Filtre"));
    setProperty("column", columnnum);

    createFilterButton();

    delaySignalTimer = new QTimer(this);
    delaySignalTimer->setInterval(100);

    connect(this, SIGNAL(textChanged(QString)), delaySignalTimer, SLOT(start()));
    connect(delaySignalTimer, SIGNAL(timeout()), this, SLOT(delayedSignalTimerTriggered()));

//    connect(this, SIGNAL(editingFinished()), this, SLOT(delayedSignalTimerTriggered()));
}

void FilterLineEdit::createFilterButton()
{
    if(!filterAction)
    {
        const QIcon icon = QIcon(QPixmap(":/Resource/Images/filter.png"));
        filterButton = new QToolButton;

        filterButton->setObjectName("FILTERBUTTON");
        filterButton->setCursor(Qt::PointingHandCursor);
        filterButton->setFocusPolicy(Qt::NoFocus);
        filterButton->setStyleSheet("* { border: none; }");
        filterButton->setIcon(icon);        

        filterButton->setPopupMode(QToolButton::InstantPopup);
        connect(filterButton, SIGNAL(clicked()), SLOT(FilterBtnClick()));

        filterAction = new QWidgetAction(this);
        filterAction->setDefaultWidget(filterButton);
        addAction(filterAction, QLineEdit::TrailingPosition);

        filterButton->hide();
    }
}

void FilterLineEdit::focusSlot()
{
    setFocus();
}
void FilterLineEdit::delayedSignalTimerTriggered()
{
    QString s = text();

    setClearButtonEnabled(!s.isEmpty());

    if(s.compare("*") == 0) return;
    if(s.compare("%") == 0) return;

    delaySignalTimer->stop();
    emit delayedTextChanged(s);

    QFont f(font());
    f.setBold(!s.isEmpty());
    setFont(f);

    QTimer::singleShot(50, this, SLOT(focusSlot()));
}

void FilterLineEdit::keyReleaseEvent(QKeyEvent* event)
{
    if(event->key() == Qt::Key_Tab)
    {
        if(columnNumber < filterList->size() - 1)
        {
            filterList->at(columnNumber + 1)->setFocus();
            event->accept();
        }
    }
    else if(event->key() == Qt::Key_Backtab)
    {
        if(columnNumber > 0)
        {
            filterList->at(columnNumber - 1)->setFocus();
            event->accept();
        }
    }
}

void FilterLineEdit::clear()
{
    // When programatically clearing the line edit's value make sure the effects are applied immediately, i.e.
    // bypass the delayed signal timer
    QLineEdit::clear();
    delayedSignalTimerTriggered();
}

void FilterLineEdit::setText(const QString& text)
{
    QLineEdit::setText(text);
    delayedSignalTimerTriggered();
}

void FilterLineEdit::setFilterVisible(bool bVisible)
{
    if(bVisible)
        createFilterButton();
    else
    {
        if(filterButton)
        {
            delete filterButton;
            filterButton = nullptr;
        }
        if(filterAction)
        {
            delete filterAction;
            filterAction = nullptr;
        }
    }
}

void FilterLineEdit::focusInEvent(QFocusEvent *e)
{
    QLineEdit::focusInEvent(e);

    if(filterAction && filterButton)
        filterButton->show();
}

void FilterLineEdit::focusOutEvent(QFocusEvent *e)
{
    QLineEdit::focusOutEvent(e);

    if(filterAction && filterButton)
        filterButton->hide();
}

bool FilterLineEdit::FilterVisible()
{
    return filterAction ? !filterAction->isVisible() : false;
}

void FilterLineEdit::FilterBtnClick()
{
    emit FilterButtonClicked(columnNumber);
    filterButton->show();
}
