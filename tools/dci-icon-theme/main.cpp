// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QGuiApplication>
#include <QImageReader>
#include <QCommandLineParser>
#include <QDirIterator>
#include <QBuffer>
#include <QDebug>

#include <QtConcurrent/QtConcurrent>
#include <DDciFile>
#include <stdexcept>
#include <atomic>

DCORE_USE_NAMESPACE

// Custom exception for DCI processing errors
class DciProcessingError : public std::runtime_error {
public:
    explicit DciProcessingError(const QString &message, int code = -1) 
        : std::runtime_error(message.toStdString()), errorCode(code) {}
    
    int getErrorCode() const { return errorCode; }
    
private:
    int errorCode;
};

#define MAX_SCALE 10
#define INVALIDE_QUALITY -2
static int quality4Scaled[MAX_SCALE] = {};
static inline void initQuality() {
    for (int i = 0; i < MAX_SCALE; ++i)
        quality4Scaled[i] = INVALIDE_QUALITY;
}

static inline void dciChecker(bool result, std::function<const QString()> cb) {
    if (!result) {
        qWarning() << "Failed on writing dci file" << cb();
        throw DciProcessingError("Failed on writing dci file", -6);
    }
}

// TODO 应该使用xdg图标查找规范解析index.theme来查找尺寸
static uint foundSize(const QFileInfo &fileInfo) {
    QDir dir = fileInfo.absoluteDir();

    // 解析尺寸
    auto parseSize = [](const QString &dirName) -> uint {
        bool ok;
        if (int size = dirName.toUInt(&ok); ok) {
            return size;
        }

        if (dirName.contains('x') && dirName.split('x').size() == 2) {
            if (int size = dirName.split('x').first().toUInt(&ok); ok) {
                return size;
            }
        }

        return 0;
    };

    if (uint size = parseSize(dir.dirName())) {
        return size;
    }

    // 尝试找上一级目录
    if (!dir.cdUp())
        return 0;

    return parseSize(dir.dirName());
}

static inline QByteArray webpImageData(const QImage &image, int quality) {
    QByteArray data;
    QBuffer buffer(&data);
    bool ok = buffer.open(QIODevice::WriteOnly);
    Q_ASSERT(ok);
    dciChecker(image.save(&buffer, "webp", quality), []{return "failed to save webp image";});
    return data;
}

static bool writeScaledImage(DDciFile &dci, const QImage &image, const QString &targetDir, int scale/* = 2*/)
{
    QString sizeDir = targetDir.mid(1, targetDir.indexOf("/", 1) - 1);
    bool ok = false;
    int baseSize = sizeDir.toInt(&ok);
    if (!ok)
        baseSize = 256;

    int size = scale * baseSize;
    QImage img;
    if (image.width() == size) {
        img = image;
    } else {
        img = image.scaledToWidth(size, Qt::SmoothTransformation);
    }

    dciChecker(dci.mkdir(targetDir + QString("/%1").arg(scale)), [&]{return dci.lastErrorString();});
    int quality = quality4Scaled[scale - 1];
    const QByteArray &data = webpImageData(img, quality);
    dciChecker(dci.writeFile(targetDir + QString("/%1/1.webp").arg(scale), data), [&]{return dci.lastErrorString();});

    return true;
}

static bool writeImage(DDciFile &dci, const QString &imageFile, const QString &targetDir)
{
    QImageReader reader(imageFile);
    if (!reader.canRead()) {
        qWarning() << "Ignore the null image file:" << imageFile;
        return false;
    }

    auto image = reader.read();
    for (int i = 0; i < MAX_SCALE; ++i) {
        if (quality4Scaled[i] == INVALIDE_QUALITY)
            continue;

        if (!writeScaledImage(dci, image, targetDir, i + 1))
            return false;
    }

    return true;
}

static bool recursionLink(DDciFile &dci, const QString &fromDir, const QString &targetDir)
{
    for (const auto &i : dci.list(fromDir, true)) {
        const QString file(fromDir + "/" + i);
        const QString targetFile(targetDir + "/" + i);
        if (dci.type(file) == DDciFile::Directory) {
            if (!dci.mkdir(targetFile))
                return false;
            if (!recursionLink(dci, file, targetFile))
                return false;
        } else {
            if (!dci.link(file, targetFile))
                return false;
        }
    }

    return true;
}

static QByteArray readNextSection(QIODevice *io) {
    QByteArray section;
    char ch;

    bool oneLine = true;
    while (io->getChar(&ch)) {
        if (!oneLine && ch == '"') { // ["] end
            if (io->peek(1) == ",")
                io->skip(1); // skip [,]
            break;
        }  else if (ch == ',') {
            break;
        } else if (ch == '"') { // ["] begin
            oneLine = false;
            continue;
        }

        section.append(ch);

        if(oneLine && io->peek(1) == "\n")
            break;
    }

    return section.trimmed();
}

