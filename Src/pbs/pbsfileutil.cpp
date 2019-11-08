#include "pbsfileutil.h"
#include <QFileInfo>
#include <QUrl>
#include <QDir>
#include <QProcess>
#include <QProcessEnvironment>
#include <QDesktopServices>
#include <QDebug>

#include <pbstools.h>

#ifdef WIN32
#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>
#endif

//lite_memory_check_begin
#if defined(WIN32) && defined(_MSC_VER) &&  defined(_DEBUG)
     #define _CRTDBG_MAP_ALLOC
     #include <stdlib.h>
     #include <crtdbg.h>
     #define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
     #define new DEBUG_NEW
#endif
//lite_memory_check_end


bool PBSFileUtil::compareFile(const QString &fileName1, const QString &fileName2, bool canonical)
{
    if (fileName1.isEmpty() || fileName2.isEmpty()) {
        return false;
    }
    if (canonical) {
#if defined(WIN32)
        return (QFileInfo(fileName1).canonicalFilePath().compare(QFileInfo(fileName2).canonicalFilePath(), Qt::CaseInsensitive) == 0);
#else
        return QFileInfo(fileName1).canonicalFilePath() == QFileInfo(fileName2).canonicalFilePath();
#endif
    }
    return QFileInfo(fileName1).filePath() == QFileInfo(fileName2).filePath();
}

QStringList PBSFileUtil::removeFiles(const QStringList &files)
{
    QStringList result;
    foreach (QString file, files) {
        if (QFile::exists(file) && QFile::remove(file)) {
            result << file;
        }
    }
    return result;
}

QStringList PBSFileUtil::removeWorkDir(const QString &workDir, const QStringList &filters)
{
    QStringList result;
    QDir dir(workDir);
    if (!dir.exists())
        return result;

    QFileInfoList dirs = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

    foreach (QFileInfo d, dirs) {
        removeWorkDir(d.filePath(),filters);
    }

    QFileInfoList files = dir.entryInfoList(filters,QDir::Files);
    foreach (QFileInfo f, files) {
        bool b = QFile::remove(f.filePath());
        if (b) {
            result << f.fileName();
        }
    }
    return result;
}

bool PBSFileUtil::Exist(const QString &src)
{
    QFile f(src);
    return f.exists();
}

bool PBSFileUtil::Rename(const QString &src, const QString &dest)
{
    QFile f(src);
    f.rename(dest);
}

bool PBSFileUtil::Copy(const QString &src, const QString &dest)
{
    QFile f(src);
    f.copy(dest);
}

bool PBSFileUtil::Move(const QString &src, const QString &dest)
{
    PBSFileUtil::removeFiles(QStringList() << dest);
    QFile f(src);
    f.copy(dest);
}

QMap<QString,QStringList> PBSFileUtil::readFileContext(QIODevice *dev)
{
    QMap<QString,QStringList> contextMap;
    QStringList list;
    QString line;
    bool bnext = false;
    while (!dev->atEnd()) {
        QByteArray ar = dev->readLine().trimmed();
        if (!ar.isEmpty() && ar.right(1) == "\\") {
            bnext = true;
            ar[ar.length()-1] = ' ';
        } else {
            bnext = false;
        }
        line.push_back(ar);
        if (!bnext && !line.isEmpty()) {
            list.push_back(line);
            line.clear();
        }
    }

    if (!line.isEmpty()) {
        list.push_back(line);
    }

    foreach (QString line, list) {
        if (line.size() >= 1 && line.at(0) == '#')
            continue;
        QStringList v = line.split(QRegExp("\\+="),QString::SkipEmptyParts);
        if (v.count() == 1) {
            v = line.split(QRegExp("="),QString::SkipEmptyParts);
            if (v.count() == 2) {
                QStringList v2 = v.at(1).split(" ",QString::SkipEmptyParts);
                if (!v2.isEmpty()) {
                    contextMap[v.at(0).trimmed()] = v2;
                }
            }
        } else if (v.count() == 2) {
            QStringList v2 = v.at(1).split(" ",QString::SkipEmptyParts);
            if (!v2.isEmpty())
                contextMap[v.at(0).trimmed()].append(v2);
        }
    }
    return contextMap;
}

