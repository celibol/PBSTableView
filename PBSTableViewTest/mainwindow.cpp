#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <pbstableviewitemdelegate.h>

PBSSQLiteDatabase pTempDatabase("TEMPORARYTEST", ":memory:");

class myModel : public PBSDataModel
{
public:
    explicit myModel(QObject *parent = nullptr) :
        PBSDataModel(parent, pTempDatabase.SQLDatabase(),
                     QString("Select ID, ORDERNO, STRFIELD, INTFIELD, DECIMALFIELD, DATETIMEFIELD, RESIM FROM TEMPTEST"))
    {
        setUpdateTableName("TEMPTEST");

        sKeyFieldName = "ID";

        openQuery();

        record().field(1).setDefaultValue(1234);
        record().field(2).setDefaultValue("STRFİELD");        
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role) const
    {
        if(orientation == Qt::Horizontal && section == 1 && role == Qt::DisplayRole)
        {
            // return "Order No";
        }
        return PBSDataModel::headerData(section, orientation, role);
    }

    virtual bool isInsertableField(QString n) override
    {
        return n.compare("ID") == 0 ? false : PBSDataModel::isInsertableField(n);
    }

    virtual bool isUpdatableField(QString n)
    {
        return n.compare("ID") == 0 ? false : PBSDataModel::isInsertableField(n);
    }

    virtual QSqlRecord newRecord() override
    {
        QSqlRecord r(record());

        QSqlField f = r.field("ID");
        f.setAutoValue(true);
        f.setReadOnly(true);

        // r.field("ID").setReadOnly(true);
        r.field("INTFIELD").setReadOnly(false);
        r.field("STRFIELD").setReadOnly(false);

        r.setValue("INTFIELD", 1234);
        r.setValue(2, "STRFİELD");

        r.setValue(5, QDate::currentDate());

        // for(int i = 0; i < r.count(); i++)
        //     qDebug() << r.field("INTFIELD").isReadOnly() << "OK" << r.fieldName(i) << r.field(i).value() << r.field(i).defaultValue();

        return r;
    }

    QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override
    {        
        if(role == Qt::DisplayRole && idx.column() == 5)
        {
            QString sD = data(idx, Qt::EditRole).toString();
            QStringList sl = sD.split("-");            
            if(sl.count() > 2)
            {
                QDate vD(sl.at(0).toInt(), sl.at(1).toInt(), sl.at(2).toInt());
                // qDebug() << "Display Date" << sD << sl.at(0).toInt() << sl.at(1).toInt() << sl.at(2).toInt() << sl.count();
                if(vD.isValid())
                {
                    QString sRet(formatValue(idx, QVariant::fromValue(vD), QString("dd.MM.yyyy"))); //   vD.toString("dd.MM.yyyy"));
                    return sRet;
                }
            }
        }
        return PBSDataModel::data(idx, role);
    }
};

