#-------------------------------------------------
#
# Project created by QtCreator 2019-08-07T15:14:38
#
#-------------------------------------------------

QT       += core gui sql xml printsupport widgets-private gui-private core-private

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PBSTableViewTest
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += ../Src
INCLUDEPATH += ../Src/pbs
INCLUDEPATH += ../Src/pbsDatabase

INCLUDEPATH += ../Src/xlsx
INCLUDEPATH += ../Src/xlsx/header

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    ../Src/pbsDatabase/pbsdatabase.cpp \
    ../Src/pbsDatabase/pbsdbtools.cpp \
    ../Src/pbsDatabase/pbssqlitedatabase.cpp \
    ../Src/pbsdatamodel.cpp \
    ../Src/pbs/pbstools.cpp \
    ../Src/pbs/KoOdfNumberStyles.cpp \
    ../Src/pbstablewidget.cpp \
    ../Src/pbstableview.cpp \
    ../Src/pbsheaderview.cpp \
    ../Src/FilterLineEdit.cpp \
    ../Src/pbsfooterframe.cpp \
    ../Src/pbscalculatethread.cpp \
    ../Src/pbsfilterframe.cpp \
    ../Src/pbsappsettings.cpp \
    ../Src/pbsmodelexporter.cpp \
    ../Src/pbs/pbsfileutil.cpp \
    ../Src/pbstableprinter.cpp \
    ../Src/pbstableviewitemdelegate.cpp \
    ../Src/xlsx/source/xlsxabstractooxmlfile.cpp \
    ../Src/xlsx/source/xlsxabstractsheet.cpp \
    ../Src/xlsx/source/xlsxcell.cpp \
    ../Src/xlsx/source/xlsxcellformula.cpp \
    ../Src/xlsx/source/xlsxcelllocation.cpp \
    ../Src/xlsx/source/xlsxcellrange.cpp \
    ../Src/xlsx/source/xlsxcellreference.cpp \
    ../Src/xlsx/source/xlsxchart.cpp \
    ../Src/xlsx/source/xlsxchartsheet.cpp \
    ../Src/xlsx/source/xlsxcolor.cpp \
    ../Src/xlsx/source/xlsxconditionalformatting.cpp \
    ../Src/xlsx/source/xlsxcontenttypes.cpp \
    ../Src/xlsx/source/xlsxdatavalidation.cpp \
    ../Src/xlsx/source/xlsxdatetype.cpp \
    ../Src/xlsx/source/xlsxdocpropsapp.cpp \
    ../Src/xlsx/source/xlsxdocpropscore.cpp \
    ../Src/xlsx/source/xlsxdocument.cpp \
    ../Src/xlsx/source/xlsxdrawing.cpp \
    ../Src/xlsx/source/xlsxdrawinganchor.cpp \
    ../Src/xlsx/source/xlsxformat.cpp \
    ../Src/xlsx/source/xlsxmediafile.cpp \
    ../Src/xlsx/source/xlsxnumformatparser.cpp \
    ../Src/xlsx/source/xlsxrelationships.cpp \
    ../Src/xlsx/source/xlsxrichstring.cpp \
    ../Src/xlsx/source/xlsxsharedstrings.cpp \
    ../Src/xlsx/source/xlsxsimpleooxmlfile.cpp \
    ../Src/xlsx/source/xlsxstyles.cpp \
    ../Src/xlsx/source/xlsxtheme.cpp \
    ../Src/xlsx/source/xlsxutility.cpp \
    ../Src/xlsx/source/xlsxworkbook.cpp \
    ../Src/xlsx/source/xlsxworksheet.cpp \
    ../Src/xlsx/source/xlsxzipreader.cpp \
    ../Src/xlsx/source/xlsxzipwriter.cpp