#ifdef Q_OS_WIN
QString PBSFileUtil::canExec(QString fileName, QStringList exts)
{
    QFileInfo info(fileName);
    QString suffix = info.suffix();
    if (!suffix.isEmpty()) {
        suffix = "."+suffix;
        foreach(QString ext, exts) {
            if (suffix == ext && info.exists()) {
                return info.canonicalFilePath();
            }
        }
    }
    foreach(QString ext, exts) {
        QFileInfo info(fileName+ext);
        if (info.exists()) {
            return info.filePath();
        }
    }
    return QString();
}

QString PBSFileUtil::lookPath(const QString &file, const QProcessEnvironment &env, bool bLocalPriority)
{
    QString fileName = file;
    QStringList exts;
    QString extenv = env.value("PATHEXT");
    if (!extenv.isEmpty()) {
        foreach(QString ext, extenv.split(';',QString::SkipEmptyParts)) {
            if (ext.isEmpty()) {
                continue;
            }
            if (ext[0] != '.') {
                ext= '.'+ext;
            }
            exts.append(ext.toLower());
        }
    }
    exts << ".exe" << ".bat" << ".cmd";
    exts.removeDuplicates();

    if (fileName.contains('\\') || fileName.contains('/')) {
        QString exec = canExec(fileName,exts);
        if (!exec.isEmpty()) {
            return exec;
        }
    }
    if (bLocalPriority) {
        QString exec = canExec(".\\"+fileName,exts);
        if (!exec.isEmpty()) {
            return exec;
        }
    }
    QString pathenv = env.value("PATH");
    if (pathenv.isEmpty()) {
        QString exec = canExec(".\\"+fileName,exts);
        if (!exec.isEmpty()) {
            return exec;
        }
    } else {
        foreach(QString dir,pathenv.split(';',QString::SkipEmptyParts)) {
            QFileInfo info(QDir(dir),fileName);
            QString exec = canExec(info.filePath(),exts);
            if (!exec.isEmpty()) {
                return exec;
            }
        }
    }
    return QString();
}

QString PBSFileUtil::lookPathInDir(const QString &file, const QString &dir)
{
    QString fileName = file;
    QStringList exts;
    QString extenv =  QProcessEnvironment::systemEnvironment().value("PATHEXT");
    if (!extenv.isEmpty()) {
        foreach(QString ext, extenv.split(';',QString::SkipEmptyParts)) {
            if (ext.isEmpty()) {
                continue;
            }
            if (ext[0] != '.') {
                ext= '.'+ext;
            }
            exts.append(ext.toLower());
        }
    }
    exts << ".exe" << ".bat" << ".cmd";
    exts.removeDuplicates();
    if (fileName.contains('\\') || fileName.contains('/')) {
        QString exec = canExec(fileName,exts);
        if (!exec.isEmpty()) {
            return exec;
        }
    }
    QFileInfo info(QDir(dir),fileName);
    QString exec = canExec(info.filePath(),exts);
    if (!exec.isEmpty()) {
        return exec;
    }
    return QString();
}

#else
QString PBSFileUtil::canExec(QString fileName, QStringList /*exts*/)
{
    QFileInfo info(fileName);
    if (info.exists() && info.isFile()  && info.isExecutable()) {
        return info.canonicalFilePath();
    }
    return QString();
}

QString PBSFileUtil::lookPath(const QString &file, const QProcessEnvironment &env, bool bLocalPriority)
{
    QString fileName = file;
    if (fileName.contains('/')) {
        QString exec = canExec(fileName);
        if (!exec.isEmpty()) {
            return exec;
        }
    }
    if (bLocalPriority) {
        QString exec = canExec("./"+fileName);
        if (!exec.isEmpty()) {
            return exec;
        }
    }
    QString pathenv = env.value("PATH");
    foreach(QString dir,pathenv.split(':',QString::KeepEmptyParts)) {
        if (dir == "") {
            dir = ".";
        }
        QString exec = canExec(dir+"/"+file);
        if (!exec.isEmpty()) {
            return exec;
        }
    }
    return QString();
}

QString PBSFileUtil::lookPathInDir(const QString &file, const QString &dir)
{
    QString fileName = file;
    if (fileName.contains('/')) {
        QString exec = canExec(fileName);
        if (!exec.isEmpty()) {
            return exec;
        }
    }
    QString exec = canExec(dir+"/"+file);
    if (!exec.isEmpty()) {
        return exec;
    }
    return QString();
}

#endif

QString PBSFileUtil::findExecute(const QString &target)
{
    QStringList targetList;
#ifdef Q_OS_WIN
    targetList << target+".exe";
#endif
    targetList << target;
    foreach (QString fileName, targetList) {
        if (QFile::exists(fileName)) {
            QFileInfo info(fileName);
            if (info.isFile() && info.isExecutable()) {
                return info.canonicalFilePath();
            }
        }
    }
    return QString();
}

