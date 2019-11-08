#include "pbstableviewitemdelegate.h"
#include <pbstableview.h>

PBSTableViewItemDelegate::PBSTableViewItemDelegate(PBSTableView *parent) :
    QItemDelegate (parent)
{

}

itemStateMap &PBSTableViewItemDelegate::ItemStateMap() const
{
    PBSTableView *pTableView = static_cast<PBSTableView*>(parent());
    if(pTableView)
        return pTableView->ItemStateMap();

    itemStateMap ism;
    return ism;
}

QWidget* PBSTableViewItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    itemStateMap ism = ItemStateMap();

    switch(ism[index.column()].editType)
    {
        case etButton:
        case etLineEdit  : return new QLineEdit(parent);

        case etMemo      : return new QTextEdit(parent);
        case etCheckBox  : return new QCheckBox(parent);

        case etProgress:
        case etIntSpin   : return new QSpinBox(parent);

        case etDoubleSpin: return new QDoubleSpinBox(parent);
        case etDate      : return new QDateEdit(parent);
        case etDateTime  : return new QDateTimeEdit(parent);
        case etPicture   : return Q_NULLPTR;
    }
    return QItemDelegate::createEditor(parent, option, index);
}

void PBSTableViewItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    itemStateMap ism = ItemStateMap();
    QVariant value = index.model()->data(index, Qt::EditRole);
    qDebug() << "Set Editor Data" << index.column() << "Value" << value;

    switch(ism[index.column()].editType)
    {
        case etButton:
        case etLineEdit:
        {
            QLineEdit *pEdit = static_cast<QLineEdit*>(editor);
            if(pEdit)
                pEdit->setText(value.toString());
        }
        break;

        case etMemo:
        {
            QTextEdit *pEdit = static_cast<QTextEdit*>(editor);
            if(pEdit)
                pEdit->setText(value.toString());
        }
        break;

        case etCheckBox:
        {
            QCheckBox *pEdit = static_cast<QCheckBox*>(editor);
            if(pEdit)
                pEdit->setChecked(value.toInt() == 1);
        }
        break;

        case etProgress:
        case etIntSpin:
        {
            QSpinBox *pEdit = static_cast<QSpinBox*>(editor);
            if(pEdit)
                pEdit->setValue(value.toInt());
        }
        break;

        case etDoubleSpin:
        {
            qDebug() << "Double Spin Value";
            QDoubleSpinBox *pEdit = static_cast<QDoubleSpinBox*>(editor);
            if(pEdit)
                pEdit->setValue(value.toDouble());
        }
        break;

        case etDate:
        {
            QDateEdit *pEdit = static_cast<QDateEdit*>(editor);
            if(pEdit)
                pEdit->setDate(value.toDate());
        }
        break;

        case etDateTime:
        {
            QDateTimeEdit *pEdit = static_cast<QDateTimeEdit*>(editor);
            if(pEdit)
                pEdit->setDateTime(value.toDateTime());
        }
        break;

        case etPicture: ;
    }
}

void PBSTableViewItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    itemStateMap ism = ItemStateMap();

    switch(ism[index.column()].editType)
    {
        case etButton:
        case etLineEdit:
        {
            QLineEdit *pEdit = static_cast<QLineEdit*>(editor);
            if(pEdit)
                model->setData(index, pEdit->text(), Qt::EditRole);
        }
        break;

        case etMemo:
        {
            QTextEdit *pEdit = static_cast<QTextEdit*>(editor);
            if(pEdit)
                model->setData(index, pEdit->toPlainText(), Qt::EditRole);
        }
        break;

        case etCheckBox:
        {
            QCheckBox *pEdit = static_cast<QCheckBox*>(editor);
            if(pEdit)
                model->setData(index, pEdit->isChecked() ? 1 : 0, Qt::EditRole);
        }
        break;

        case etProgress:
        case etIntSpin:
        {
            QSpinBox *pEdit = static_cast<QSpinBox*>(editor);
            if(pEdit)
            {
                pEdit->interpretText();
                model->setData(index, pEdit->value(), Qt::EditRole);
            }
        }
        break;

        case etDoubleSpin:
        {
            QDoubleSpinBox *pEdit = static_cast<QDoubleSpinBox*>(editor);
            if(pEdit)
            {
                pEdit->interpretText();
                model->setData(index, pEdit->value(), Qt::EditRole);
            }
        }
        break;

        case etDate:
        {
            QDateEdit *pEdit = static_cast<QDateEdit*>(editor);
            if(pEdit)
            {
                pEdit->interpretText();
                model->setData(index, pEdit->date(), Qt::EditRole);
            }
        }
        break;

        case etDateTime:
        {
            QDateTimeEdit *pEdit = static_cast<QDateTimeEdit*>(editor);
            if(pEdit)
            {
                pEdit->interpretText();
                model->setData(index, pEdit->dateTime(), Qt::EditRole);
            }
        }
        break;

        case etPicture: ;
    }
}

void PBSTableViewItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}

bool PBSTableViewItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonPress)
    {
        itemStateMap ism = ItemStateMap();
        if(ism[index.column()].editType == etButton)
        {
            emit buttonPressed(index);
        }
    }

    return QItemDelegate::editorEvent(event, model, option, index);
}

void PBSTableViewItemDelegate::paintProgress(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    int value = index.model()->data(index).toInt();

    QStyleOptionProgressBarV2 progressBarOption;
    progressBarOption.rect = option.rect.adjusted(4, 4, -4, -4);
    progressBarOption.minimum = 0;
    progressBarOption.maximum = 100;
    progressBarOption.textAlignment = Qt::AlignRight;
    progressBarOption.textVisible = true;
    progressBarOption.progress = value;
    progressBarOption.text = tr("%1%").arg(progressBarOption.progress);

    painter->save();
    if (option.state & QStyle::State_Selected)
    {
        painter->fillRect(option.rect, option.palette.highlight());
        painter->setBrush(option.palette.highlightedText());
    }

    QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);

    painter->restore();
}

void PBSTableViewItemDelegate::paintButton(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString value = index.model()->data(index).toString();

    QStyleOptionButton button;
    button.text = value;
    button.rect = option.rect.adjusted(4, 4, -4, -4);
    button.state |= QStyle::State_Enabled;

    painter->save();

    if (option.state & QStyle::State_Selected)
        painter->fillRect(option.rect, option.palette.highlight());

    painter->restore();

    QApplication::style()->drawControl(QStyle::CE_PushButton, &button, painter);
}

void PBSTableViewItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    itemStateMap ism = ItemStateMap();

    switch(ism[index.column()].editType)
    {
        case etProgress: paintProgress(painter, option, index); break;
        case etButton  : paintButton(painter, option, index); break;

        default        : QItemDelegate::paint (painter, option, index);
    }
}
