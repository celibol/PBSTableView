#ifndef FILEUTIL_H
#define FILEUTIL_H

#include <QObject>
#include <QMap>
#include <QIODevice>
#include <QStringList>
#include <QProcessEnvironment>
#include <QFileDialog>


#ifndef QL1S
#define QL1S(x) QLatin1String(x)
#endif

#ifndef QL1C
#define QL1C(x) QLatin1Char(x)
#endif

struct PBSFileUtil
{
    static QString extractPath(const QString &fileName);
    static QString extractName(const QString &fileName);
    static QString extractOnlyName(const QString &fileName);
    static QString extractExtension(const QString &fileName);
    static QString changeExtension(const QString &fileName, const QString &newExt);
    static qint64 size(const QString &fileName);

    static bool compareFile(const QString &fileName1, const QString &fileName2, bool canonical = true);

    static QMap<QString,QStringList> readFileContext(QIODevice *dev);
    static QString readAllFileContents(const QString &filename);
    static QByteArray readAllFileByteContents(const QString &filename);

    static QString findExecute(const QString &target);
    static QString canExec(QString file, QStringList exts = QStringList());
    static QString lookPath(const QString &fileName,const QProcessEnvironment &env, bool bLocalPriority);
    static QString lookPathInDir(const QString &fileName,const QString &dir);
    static bool startProcess(const QString &cmd, const QStringList args = QStringList(), const QString workDir = QString());

    static QStringList removeFiles(const QStringList &files);
    static QStringList removeWorkDir(const QString &workDir, const QStringList &filters);
    static bool Exist(const QString &src);
    static bool Rename(const QString &src, const QString &dest);
    static bool Copy(const QString &src, const QString &dest);
    static bool Move(const QString &src, const QString &dest);
    static bool CopyDirectory(const QString &src, const QString &dest);
    static void openInExplorer(const QString &path);

    static QString getExistingDirectory(const QString &name, QWidget* parent = 0, const QString &caption = QString(), const QString &dir = QString(), QFileDialog::Options options = QFileDialog::ShowDirsOnly);
    static QString getOpenFileName(QWidget* parent = nullptr, const QString &caption = QString(), const QString &dir = QString(), const QString &filter = QString(), QString* selectedFilter = 0, QFileDialog::Options options = 0);
    static QStringList getOpenFileNames(QWidget* parent = 0, const QString &caption = QString(), const QString &dir = QString(), const QString &filter = QString(), QString* selectedFilter = 0, QFileDialog::Options options = 0);
    static QString getSaveFileName(QWidget* parent = 0, const QString &caption = QString(), const QString &dir = QString(), const QString &filter = QString(), QString* selectedFilter = 0, QFileDialog::Options options = 0);

    static void QStringToFile(const QString &str, const QString fileName);
    static void QStringListToFile(const QStringList &str, const QString fileName);

    static bool copyDir(const QString &source, const QString &destination, const bool overwrite);
    static bool deleteDir(const QString &source);

    static bool startExternalProcess(const QString &executable, const QString &args);
    static QStringList splitCommandArguments(const QString &command);
    static QString ensureUniqueFilename(const QString &name, const QString &appendFormat = QString("(%1)"));
};

#endif // FILEUTIL_H