bool PBSFileUtil::CopyDirectory(const QString &src, const QString &dest)
{
    QDir dir(src);
    foreach(QFileInfo info, dir.entryInfoList(QDir::Files)) {
        if (info.isFile() && !info.isSymLink()) {
            QFile in(info.filePath());
            if (!in.open(QFile::ReadOnly)) {
                return false;
            }
            QFile out(dest+"/"+info.fileName());
            if (!out.open(QFile::WriteOnly)) {
                return false;
            }
            out.write(in.readAll());
        }
    }
    return true;
}

#ifdef Q_OS_WIN
bool openBrowser(LPCTSTR lpszFileName)
{
    HINSTANCE hl= LoadLibrary(TEXT("liteshell.dll"));
    typedef BOOL (*BrowseToFileProc)(const wchar_t* filename);
    if(!hl)
        return false;
    bool b = false;
    BrowseToFileProc proc = (BrowseToFileProc)GetProcAddress(hl,"BrowseToFile");
    if (proc) {
        b = proc(lpszFileName);
    }
    FreeLibrary(hl);
    return b;
}

bool shellOpenFolder(LPCTSTR filename)
{
    HINSTANCE hl= LoadLibrary(TEXT("Shell32.dll"));
    if (!hl) {
        return false;
    }
    typedef LPITEMIDLIST(*ILCreateFromPathProc)(LPCTSTR);
    typedef void (*ILFreeProc)(LPITEMIDLIST);
    typedef void (*SHOpenFolderAndSelectItemsProc)(LPCITEMIDLIST pidlFolder, UINT cidl, LPCITEMIDLIST *apidl, DWORD dwFlags);
    ILCreateFromPathProc fnILCreateFromPath =0;
    ILFreeProc fnILFree = 0;
    SHOpenFolderAndSelectItemsProc fnSHOpenFolderAndSelectItems = 0;
    fnILCreateFromPath = (ILCreateFromPathProc)GetProcAddress(hl,"ILCreateFromPath");
    fnILFree = (ILFreeProc)GetProcAddress(hl,"ILFree");
    fnSHOpenFolderAndSelectItems = (SHOpenFolderAndSelectItemsProc)GetProcAddress(hl,"SHOpenFolderAndSelectItems");
    bool b = false;
    if(fnILCreateFromPath && fnILFree && fnSHOpenFolderAndSelectItems){
        ITEMIDLIST *pidl=0;
        pidl = fnILCreateFromPath(filename);
        if (pidl) {
            fnSHOpenFolderAndSelectItems(pidl,0,0,0);
            fnILFree(pidl);
            b = true;
        }
    }
    FreeLibrary(hl);
    return b;
}
#endif

void PBSFileUtil::openInExplorer(const QString &path)
{
#ifdef Q_OS_WIN
    if (openBrowser((LPCTSTR)QDir::toNativeSeparators(path).utf16())) {
        return;
    }
    if (shellOpenFolder((LPCTSTR)QDir::toNativeSeparators(path).utf16())) {
        return;
    }
    const QString explorer = PBSFileUtil::lookPath("explorer.exe",QProcessEnvironment::systemEnvironment(),false);
    if (!explorer.isEmpty()) {
        QStringList param;
        if (!QFileInfo(path).isDir())
            param += QLatin1String("/select,");
        param += QDir::toNativeSeparators(path);
        QProcess::startDetached(explorer, param);
        return;
    }
#endif
#ifdef Q_OS_MAC
    if (QFileInfo("/usr/bin/osascript").exists()) {
        QStringList scriptArgs;
        scriptArgs << QLatin1String("-e")
                   << QString::fromLatin1("tell application \"Finder\" to reveal POSIX file \"%1\"")
                                         .arg(path);
        QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
        scriptArgs.clear();
        scriptArgs << QLatin1String("-e")
                   << QLatin1String("tell application \"Finder\" to activate");
        QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
        return;
    }
#endif
    QFileInfo info(path);
    if (info.isDir()) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(info.filePath()));
    } else {
        QDesktopServices::openUrl(QUrl::fromLocalFile(info.path()));
    }
}

bool PBSFileUtil::startProcess(const QString &cmd, const QStringList args, const QString workDir)
{
    return QProcess::startDetached(cmd, args, workDir);
}