QMultiHash<QString, QString> parseIconFileSymlinkMap(const QString &csvFile) {
    QFile file(csvFile);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed on open symlink map file:" << csvFile;
        throw DciProcessingError("Failed on open symlink map file", -7);
    }

    QMultiHash<QString, QString> map;
    while (!file.atEnd()) {
        QByteArray key = readNextSection(&file);
        QByteArray value = readNextSection(&file);
        for (const auto &i : value.split('\n')) {
            map.insert(QString::fromLocal8Bit(key), QString::fromLocal8Bit(i));
        }

        char ch = 0;
        while (file.getChar(&ch) && ch != '\n');
    }

    qInfo() << "Got symlinks:" << map.size();

    return map;
}

void makeLink(const QFileInfo &file, const QDir &outputDir, const QString &dciFilePath,
              const QMultiHash<QString, QString> &symlinksMap)
{
    if (symlinksMap.contains(file.completeBaseName())) {
        const QString symlinkKey = QFileInfo(dciFilePath).fileName();
        for (const auto &symTarget : symlinksMap.values(file.completeBaseName())) {
            const QString newSymlink = outputDir.absoluteFilePath(symTarget + ".dci");
            qInfo() << "Create symlink from" << symlinkKey << "to" << newSymlink;
            if (!QFile::link(symlinkKey, newSymlink)) {
                qWarning() << "Failed on create symlink from" << symlinkKey << "to" << newSymlink;
            }
        }
    }
}

void doFixDarkTheme(const QFileInfo &file, const QDir &outputDir, const QMultiHash<QString, QString> &symlinksMap)
{
    const QString &newFile = outputDir.absoluteFilePath(file.fileName());

    DDciFile dciFile(file.absoluteFilePath());
    if (!dciFile.isValid()) {
        qWarning() << "Skip invalid dci file:" << file.absoluteFilePath();
        return;
    }

    for (const auto &i : dciFile.list("/")) {
        if (dciFile.type(i) != DDciFile::Directory)
            continue;

        for (const auto &j : dciFile.list(i)) {
            if (dciFile.type(j) != DDciFile::Directory || !j.endsWith(".light"))
                continue;

            const QString darkDir(j.left(j.size() - 5) + "dark");
            Q_ASSERT(darkDir.endsWith(".dark"));
            if (!dciFile.exists(darkDir)) {
                dciChecker(dciFile.mkdir(darkDir), [&]{return dciFile.lastErrorString();});
                dciChecker(recursionLink(dciFile, j, darkDir), [&]{return dciFile.lastErrorString();});
            }
        }
    }

    dciChecker(dciFile.writeToFile(newFile), [&]{return dciFile.lastErrorString();});

    makeLink(file, outputDir, newFile, symlinksMap);
}

