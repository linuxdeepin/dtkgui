// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "xdgiconproxyengine_p.h"

#include <QFile>
#include <QIconEngine>
#include <QThreadStorage>
#include <QXmlStreamReader>
#include <QDebug>
#include <QPainter>
#include <QPalette>
#include <QGuiApplication>

#include <cxxabi.h>
#include <qmath.h>

#if XDG_ICON_VERSION_MAR >= 3
#define private public
#include <private/xdgiconloader/xdgiconloader_p.h>
#undef private
#elif XDG_ICON_VERSION_MAR == 2
//这个版本中的xdgiconloader_p.h定义和qiconloader_p.h有冲突
//只能通过此方式提供创建XdgIconLoaderEngine对象的接口
#include "xdgiconenginecreator.h"
#endif

static bool XdgIconFollowColorScheme()
{
    // XdgIcon::followColorScheme()
    return XdgIconLoader::instance()->followColorScheme();
}

#if XDG_ICON_VERSION_MAR >= 3
namespace DEEPIN_QT_THEME {
QThreadStorage<QString> colorScheme;
void (*setFollowColorScheme)(bool);
bool (*followColorScheme)();
} // namespace DEEPIN_QT_THEME
#endif

DGUI_BEGIN_NAMESPACE

XdgIconProxyEngine::XdgIconProxyEngine(XdgIconLoaderEngine *proxy)
    : engine(proxy)
    , lastMode(QIcon::Normal)
{
}

XdgIconProxyEngine::~XdgIconProxyEngine()
{
    if (engine)
        delete engine;
}

quint64 XdgIconProxyEngine::entryCacheKey(const ScalableEntry *color_entry, const QIcon::Mode mode, const QIcon::State state)
{
    return quint64(color_entry) ^ (quint64(mode) << 56) ^ (quint64(state) << 48);
}

QPixmap XdgIconProxyEngine::followColorPixmap(ScalableEntry *color_entry, const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    if (mode == QIcon::Selected && mode != lastMode) {
        // 由非选中状态切换至选中状态，如果颜色值不更新，会按照上一次的状态（非选中时的颜色值）
        // 从svg图标缓存中寻找异常图标，导致图标异常。
        for (int each_mode = QIcon::Normal; each_mode <= QIcon::Selected; ++each_mode) {
            quint64 each_cache_key = entryCacheKey(color_entry, QIcon::Mode(each_mode), state);
            entryToColorScheme.remove(each_cache_key);
        }
    }

    lastMode = mode;
    quint64 cache_key = entryCacheKey(color_entry, mode, state);
    const QString &cache_color_scheme = entryToColorScheme.value(cache_key);

    // 当size为1时表示此svg文件不需要处理ColorScheme标签
    if (cache_color_scheme.size() == 1)
        return color_entry->pixmap(size, mode, state);

    const QString &color_scheme = DEEPIN_QT_THEME::colorScheme.localData();
    QPixmap pm = color_scheme == cache_color_scheme ? color_entry->svgIcon.pixmap(size, mode, state) : QPixmap();
    // Note: not checking the QIcon::isNull(), because in Qt5.10 the isNull() is not reliable
    // for svg icons desierialized from stream (see https://codereview.qt-project.org/#/c/216086/)
    if (pm.isNull()) {
        // The following lines are adapted and updated from KDE's "kiconloader.cpp" ->
        // KIconLoaderPrivate::processSvg() and KIconLoaderPrivate::createIconImage().
        // They read the SVG color scheme of SVG icons and give images based on the icon mode.
        QHash<int, QByteArray> svg_buffers;
        bool invalidBuffers = true;
        QFile device {color_entry->filename};
        if (device.open(QIODevice::ReadOnly)) {
            // Note: indexes are assembled as in qtsvg (QSvgIconEnginePrivate::hashKey())
            QPair<int, QString> style_sheet;
            style_sheet = qMakePair((mode << 4) | state, QStringLiteral(".ColorScheme-Text{color:%1;}").arg(color_scheme));
            QSharedPointer<QXmlStreamWriter> writer(new QXmlStreamWriter {&svg_buffers[style_sheet.first]});

            QXmlStreamReader xmlReader(&device);
            while (!xmlReader.atEnd()) {
                if (xmlReader.readNext() == QXmlStreamReader::StartElement
                    && xmlReader.qualifiedName() == QLatin1String("style")
                    && xmlReader.attributes().value(QLatin1String("id")) == QLatin1String("current-color-scheme")) {
                    invalidBuffers = false;

                    writer->writeStartElement(QLatin1String("style"));
                    writer->writeAttributes(xmlReader.attributes());
                    writer->writeCharacters(style_sheet.second);
                    writer->writeEndElement();

                    while (xmlReader.tokenType() != QXmlStreamReader::EndElement)
                        xmlReader.readNext();
                } else if (xmlReader.tokenType() != QXmlStreamReader::Invalid) {
                    writer->writeCurrentToken(xmlReader);
                }
            }
            // duplicate the contents also for opposite state
            //                svg_buffers[(QIcon::Normal<<4)|QIcon::On] = svg_buffers[(QIcon::Normal<<4)|QIcon::Off];
            //                svg_buffers[(QIcon::Selected<<4)|QIcon::On] = svg_buffers[(QIcon::Selected<<4)|QIcon::Off];
        }

        if (invalidBuffers) {
            // 此svg图标无ColorScheme标签时不应该再下面的操作，且应该记录下来，避免后续再处理svg文件内容
            entryToColorScheme[cache_key] = QStringLiteral("#");
            return color_entry->pixmap(size, mode, state);
        }

        // use the QSvgIconEngine
        //  - assemble the content as it is done by the operator <<(QDataStream &s, const QIcon &icon)
        //  (the QSvgIconEngine::key() + QSvgIconEngine::write())
        //  - create the QIcon from the content by usage of the QIcon::operator >>(QDataStream &s, const QIcon &icon)
        //  (icon with the (QSvgIconEngine) will be used)
        QByteArray icon_arr;
        QDataStream str {&icon_arr, QIODevice::WriteOnly};
        str.setVersion(QDataStream::Qt_4_4);
        QHash<int, QString> filenames;
        filenames[0] = color_entry->filename; // Note: filenames are ignored in the QSvgIconEngine::read()
        filenames[-1] = color_scheme; // 在dsvg插件中会为svg图标做缓存，此处是为其添加额外的缓存文件key标识，避免不同color的svg图标会命中同一个缓存文件
        str << QStringLiteral("svg") << filenames << static_cast<int>(0) /*isCompressed*/ << svg_buffers << static_cast<int>(0) /*hasAddedPimaps*/;

        QDataStream str_read {&icon_arr, QIODevice::ReadOnly};
        str_read.setVersion(QDataStream::Qt_4_4);

        str_read >> color_entry->svgIcon;
        pm = color_entry->svgIcon.pixmap(size, mode, state);

        // load the icon directly from file, if still null
        if (pm.isNull()) {
            color_entry->svgIcon = QIcon(color_entry->filename);
            pm = color_entry->svgIcon.pixmap(size, mode, state);
        }

        entryToColorScheme[cache_key] = color_scheme;
    }

    return pm;
}