QString PBSFileUtil::extractPath(const QString &fileName)
{
    QFileInfo fi(fileName);
    if(fi.exists())
        return fi.path();
    return "";
}

QString PBSFileUtil::extractName(const QString &fileName)
{
    QFileInfo fi(fileName);
    if(fi.exists())
        return fi.fileName();

    return "";
}


QString PBSFileUtil::extractOnlyName(const QString &fileName)
{
    QFileInfo fi(fileName);
    if(fi.exists())
        return fi.completeBaseName();

    return "";
}

QString PBSFileUtil::extractExtension(const QString &fileName)
{
    QFileInfo fi(fileName);
    if(fi.exists())
        return fi.completeSuffix();

    return "";
}

qint64 PBSFileUtil::size(const QString &fileName)
{
    QFileInfo fi(fileName);
    if(fi.exists())
        return fi.size();

    return 0;
}

QString PBSFileUtil::changeExtension(const QString &fileName, const QString &newExt)
{
    return PBSFileUtil::extractPath(fileName) + QDir::separator() + PBSFileUtil::extractOnlyName(fileName) + "." + newExt;
}

QString PBSFileUtil::readAllFileContents(const QString &filename)
{
    return QString::fromUtf8(readAllFileByteContents(filename));
}

QByteArray PBSFileUtil::readAllFileByteContents(const QString &filename)
{
    QFile file(filename);

    if (!filename.isEmpty() && file.open(QFile::ReadOnly)) {
        const QByteArray a = file.readAll();
        file.close();
        return a;
    }

    return QByteArray();
}

QString PBSFileUtil::getExistingDirectory(const QString &name, QWidget* parent, const QString &caption, const QString &dir, QFileDialog::Options options)
{
    QString lastDir = dir;

    QString path = QFileDialog::getExistingDirectory(parent, caption, lastDir, options);

    return path;
}

QString PBSFileUtil::getOpenFileName(QWidget* parent, const QString &caption, const QString &dir, const QString &filter, QString* selectedFilter, QFileDialog::Options options)
{
    QString path = QFileDialog::getOpenFileName(parent, caption, dir, filter, selectedFilter, options);
    return path;
}

QStringList PBSFileUtil::getOpenFileNames(QWidget* parent, const QString &caption, const QString &dir, const QString &filter, QString* selectedFilter, QFileDialog::Options options)
{
    QStringList paths = QFileDialog::getOpenFileNames(parent, caption, dir, filter, selectedFilter, options);
    return paths;
}

QString PBSFileUtil::getSaveFileName(QWidget* parent, const QString &caption, const QString &dir, const QString &filter, QString* selectedFilter, QFileDialog::Options options)
{
    QString path = QFileDialog::getSaveFileName(parent, caption, dir, filter, selectedFilter, options);
    return path;
}

void PBSFileUtil::QStringToFile(const QString &str, const QString fileName)
{
    QFile file(fileName);
    if(!file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate))
    {
        qDebug() << fileName << " Dosyas Açılamadı";
        return;
    }

    QTextStream out(&file);
    out << str;
    file.flush();
    file.close();
}

void PBSFileUtil::QStringListToFile(const QStringList &str, const QString fileName)
{
    QFile file(fileName);
    if(!file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate))
    {
        qDebug() << fileName << " Dosyas Açılamadı";
        return;
    }

    QTextStream out(&file);
    foreach(QString text, str)
        out << text << "\n";
    file.flush();
    file.close();
}


bool PBSFileUtil::copyDir(const QString &source, const QString &destination, const bool overwrite)
{
    QDir directory(source);
    bool error = false;

    if (!directory.exists())
    {
        return false;
    }

    QDir destDir(destination);
    destDir.mkpath(destination);

    QStringList dirs = directory.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
    QStringList files = directory.entryList(QDir::Files);

    QList<QString>::iterator d, f;

    for (d = dirs.begin(); d != dirs.end(); ++d)
    {
        if (!QFileInfo(directory.path() + "/" + (*d)).isDir())
        {
            continue;
        }

        QDir temp(destination + "/" + (*d));
        temp.mkpath(temp.path());

        if (!copyDir(directory.path() + "/" + (*d), destination + "/" + (*d), overwrite))
        {
            error = true;
        }
    }

    for (f = files.begin(); f != files.end(); ++f)
    {

        QFile tempFile(directory.path() + "/" + (*f));

        if (QFileInfo(directory.path() + "/" + (*f)).isDir())
        {
            continue;
        }

        QFile destFile(destination + "/" + directory.relativeFilePath(tempFile.fileName()));

        if (destFile.exists() && overwrite)
        {
            destFile.remove();
        }

        if (!tempFile.copy(destination + "/" + directory.relativeFilePath(tempFile.fileName())))
        {
            error = true;
        }
    }

    return !error;
}

