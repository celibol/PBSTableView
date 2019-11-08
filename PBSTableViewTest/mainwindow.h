#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>

#include <pbssqlitedatabase.h>
#include <pbsdatamodel.h>
#include <pbstableview.h>
#include <pbstableviewitemdelegate.h>

namespace Ui {
class MainWindow;
}

class MyDelegate : public PBSTableViewItemDelegate
{
    Q_OBJECT
public:
    explicit MyDelegate(PBSTableView *parent = 0) :
        PBSTableViewItemDelegate (parent)
    {}

    // Create Editor when we construct MyDelegate
    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {        
        QWidget *pWidget = PBSTableViewItemDelegate::createEditor(parent, option, index);
        switch(index.column())
        {
            case 3: // IntField
            {
                QSpinBox *editor = static_cast<QSpinBox*>(pWidget);
                editor->setMinimum(0);
                editor->setMaximum(10000);
                editor->setAlignment(Qt::AlignRight);
            }
            break;

            case 4: // DEcimalField
            {
                QDoubleSpinBox *editor = static_cast<QDoubleSpinBox*>(pWidget);
                editor->setMinimum(0);
                editor->setMaximum(10000);
                editor->setDecimals(5);
                editor->setAlignment(Qt::AlignRight);
            }
            break;

            case 5: // dateField
            {
                QDateEdit *editor = static_cast<QDateEdit*>(pWidget);
                editor->setCalendarPopup(true);
                editor->setDisplayFormat("dd.MM.yyyy");
            }
            break;
        }

        return pWidget;
    }

    // Then, we set the Editor
    void setEditorData(QWidget *editor, const QModelIndex &index) const
    {
        switch(index.column())
        {
            case 5:
            {
                QString sD(index.model()->data(index, Qt::EditRole).toString());
                QStringList sl = sD.split("-");

                if(sl.count() > 2)
                {
                    QDate value(QDate::fromString(sD));
                    QDate value2(sl.at(0).toInt(), sl.at(1).toInt(), sl.at(2).toInt());
                    // qDebug() << "String" << sD << "List" << sl << "TARIH" << value << "TARIH2" << value2;

                    QDateEdit *de = static_cast<QDateEdit*>(editor);
                    if(de)
                        de->setDate(value2);
                }
            }
            break;

            default:
            {
                PBSTableViewItemDelegate::setEditorData(editor, index);
            }
            break;
        }
    }
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    PBSTableView pTableView;
};

#endif // MAINWINDOW_H