void createTempTable()
{
    pTempDatabase.execSQL("DROP TABLE IF EXISTS TEMPTEST");
    pTempDatabase.execSQL("CREATE TABLE TEMPTEST(" \
       "ID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, " \
       "ORDERNO INT DEFAULT 12, " \
       "STRFIELD VARCHAR(64), " \
       "INTFIELD INT DEFAULT 13, " \
       "DECIMALFIELD REAL DEFAULT 5.25, " \
       "DATETIMEFIELD DATETIME, " \
       "RESIM BLOB " \
       ")", true
    );

    pTempDatabase.execSQL(QString("INSERT INTO TEMPTEST(STRFIELD, INTFIELD, DECIMALFIELD, DATETIMEFIELD, ORDERNO) " \
          " VALUES(NULL, %1, %2, NULL, 75)").arg(500).arg(125));

    QScreen *screen = qApp->primaryScreen();
    QPixmap inPixmap = screen->grabWindow( 0 );
    QByteArray inByteArray;
    QBuffer inBuffer( &inByteArray );
    inBuffer.open( QIODevice::WriteOnly );
    inPixmap.save( &inBuffer, "PNG" ); // write inPixmap into inByteArray in PNG format

    for(int y = 0; y < 24; y++)
    {
        for(int i = 0; i < 5; i++)
        {
            int x = ((i + 1) * y) > 128 ? i % 128 : i;
            qreal r = x * 1.2343;

            int d = (i > 27 ? i % 27 : i) + 1, m = (i > 11 ? i % 11 : i) + 1;

            QString sSQL(QString("INSERT INTO TEMPTEST(STRFIELD, INTFIELD, DECIMALFIELD, DATETIMEFIELD, ORDERNO, RESIM) " \
                                 " VALUES('STRING %1', %2, %3, '2018-%4-%5', %6, :PIC)").arg(y).arg(x * 1000).arg(r).arg(m).arg(d).arg(i));

            QSqlQuery q(pTempDatabase.SQLDatabase());
            q.bindValue(":PIC", inByteArray);

            if(q.prepare(sSQL))
            {
                // qDebug() << "Exec" << sSQL;
                if(!q.exec())
                    qDebug() << "Exec Error" << q.lastError().text();
            }
             else
                qDebug() << "Prepare Error" << pTempDatabase.SQLDatabase().lastError().text();
        }
    }

    QSqlQuery q("SELECT COUNT(*) FROM TEMPTEST", pTempDatabase.SQLDatabase());

    if(q.next())
        qDebug() << "Toplam Eklenen Kayıt" << q.record().value(0);

    QSqlQuery q1("Select ID, ORDERNO, STRFIELD, INTFIELD, DECIMALFIELD, DATETIMEFIELD FROM TEMPTEST", pTempDatabase.SQLDatabase());
    if(q1.next())
        qDebug() << "Okunan Kayıt" << q1.record().value(3);

}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    pTableView("PBSTableView", this)
{
    ui->setupUi(this);
    setWindowTitle("PBS TableView Test");

    if(!pTempDatabase.SQLDatabase().isOpen())
        pTempDatabase.open();

    createTempTable();

    myModel *model = new myModel(&pTableView);
    model->fetchMore();

    qDebug() << "Toplam:" << model->rowCount() << " Kayıt" << pTempDatabase.lastError();

    QFont fb("Tahoma", 10);
    QFont cb("Courier New", 10);

    pTableView.tableWidget()->horizontalHeader()->setFont(fb);
    pTableView.setFont(cb);

    pTableView.setNextCellOnEnter(true);
    pTableView.setNextRowOnLastCol(true);

    pTableView.setAskForDelete(true);
    pTableView.setCanDeleteRow(true);
    pTableView.setCanAddRow(true);

    pTableView.setItemCaption(1, "Order No");

    pTableView.setItemState(0, false, false, false, false, false, false); // ID Kolonu
    pTableView.setItemState(0, PBSTableView::Hidden, false);

    pTableView.setItemFormat(1, "#,#0", "#,#0");
    pTableView.setItemFormat(3, "#,#0", "#,#0");
    pTableView.setItemFormat(4, "#,#0.#0", "#,#0.###0");
    pTableView.setItemFormat(5, "dd.MMMM.yyy", "#,#0");

    pTableView.setItemAlignment(0, Qt::AlignVCenter | Qt::AlignRight, Qt::AlignVCenter | Qt::AlignRight);
    pTableView.setItemAlignment(1, Qt::AlignVCenter | Qt::AlignRight, Qt::AlignVCenter | Qt::AlignRight);
    pTableView.setItemAlignment(2, Qt::AlignVCenter | Qt::AlignLeft, Qt::AlignVCenter | Qt::AlignRight);
    pTableView.setItemAlignment(3, Qt::AlignVCenter | Qt::AlignRight, Qt::AlignVCenter | Qt::AlignRight);
    pTableView.setItemAlignment(4, Qt::AlignVCenter | Qt::AlignRight, Qt::AlignVCenter | Qt::AlignCenter);
    pTableView.setItemAlignment(5, Qt::AlignVCenter | Qt::AlignLeft, Qt::AlignVCenter | Qt::AlignLeft);

    pTableView.setFooterEvent(1, skSum);
    pTableView.setFooterEvent(2, skCount);
    pTableView.setFooterEvent(3, skSum);

    pTableView.setModel(model);

    // QString("Select ID, ORDERNO, STRFIELD, INTFIELD, DECIMALFIELD, DATETIMEFIELD, RESIM FROM TEMPTEST"))

    pTableView.setItemEditor(1, etProgress);
    pTableView.setItemEditor(2, etButton);
    pTableView.setItemEditor(3, etIntSpin);
    pTableView.setItemEditor(4, etDoubleSpin);
    pTableView.setItemEditor(5, etDate);

    MyDelegate *myDelegate= new MyDelegate(&pTableView);
    connect(myDelegate,  &MyDelegate::buttonPressed, [=](QModelIndex index)
    {
        qDebug() << "BUtton Pressed" << index;
    });

    pTableView.setItemDelegate(myDelegate);

    // pLayout.setSizeConstraint(QLayout::SetMaximumSize);

    QHBoxLayout *pLayout = new QHBoxLayout;

    QFrame *pFrame = new QFrame(this);
    pFrame->setFixedWidth(250);

    QVBoxLayout *fLayout = new QVBoxLayout;
    fLayout->setDirection(QBoxLayout::TopToBottom);

    QCheckBox *pFilterRowCheck = new QCheckBox("Filter Row Visible");
    connect(pFilterRowCheck,  &QCheckBox::stateChanged, [=](int state)
    {
        pTableView.setFilterRowVisible(state != 0);
    });
    fLayout->addWidget(pFilterRowCheck);

    QCheckBox *pFooterCheck = new QCheckBox("Footer Row Visible");
    connect(pFooterCheck,  &QCheckBox::stateChanged, [=](int state)
    {
        pTableView.setFooterRowVisible(state != 0);
    });
    fLayout->addWidget(pFooterCheck);

    QCheckBox *pColumnAutoWidth = new QCheckBox("Column Auto Width");
    connect(pColumnAutoWidth,  &QCheckBox::stateChanged, [=](int state)
    {
        pTableView.setColumnAutoWidth(state != 0);
    });
    fLayout->addWidget(pColumnAutoWidth);

    QCheckBox *pSortEnabled = new QCheckBox("Sort Enabled");
    pSortEnabled->setCheckState(Qt::Checked);
    connect(pSortEnabled,  &QCheckBox::stateChanged, [=](int state)
    {
        pTableView.setSortEnabled(state != 0);
    });
    fLayout->addWidget(pSortEnabled);

    QCheckBox *pAutoEdit = new QCheckBox("Auto Edit");
    connect(pAutoEdit,  &QCheckBox::stateChanged, [=](int state)
    {
        pTableView.setAutoEdit(state != 0);
    });
    fLayout->addWidget(pAutoEdit);

    QCheckBox *pReadOnly = new QCheckBox("Read Only");
    connect(pReadOnly,  &QCheckBox::stateChanged, [=](int state)
    {
        pTableView.setReadOnly(state != 0);
    });
    fLayout->addWidget(pReadOnly);

    QCheckBox *pSelectRow = new QCheckBox("Select Row");
    connect(pSelectRow,  &QCheckBox::stateChanged, [=](int state)
    {
        pTableView.setSelectRow(state != 0);
    });
    fLayout->addWidget(pSelectRow);

    QCheckBox *pNextColEnter = new QCheckBox("Next Col On Enter");
    pNextColEnter->setCheckState(Qt::Checked);
    connect(pNextColEnter,  &QCheckBox::stateChanged, [=](int state)
    {
        pTableView.setNextCellOnEnter(state != 0);
    });
    fLayout->addWidget(pNextColEnter);

    QCheckBox *pNextRowLastCol = new QCheckBox("Next Row On Last Col");
    pNextRowLastCol->setCheckState(Qt::Checked);
    connect(pNextRowLastCol,  &QCheckBox::stateChanged, [=](int state)
    {
        pTableView.setNextRowOnLastCol(state != 0);
    });
    fLayout->addWidget(pNextRowLastCol);

    QCheckBox *pFirstRowNewRec = new QCheckBox("Goto First Col On New Record");
    connect(pFirstRowNewRec,  &QCheckBox::stateChanged, [=](int state)
    {
        pTableView.setGotoFirstCellOnNewRecord(state != 0);
    });
    fLayout->addWidget(pFirstRowNewRec);

    QCheckBox *pCinsiColReadOnly = new QCheckBox("Order No Col Read Only");
    connect(pCinsiColReadOnly,  &QCheckBox::stateChanged, [=](int state)
    {
        pTableView.setItemState(1, PBSTableView::Editable, state == 0);
    });
    fLayout->addWidget(pCinsiColReadOnly);

    QCheckBox *pEnableQuickCustomize = new QCheckBox("Quick Customize");
    pEnableQuickCustomize->setChecked(true);
    connect(pEnableQuickCustomize,  &QCheckBox::stateChanged, [=](int state)
    {
        pTableView.setEnableQuickCustomize(state == 2);
    });
    fLayout->addWidget(pEnableQuickCustomize);

    QCheckBox *pCanAddRowCheck = new QCheckBox("Add Row");
    pCanAddRowCheck->setChecked(true);
    connect(pCanAddRowCheck,  &QCheckBox::stateChanged, [=](int state)
    {
        pTableView.setCanAddRow(state != 0);
    });
    fLayout->addWidget(pCanAddRowCheck);

    QCheckBox *pCanDeleteRowCheck = new QCheckBox("Delete Row");
    pCanDeleteRowCheck->setChecked(true);
    connect(pCanDeleteRowCheck,  &QCheckBox::stateChanged, [=](int state)
    {
        qDebug() << "Can Delete" << (state != 0);
        pTableView.setCanDeleteRow(state != 0);
    });
    fLayout->addWidget(pCanDeleteRowCheck);

    QCheckBox *pAskForDeleteCheck = new QCheckBox("Ask For Delete");
    pAskForDeleteCheck->setChecked(true);
    connect(pAskForDeleteCheck,  &QCheckBox::stateChanged, [=](int state)
    {
        pTableView.setAskForDelete(state != 0);
    });
    fLayout->addWidget(pAskForDeleteCheck);

    fLayout->addStretch();

    QLabel *pLabel = new QLabel("Position");

    connect(&pTableView, &PBSTableView::onPositionChanged, [=](int r, int c){
        pLabel->setText(QString("Pos: %1:%2").arg(r).arg(c));
    });
    fLayout->addWidget(pLabel);

    QLabel *pLabel2 = new QLabel("Values");

    connect(&pTableView, &PBSTableView::onFooterValueChanged, [=](){
        pLabel2->setText(QString("3.Value: %1").arg(pTableView.footerValue(3).toDouble()));
    });
    fLayout->addWidget(pLabel2);

    pFrame->setLayout(fLayout);

    pLayout->addWidget(pFrame);
    pLayout->addWidget(&pTableView);

    centralWidget()->setLayout(pLayout);

    resize(800, 800);    
}

MainWindow::~MainWindow()
{
    delete ui;
}