int main(int argc, char *argv[])
{
    QCommandLineOption fileFilter({"m", "match"}, "Give wildcard rules on search icon files, "
                                                  "Each eligible icon will be packaged to a dci file, "
                                                  "If the icon have the dark mode, it needs to store "
                                                  "the dark icon file at \"dark/\" directory relative "
                                                  "to current icon file, and the file name should be "
                                                  "consistent.", "wildcard palette");
    QCommandLineOption outputDirectory({"o", "output"}, "Save the *.dci files to the given directory.",
                                       "directory");
    QCommandLineOption symlinkMap({"s", "symlink"}, "Give a csv file to create symlinks for the output icon file."
                                                    "\nThe content of symlink.csv like:\n"
                                                    "\t\t sublime-text, com.sublimetext.2\n"
                                                    "\t\t deb, \"\n"
                                                    "\t\t application-vnd.debian.binary-package\n"
                                                    "\t\t application-x-deb\n"
                                                    "\t\t gnome-mime-application-x-deb\n"
                                                    "\t\t \"\n"
                                  ,
                                       "csv file");
    QCommandLineOption fixDarkTheme("fix-dark-theme", "Create symlinks from light theme for dark theme files.");
    QCommandLineOption scaleQuality({"O","scale-quality"}, "Quility of dci scaled icon image\n"
                                                "The value may like <scale size>=<quality value>  e.g. 2=98:3=95\n"
                                                "The quality factor must be in the range 0 to 100 or -1.\n"
                                                "Specify 0 to obtain small compressed files, 100 for large uncompressed files "
                                                "and -1 to use the image handler default settings.\n"
                                                "The higher the quality, the larger the dci icon file size", "scale quality");

    QGuiApplication a(argc, argv);

    a.setApplicationName("dci-icon-theme");
    a.setApplicationVersion(QString("%1.%2.%3")
                                .arg(DTK_VERSION_MAJOR)
                                .arg(DTK_VERSION_MINOR)
                                .arg(DTK_VERSION_PATCH));

    QCommandLineParser cp;
    cp.setApplicationDescription("dci-icon-theme tool is a command tool that generate dci icons from common icons.\n"
                                 "For example, the tool is used in the following ways: \n"
                                 "\t dci-icon-theme /usr/share/icons/hicolor/256x256/apps -o ~/Desktop/hicolor -O 3=95\n"
                                 "\t dci-icon-theme -m *.png /usr/share/icons/hicolor/256x256/apps -o ~/Desktop/hicolor -O 3=95\n"
                                 "\t dci-icon-theme --fix-dark-theme <input dci files directory> -o <output directory path> \n"
                                 "\t dci-icon-theme <input file directory> -o <output directory path> -s <csv file> -O <qualities>\n"""
                                 );

    cp.addOptions({fileFilter, outputDirectory, symlinkMap, fixDarkTheme, scaleQuality});
    cp.addPositionalArgument("source", "Search the given directory and it's subdirectories, "
                                       "get the files conform to rules of --match.",
                             "~/dci-png-icons");
    cp.addHelpOption();
    cp.addVersionOption();
    cp.process(a);

    if (a.arguments().size() == 1)
        cp.showHelp(-1);

    if (cp.positionalArguments().isEmpty()) {
        qWarning() << "Not give a source directory.";
        cp.showHelp(-2);
    }

    if (!cp.isSet(outputDirectory)) {
        qWarning() << "Not give -o argument";
        cp.showHelp(-4);
    }

    if (!cp.isSet(scaleQuality) && !cp.isSet(fixDarkTheme)) {
        qWarning() << "Not give -O argument"; scaleQuality.flags();
        cp.showHelp(-5);
    }

    initQuality();
    QString surfix;
    if (cp.isSet(scaleQuality)) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        auto behavior = Qt::SkipEmptyParts;
#else
        auto behavior = QString::SkipEmptyParts;
#endif
        QStringList qualityList = cp.value(scaleQuality).split(":", behavior);

        for (const QString &kv : qualityList) {
            auto sq = kv.split("=");
            if (sq.size() != 2) {
                qWarning() << "Invalid quality value:" << kv;
                continue;
            }

            int scaleSize = sq.value(0).toInt();
            if (scaleSize < 1 || scaleSize > MAX_SCALE) {
                qWarning() << "Invalid scale size:" << kv;
                continue;
            }
            int validQuality = qMax(qMin(sq.value(1).toInt(), 100), -1); // -1,  0~100
            quality4Scaled[scaleSize - 1] = validQuality;
        }
    }

    QDir outputDir(cp.value(outputDirectory));
    if (!outputDir.exists()) {
        if (!QDir::current().mkpath(outputDir.absolutePath())) {
            qWarning() << "Can't create the" << outputDir.absolutePath() << "directory";
            cp.showHelp(-5);
        }
    } else {
        qErrnoWarning("The output directory have been exists.");
#ifndef QT_DEBUG
        return -1;
#endif
    }

    QMultiHash<QString, QString> symlinksMap;
    if (cp.isSet(symlinkMap)) {
        try {
            symlinksMap = parseIconFileSymlinkMap(cp.value(symlinkMap));
        } catch (const DciProcessingError &e) {
            qWarning() << "Error parsing symlink map:" << e.what();
            return e.getErrorCode();
        }
    }

    const QStringList nameFilter = cp.isSet(fileFilter) ? cp.values(fileFilter) : QStringList();
    const auto sourceDirectory = cp.positionalArguments();
    for (const auto &sd : sourceDirectory) {
        QDir sourceDir(sd);
        if (!sourceDir.exists()) {
            qWarning() << "Ignore the non-exists directory:" << sourceDir;
            continue;
        }

        // read all links first
        {
            QDirIterator di(sourceDir.absolutePath(), nameFilter,
                            QDir::NoDotAndDotDot | QDir::Files,
                            QDirIterator::Subdirectories);
            while (di.hasNext()) {
                di.next();
                QFileInfo file = di.fileInfo();
                if (!file.isSymLink())
                    continue;

#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
        auto link = file.symLinkTarget();
#else
        auto link = file.readLink();
#endif
                 const QString &linkTarget = QFileInfo(link).completeBaseName();
                 if (!symlinksMap.values(linkTarget).contains(file.completeBaseName())) {
                     symlinksMap.insert(linkTarget, file.completeBaseName());
                     qInfo() << "Add link" << file.completeBaseName() << "->" << linkTarget;
                 } else {
//                        qDebug() << "** link already existed in symlinksMap **";
                 }
            }
        }

        // Collect all files first, grouped by icon name to avoid concurrent access to same DCI file
        QMap<QString, QList<QFileInfo>> iconGroups;
        QDirIterator di(sourceDir.absolutePath(), nameFilter,
                        QDir::NoDotAndDotDot | QDir::Files,
                        QDirIterator::Subdirectories);
        while (di.hasNext()) {
            di.next();
            QFileInfo file = di.fileInfo();

            if (file.isSymLink())
                continue;

            if (cp.isSet(fixDarkTheme)) {
                try {
                    doFixDarkTheme(file, outputDir, symlinksMap);
                } catch (const DciProcessingError &e) {
                    qWarning() << "Error fixing dark theme for file" << file.absoluteFilePath() << ":" << e.what();
                    return e.getErrorCode();
                }
                continue;
            }

            if (file.path().endsWith(QStringLiteral("/dark"))) {
                qInfo() << "Ignore the dark icon file:"  << file;
                continue;
            }

            iconGroups[file.completeBaseName()].append(file);
        }
        
        // Process with proper exception handling
        std::atomic<bool> hasError{false};
        int errorCode = 0;
        
        // Process icon groups concurrently (each group shares same DCI file)
        QList<QString> iconNames = iconGroups.keys();
        QtConcurrent::blockingMap(iconNames, [&](const QString &iconName) {
            if (hasError.load()) return; // Skip if already has error
            
            try {
                const QList<QFileInfo> &files = iconGroups[iconName];
                const QString dciFilePath(outputDir.absoluteFilePath(iconName) + surfix + ".dci");
                QScopedPointer<DDciFile> dciFile;
        
                for (const QFileInfo &file : files) {
                    QString dirName = file.absoluteDir().dirName();
                    uint iconSize = foundSize(file);
                    dirName = iconSize > 0 ? QString("/%1").arg(iconSize) : dirName.prepend("/");

                    // Initialize DCI file once per icon group
                    if (dciFile.isNull()) {
                        if (QFileInfo::exists(dciFilePath)) {
                            dciFile.reset(new DDciFile(dciFilePath));
                        }
                        if (dciFile.isNull() || !dciFile->isValid()) {
                            dciFile.reset(new DDciFile);
                        }
                    }
                    if (dciFile->exists(dirName)) {
                        qWarning() << "Skip exists dci file:" << dciFilePath << dirName << dciFile->list(dirName);
                        continue;
                    }

                    qInfo() << "Writing to dci file:" << file.absoluteFilePath() << "==>" << dciFilePath;

                    QString sizeDir = iconSize > 0 ? dirName : "/256";  // "/256" as default
                    QString normalLight = sizeDir + "/normal.light";         //  "/256/normal.light"
                    QString normalDark = sizeDir + "/normal.dark";          //   "/256/normal.dark"

                    if (dciFile->exists(sizeDir)) {
                        qWarning() << "Skip exists dci file:" << dciFilePath << sizeDir << dciFile->list(sizeDir);
                        continue;
                    }

                    dciChecker(dciFile->mkdir(sizeDir), [&]{return dciFile->lastErrorString();});
                    dciChecker(dciFile->mkdir(normalLight), [&]{return dciFile->lastErrorString();});
                    if (!writeImage(*dciFile, file.filePath(), normalLight))
                        continue;
                    
                    dciChecker(dciFile->mkdir(normalDark), [&]{return dciFile->lastErrorString();});
                    QFileInfo darkIcon(file.dir().absoluteFilePath("dark/" + file.fileName()));
                    if (darkIcon.exists()) {
                        writeImage(*dciFile, darkIcon.filePath(), normalDark);
                    } else {
                        dciChecker(recursionLink(*dciFile, normalLight, normalDark), [&]{return dciFile->lastErrorString();});
                    }
                }
                
                // Write DCI file once per icon group
                if (!dciFile.isNull()) {
                    dciChecker(dciFile->writeToFile(dciFilePath), [&]{return dciFile->lastErrorString();});
                    
                    // Create symlinks for all files in this group
                    for (const QFileInfo &file : files) {
                        makeLink(file, outputDir, dciFilePath, symlinksMap);
                    }
                }
            } catch (const DciProcessingError &e) {
                qWarning() << "Error processing icon group" << iconName << ":" << e.what();
                hasError.store(true);
                errorCode = e.getErrorCode();
            }
        });

        if (hasError.load()) {
            qWarning() << "Encountered errors during DCI file writing. Exiting with error code:" << errorCode;
            return errorCode;
        }
    }

    return 0;
}
