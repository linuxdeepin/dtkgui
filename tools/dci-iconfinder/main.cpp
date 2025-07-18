// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QGuiApplication>
#include <QCommandLineParser>
#include <QDebug>

#include <DIconTheme>

DGUI_USE_NAMESPACE

int main(int argc, char *argv[]) 
{
    QGuiApplication a(argc, argv);

    a.setApplicationName("dci-iconfinder");
    a.setApplicationVersion(QString("%1.%2.%3")
                                .arg(DTK_VERSION_MAJOR)
                                .arg(DTK_VERSION_MINOR)
                                .arg(DTK_VERSION_PATCH));

    QCommandLineParser cp;
    cp.setApplicationDescription(
        "dci-iconfinder tool is a command tool that find dci icons in the icon "
        "theme.\n"
        "For example, the tool is used in the following ways: \n"
        "\t dci-iconfinder <icon name>\n"
        "\t dci-iconfinder <icon name> -t bloom\n");

    QCommandLineOption themeOpt({"t", "theme"},
                                "Give a theme name to find dci icon file path",
                                "theme name");
    cp.addOptions({themeOpt});
    cp.addPositionalArgument("iconnames", "The icon names to search for",
                            "iconnames");
    cp.addHelpOption();
    cp.addVersionOption();
    cp.process(a);

    if (cp.positionalArguments().isEmpty()) {
        qWarning() << "Not give icon name.";
        cp.showHelp(-1);
    }

    QString iconThemeName;
    if (cp.isSet(themeOpt)) {
        iconThemeName = cp.value(themeOpt);
    } else {
        iconThemeName = QIcon::themeName();
    }

    const auto icons = cp.positionalArguments();
    for (const QString &iconName : icons) {
        QString iconPath = DIconTheme::findDciIconFile(iconName, iconThemeName);
        if (!iconPath.isEmpty())
            qInfo().noquote() << iconName << "[" << iconThemeName << "]:" << iconPath;
    }

    return 0;
}