bool PBSFileUtil::deleteDir(const QString &source)
{
    QDir directory(source);
    bool error = false;

    if (!directory.exists())
    {
        return false;
    }

    QStringList dirs = directory.entryList(QDir::AllDirs | QDir::Hidden | QDir::NoDotAndDotDot);
    QStringList files = directory.entryList(QDir::Files);

    QList<QString>::iterator d, f;

    for (d = dirs.begin(); d != dirs.end(); ++d)
    {
        QString dirPath = directory.path() + "/" + (*d);
        if (!QFileInfo(dirPath).isDir())
        {
            continue;
        }
        if (!deleteDir(dirPath))
        {
            error = true;
        }
        directory.rmdir(dirPath);
    }

    for (f = files.begin(); f != files.end(); ++f)
    {
        QFile::remove
        (directory.path() + "/" + (*f));
    }

    QString dir = directory.dirName();
    if (directory.cdUp())
    {
        directory.rmdir(dir);
    }

    return !error;
}

QStringList PBSFileUtil::splitCommandArguments(const QString &command)
{
    QString line = command.trimmed();

    if (line.isEmpty()) {
        return QStringList();
    }

    QChar SPACE(' ');
    QChar EQUAL('=');
    QChar BSLASH('\\');
    QChar QUOTE('"');
    QStringList r;

    int equalPos = -1; // Position of = in opt="value"
    int startPos = PBSTools::isQuote(line.at(0)) ? 1 : 0;
    bool inWord = !PBSTools::isQuote(line.at(0));
    bool inQuote = !inWord;

    if (inQuote) {
        QUOTE = line.at(0);
    }

    const int strlen = line.length();
    for (int i = 0; i < strlen; ++i) {
        const QChar c = line.at(i);

        if (inQuote && c == QUOTE && i > 0 && line.at(i - 1) != BSLASH) {
            QString str = line.mid(startPos, i - startPos);
            if (equalPos > -1) {
                str.remove(equalPos - startPos + 1, 1);
            }

            inQuote = false;
            if (!str.isEmpty()) {
                r.append(str);
            }
            continue;
        }
        else if (!inQuote && PBSTools::isQuote(c)) {
            inQuote = true;
            QUOTE = c;

            if (!inWord) {
                startPos = i + 1;
            }
            else if (i > 0 && line.at(i - 1) == EQUAL) {
                equalPos = i - 1;
            }
        }

        if (inQuote) {
            continue;
        }

        if (inWord && (c == SPACE || i == strlen - 1)) {
            int len = (i == strlen - 1) ? -1 : i - startPos;
            const QString str = line.mid(startPos, len);

            inWord = false;
            if (!str.isEmpty()) {
                r.append(str);
            }
        }
        else if (!inWord && c != SPACE) {
            inWord = true;
            startPos = i;
        }
    }

    // Unmatched quote
    if (inQuote) {
        return QStringList();
    }

    return r;
}

bool PBSFileUtil::startExternalProcess(const QString &executable, const QString &args)
{
    const QStringList arguments = splitCommandArguments(args);

    bool success = QProcess::startDetached(executable, arguments);

    if (!success) {
        QString info = "<ul><li><b>%1</b>%2</li><li><b>%3</b>%4</li></ul>";
        info = info.arg(QObject::tr("Executable: "), executable,
                        QObject::tr("Arguments: "), arguments.join(QLatin1String(" ")));

        qDebug() << QObject::tr("Cannot start external program! %1").arg(info);
    }

    return success;
}

QString PBSFileUtil::ensureUniqueFilename(const QString &name, const QString &appendFormat)
{
    Q_ASSERT(appendFormat.contains(QL1S("%1")));

    QFileInfo info(name);

    if (!info.exists())
        return name;

    const QDir dir = info.absoluteDir();
    const QString fileName = info.fileName();

    int i = 1;

    while (info.exists()) {
        QString file = fileName;
        int index = file.lastIndexOf(QL1C('.'));
        const QString appendString = appendFormat.arg(i);
        if (index == -1)
            file.append(appendString);
        else
            file = file.left(index) + appendString + file.mid(index);
        info.setFile(dir, file);
        i++;
    }

    return info.absoluteFilePath();
}
