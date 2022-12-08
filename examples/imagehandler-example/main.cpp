// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <DImageHandler>

#include <QHash>
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDebug>

DGUI_USE_NAMESPACE

bool rotateImage(DImageHandler &handler, QImage &image, const QString &param)
{
    int degresses = param.toInt();
    if (!handler.rotateImage(image, degresses)) {
        qWarning() << "Rotate image failed" << handler.lastError();
        return false;
    }

    return true;
}

bool applyImageFilter(QImage &image, const QString &param)
{
    enum FilterType { Old, Warm, Cool, Gray, AntiColor, Metal };
    static const QHash<QString, FilterType> colorFilters{
        {"old", Old}, {"warm", Warm}, {"cool", Cool}, {"gray", Gray}, {"anticolor", AntiColor}, {"metal", Metal}};

    QString filter = param.toLower();
    if (!colorFilters.contains(filter)) {
        qWarning() << QString("There is no color filter named %1.").arg(filter);
        return false;
    }

    FilterType type = colorFilters.value(filter);
    switch (type) {
        case Old:
            image = DImageHandler::oldColorFilter(image);
            break;
        case Warm:
            image = DImageHandler::warmColorFilter(image);
            break;
        case Cool:
            image = DImageHandler::coolColorFilter(image);
            break;
        case Gray:
            image = DImageHandler::grayScaleColorFilter(image);
            break;
        case AntiColor:
            image = DImageHandler::antiColorFilter(image);
            break;
        case Metal:
            image = DImageHandler::metalColorFilter(image);
            break;
        default:
            break;
    }

    return true;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("my-copy-program");

    QCommandLineOption rotateOption({"r", "rotate"}, "Rotate image with geven degresses.", "degresses");
    QCommandLineOption saveOption({"o", "output"}, "Save image with file path.", "file");
    QCommandLineOption filterOption("f",
                                    "Apply filters to pictures.\n"
                                    "Contains old, warm, cool, gray, anticolor and metal.",
                                    "filter");

    QCommandLineParser parser;
    parser.setApplicationDescription("Image handler");
    parser.addHelpOption();
    parser.addOption(rotateOption);
    parser.addOption(saveOption);
    parser.addOption(filterOption);
    parser.addPositionalArgument("file", "Open file.", "[file]");
    parser.process(app);

    const QStringList args = parser.positionalArguments();
    if (args.isEmpty()) {
        parser.showHelp();
        return 0;
    }

    const QStringList fileArgs = parser.positionalArguments();
    if (fileArgs.isEmpty()) {
        qInfo() << "Not set open file.";
        return 0;
    }
    if (!parser.isSet(saveOption)) {
        qInfo() << "Not set save file path";
        return 0;
    }

    QString fileName = fileArgs.first();
    QString saveFileName = parser.value(saveOption);

    DImageHandler handler;
    handler.setFileName(fileName);
    QImage image = handler.readImage();
    if (image.isNull()) {
        qWarning() << "Can't read image." << handler.lastError();
        return 0;
    }

    if (parser.isSet(rotateOption)) {
        if (!rotateImage(handler, image, parser.value(rotateOption))) {
            return 0;
        }
    }

    if (parser.isSet(filterOption)) {
        if (!applyImageFilter(image, parser.value(filterOption))) {
            return 0;
        }
    }

    if (!handler.saveImage(image, saveFileName)) {
        qWarning() << QString("Save file %1 failed.").arg(saveFileName) << handler.lastError();
        return 0;
    }

    return 0;
}