QPixmap XdgIconProxyEngine::pixmapByEntry(QIconLoaderEngineEntry *entry, const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    if (!XdgIconFollowColorScheme()) {
        DEEPIN_QT_THEME::colorScheme.setLocalData(QString());

        return entry->pixmap(size, mode, state);
    }

    QPixmap pixmap;
    char *type_name = abi::__cxa_demangle(typeid(*entry).name(), 0, 0, 0);

    if (type_name == QByteArrayLiteral("ScalableFollowsColorEntry")) {
        if (DEEPIN_QT_THEME::colorScheme.localData().isEmpty()) {
            const QPalette &pal = qApp->palette();
            DEEPIN_QT_THEME::colorScheme.setLocalData(mode == QIcon::Selected ? pal.highlightedText().color().name() : pal.windowText().color().name());
        }

        pixmap = followColorPixmap(static_cast<ScalableEntry *>(entry), size, mode, state);
    } else {
        pixmap = entry->pixmap(size, mode, state);
    }

    free(type_name);
    DEEPIN_QT_THEME::colorScheme.setLocalData(QString());

    return pixmap;
}

void XdgIconProxyEngine::paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state)
{
    if (painter->device()->devType() == QInternal::Widget
        && XdgIconFollowColorScheme()
        && DEEPIN_QT_THEME::colorScheme.localData().isEmpty()) {
        const QPalette &pal = qvariant_cast<QPalette>(dynamic_cast<QObject *>(painter->device())->property("palette"));
        DEEPIN_QT_THEME::colorScheme.setLocalData(mode == QIcon::Selected ? pal.highlightedText().color().name() : pal.windowText().color().name());
    }

    const QPixmap pix = pixmap(rect.size(), mode, state);

    if (pix.isNull())
        return;

    painter->drawPixmap(rect, pix);
}

QPixmap XdgIconProxyEngine::pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    engine->ensureLoaded();

    QIconLoaderEngineEntry *entry = engine->entryForSize(size);

    if (!entry) {
        DEEPIN_QT_THEME::colorScheme.setLocalData(QString());

        return QPixmap();
    }

    return pixmapByEntry(entry, size, mode, state);
}

void XdgIconProxyEngine::addPixmap(const QPixmap &pixmap, QIcon::Mode mode, QIcon::State state)
{
    return engine->addPixmap(pixmap, mode, state);
}

void XdgIconProxyEngine::addFile(const QString &fileName, const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    return engine->addFile(fileName, size, mode, state);
}

QString XdgIconProxyEngine::key() const
{
    return QLatin1String("XdgIconProxyEngine");
}

QSize XdgIconProxyEngine::actualSize(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    return engine->actualSize(size, mode, state);
}

QIconEngine *XdgIconProxyEngine::clone() const
{
    return new XdgIconProxyEngine(static_cast<XdgIconLoaderEngine *>(engine->clone()));
}

bool XdgIconProxyEngine::read(QDataStream &in)
{
    return engine->read(in);
}

bool XdgIconProxyEngine::write(QDataStream &out) const
{
    return engine->write(out);
}

void XdgIconProxyEngine::virtual_hook(int id, void *data)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 9, 0)
    if (id == QIconEngine::ScaledPixmapHook) {
        engine->ensureLoaded();

        QIconEngine::ScaledPixmapArgument &arg = *reinterpret_cast<QIconEngine::ScaledPixmapArgument *>(data);
        // QIcon::pixmap() multiplies size by the device pixel ratio.
        const int integerScale = qCeil(arg.scale);
        QIconLoaderEngineEntry *entry = engine->entryForSize(arg.size / integerScale, integerScale);
        // 先禁用缩放，因为此size是已经缩放过的
        bool useHighDpiPixmap = qGuiApp->testAttribute(Qt::AA_UseHighDpiPixmaps);
        qGuiApp->setAttribute(Qt::AA_UseHighDpiPixmaps, false);
        arg.pixmap = entry ? pixmapByEntry(entry, arg.size, arg.mode, arg.state) : QPixmap();
        qGuiApp->setAttribute(Qt::AA_UseHighDpiPixmaps, useHighDpiPixmap);
        DEEPIN_QT_THEME::colorScheme.setLocalData(QString());

        return;
    }
#endif

    return engine->virtual_hook(id, data);
}

DGUI_END_NAMESPACE