HEADERS += \
        mainwindow.h \
    ../Src/pbsDatabase/pbsdatabase.h \
    ../Src/pbsDatabase/pbsdbtools.h \
    ../Src/pbsDatabase/pbssqlitedatabase.h \
    ../Src/pbstableviewtool.h \
    ../Src/pbsdatamodel.h \
    ../Src/pbs/pbstools.h \
    ../Src/pbs/KoOdfNumberStyles.h \
    ../Src/pbstablewidget.h \
    ../Src/pbstableview.h \
    ../Src/pbsheaderview.h \
    ../Src/FilterLineEdit.h \
    ../Src/pbsfooterframe.h \
    ../Src/pbscalculatethread.h \
    ../Src/pbsfilterframe.h \
    ../Src/pbsappsettings.h \
    ../Src/pbsmodelexporter.h \
    ../Src/pbs/pbsfileutil.h \
    ../Src/pbstableprinter.h \
    ../Src/pbstableviewitemdelegate.h \
    ../Src/xlsx/header/xlsxabstractooxmlfile.h \
    ../Src/xlsx/header/xlsxabstractooxmlfile_p.h \
    ../Src/xlsx/header/xlsxabstractsheet.h \
    ../Src/xlsx/header/xlsxabstractsheet_p.h \
    ../Src/xlsx/header/xlsxcell.h \
    ../Src/xlsx/header/xlsxcell_p.h \
    ../Src/xlsx/header/xlsxcellformula.h \
    ../Src/xlsx/header/xlsxcellformula_p.h \
    ../Src/xlsx/header/xlsxcelllocation.h \
    ../Src/xlsx/header/xlsxcellrange.h \
    ../Src/xlsx/header/xlsxcellreference.h \
    ../Src/xlsx/header/xlsxchart.h \
    ../Src/xlsx/header/xlsxchart_p.h \
    ../Src/xlsx/header/xlsxchartsheet.h \
    ../Src/xlsx/header/xlsxchartsheet_p.h \
    ../Src/xlsx/header/xlsxcolor_p.h \
    ../Src/xlsx/header/xlsxconditionalformatting.h \
    ../Src/xlsx/header/xlsxconditionalformatting_p.h \
    ../Src/xlsx/header/xlsxcontenttypes_p.h \
    ../Src/xlsx/header/xlsxdatavalidation.h \
    ../Src/xlsx/header/xlsxdatavalidation_p.h \
    ../Src/xlsx/header/xlsxdatetype.h \
    ../Src/xlsx/header/xlsxdocpropsapp_p.h \
    ../Src/xlsx/header/xlsxdocpropscore_p.h \
    ../Src/xlsx/header/xlsxdocument.h \
    ../Src/xlsx/header/xlsxdocument_p.h \
    ../Src/xlsx/header/xlsxdrawing_p.h \
    ../Src/xlsx/header/xlsxdrawinganchor_p.h \
    ../Src/xlsx/header/xlsxformat.h \
    ../Src/xlsx/header/xlsxformat_p.h \
    ../Src/xlsx/header/xlsxglobal.h \
    ../Src/xlsx/header/xlsxmediafile_p.h \
    ../Src/xlsx/header/xlsxnumformatparser_p.h \
    ../Src/xlsx/header/xlsxrelationships_p.h \
    ../Src/xlsx/header/xlsxrichstring.h \
    ../Src/xlsx/header/xlsxrichstring_p.h \
    ../Src/xlsx/header/xlsxsharedstrings_p.h \
    ../Src/xlsx/header/xlsxsimpleooxmlfile_p.h \
    ../Src/xlsx/header/xlsxstyles_p.h \
    ../Src/xlsx/header/xlsxtheme_p.h \
    ../Src/xlsx/header/xlsxutility_p.h \
    ../Src/xlsx/header/xlsxworkbook.h \
    ../Src/xlsx/header/xlsxworkbook_p.h \
    ../Src/xlsx/header/xlsxworksheet.h \
    ../Src/xlsx/header/xlsxworksheet_p.h \
    ../Src/xlsx/header/xlsxzipreader_p.h \
    ../Src/xlsx/header/xlsxzipwriter_p.h

FORMS += \
        mainwindow.ui

RESOURCES += \
    pbslib.qrc
