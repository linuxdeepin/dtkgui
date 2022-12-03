// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ddciicon.h"
#include "dguiapplicationhelper.h"
#include "dicontheme.h"

#include <DObjectPrivate>
#include <DDciFile>
#include <DPlatformTheme>
#include <DSGApplication>
#include <private/qguiapplication_p.h>

#include <QPainter>
#include <QBuffer>
#include <QImageReader>
#include <QtMath>

DCORE_USE_NAMESPACE
DGUI_BEGIN_NAMESPACE

#define MODE_NORMAL "normal"
#define MODE_DISABLED "disabled"
#define MODE_HOVER "hover"
#define MODE_PRESSED "pressed"

#define THEME_LIGHT "light"
#define THEME_DARK "dark"
#define ALPHA8STRING "alpha8"

struct DDciIconEntry {
    struct ScalableLayer {
        int imagePixelRatio = 0;
        struct Layer {
            int prior = 0;
            DDciIconPalette::PaletteRole role = DDciIconPalette::NoPalette;
            QByteArray format;
            QByteArray data;
            bool isAlpha8Format = false;

            qint8 hue = 0;
            qint8 saturation = 0;
            qint8 lightness = 0;
            qint8 red = 0;
            qint8 green = 0;
            qint8 blue = 0;
            qint8 alpha = 0;
            qint16 padding = 0;
        };
        QVector<Layer> layers;
    };


    int iconSize = 0;
    DDciIcon::Mode mode = DDciIcon::Normal;
    DDciIcon::Theme theme = DDciIcon::Light;
    QVector<ScalableLayer> scalableLayers;
    inline bool isNull() const { return scalableLayers.isEmpty(); }
};

struct EntryNode {
    int iconSize;
    QVector<DDciIconEntry *> entries;
};
using EntryNodeList = QVector<EntryNode>;

class EntryPropertyParser
{
public:
    static void registerSteps();
    static void doParse(DDciIconEntry::ScalableLayer::Layer *layer, const QVector<QStringRef> &properties);
private:
    static struct Step {
        virtual ~Step() {}
        Step *nextStep = nullptr;
        virtual QVector<QStringRef> parse(DDciIconEntry::ScalableLayer::Layer *layer, const QVector<QStringRef> &properties) = 0;
    } *root;

    struct PriorStep : public Step {
        QVector<QStringRef> parse(DDciIconEntry::ScalableLayer::Layer *layer, const QVector<QStringRef> &properties) override;
    };

    struct FormatAndAlpha8Step : public Step {
        QVector<QStringRef> parse(DDciIconEntry::ScalableLayer::Layer *layer, const QVector<QStringRef> &properties) override;
    };

    struct PaddingStep : public Step {
        QVector<QStringRef> parse(DDciIconEntry::ScalableLayer::Layer *layer, const QVector<QStringRef> &properties) override;
    };

    struct PaletteStep : public Step {
        QVector<QStringRef> parse(DDciIconEntry::ScalableLayer::Layer *layer, const QVector<QStringRef> &properties) override;
    };
};

EntryPropertyParser::Step *EntryPropertyParser::root = nullptr;

void EntryPropertyParser::registerSteps()
{
    static PriorStep priorSt;
    static FormatAndAlpha8Step formatSt;
    static PaddingStep paddingSt;
    static PaletteStep paletteSt;

    priorSt.nextStep = &formatSt;
    formatSt.nextStep = &paddingSt;
    paddingSt.nextStep = &paletteSt;
    paletteSt.nextStep = nullptr;
    root = &priorSt;
}

void EntryPropertyParser::doParse(DDciIconEntry::ScalableLayer::Layer *layer, const QVector<QStringRef> &properties)
{
    Q_ASSERT(layer);
    if (!root)
        registerSteps();
    EntryPropertyParser::Step *step = root;
    QVector<QStringRef> ps = properties;
    while (step) {
        // If the input information flow is empty, it means that the next steps do not need to be continued
        if (ps.isEmpty())
            break;
        ps = step->parse(layer, ps);
        step = step->nextStep;
    }
}

QVector<QStringRef> EntryPropertyParser::PriorStep::parse(DDciIconEntry::ScalableLayer::Layer *layer, const QVector<QStringRef> &properties)
{
    bool ok = false;
    QVector<QStringRef> ps = properties;
    layer->prior = ps.takeFirst().toInt(&ok);
    if (!ok)
        return {};  // error priority.
    return ps;
}

QVector<QStringRef> EntryPropertyParser::FormatAndAlpha8Step::parse(DDciIconEntry::ScalableLayer::Layer *layer, const QVector<QStringRef> &properties)
{
    QVector<QStringRef> ps = properties;
    const QString alpha8OrFormat = ps.takeLast().toString();
    if (alpha8OrFormat.compare(ALPHA8STRING, Qt::CaseInsensitive) == 0) {
        // Alpha8 format
        layer->isAlpha8Format = true;
        layer->format = ps.takeLast().toLatin1();
    } else {
        // normal format
        layer->format = alpha8OrFormat.toLatin1();
    }

    return ps;
}

QVector<QStringRef> EntryPropertyParser::PaddingStep::parse(DDciIconEntry::ScalableLayer::Layer *layer, const QVector<QStringRef> &properties)
{
    QVector<QStringRef> ps = properties;
    // Take the padding property
    auto it = std::find_if(ps.cbegin(), ps.cend(), [](const QStringRef &p) {
        return p.endsWith(QLatin1Char('p'));
    });

    if (it != ps.cend()) {
        layer->padding = it->left(it->length() - 1).toShort();
        ps.removeOne(*it);
    }

    return ps;
}

QVector<QStringRef> EntryPropertyParser::PaletteStep::parse(DDciIconEntry::ScalableLayer::Layer *layer, const QVector<QStringRef> &properties)
{
    QVector<QStringRef> ps = properties;
    QStringRef palettes = ps.takeFirst();
    // `role_hue_saturation_lightness_red_green_blue_alpha` or `role`
    if (palettes.contains(QLatin1Char('_'))) {
        const QVector<QStringRef> paletteProps = palettes.split(QLatin1Char('_'));
        if (paletteProps.length() != 8)
            return ps;

        int role = paletteProps.at(0).toInt();
        if (role < DDciIconPalette::NoPalette || role > DDciIconPalette::PaletteCount)
            role = DDciIconPalette::NoPalette;

        layer->role = DDciIconPalette::PaletteRole(role);
        layer->hue = static_cast<qint8>(paletteProps.at(1).toShort());
        layer->saturation = static_cast<qint8>(paletteProps.at(2).toShort());
        layer->lightness = static_cast<qint8>(paletteProps.at(3).toShort());
        layer->red = static_cast<qint8>(paletteProps.at(4).toShort());
        layer->green = static_cast<qint8>(paletteProps.at(5).toShort());
        layer->blue = static_cast<qint8>(paletteProps.at(6).toShort());
        layer->alpha = static_cast<qint8>(paletteProps.at(7).toShort());
    } else {
        // only palette
        int role = palettes.toInt();
        if (role < DDciIconPalette::NoPalette || role > DDciIconPalette::PaletteCount)
            role = DDciIconPalette::NoPalette;
        layer->role = DDciIconPalette::PaletteRole(role);
    }

    return ps;
}

class DDciIconPrivate : public QSharedData
{
public:
    DDciIconPrivate()
        : dciFile(nullptr)
    {
    }

    DDciIconPrivate(const DDciIconPrivate &other)
        : QSharedData(other)
        , dciFile(other.dciFile)
    {
    }

    ~DDciIconPrivate();

    DDciIconEntry *loadIcon(const QString &parentDir, const QString &imageDir);
    void loadIconList();
    void ensureLoaded();

    DDciIconEntry *tryMatchIcon(int iconSize, DDciIcon::Theme theme, DDciIcon::Mode mode, DDciIcon::IconMatchedFlags flags = DDciIcon::None) const;
    void paint(QPainter *painter, const QRect &rect, qreal devicePixelRatio, Qt::Alignment alignment,
               const DDciIconEntry *entry, const DDciIconPalette &palette, qreal pixmapScale) const;
    bool hasPalette(DDciIconMatchResult result) const;
    static inline bool entryIsValid(const DDciIconEntry *entry) {
        return entry && !entry->isNull();
    }

    QSharedPointer<const DDciFile> dciFile;
    EntryNodeList icons;
};

#ifndef QT_NO_DATASTREAM
__attribute__((constructor))
static void registerMetaType()
{
    qRegisterMetaTypeStreamOperators<DDciIcon>("DDciIcon");
}
#endif

static inline bool toMode(const QStringRef &name, DDciIcon::Mode *mode) {
    if (name == QLatin1String(MODE_NORMAL)) {
        *mode = DDciIcon::Normal;
        return true;
    }

    if (name == QLatin1String(MODE_DISABLED)) {
        *mode = DDciIcon::Disabled;
        return true;
    }

    if (name == QLatin1String(MODE_HOVER)) {
        *mode = DDciIcon::Hover;
        return true;
    }

    if (name == QLatin1String(MODE_PRESSED)) {
        *mode = DDciIcon::Pressed;
        return true;
    }

    return false;
}

static inline bool toTheme(const QStringRef &name, DDciIcon::Theme *theme) {
    if (name == QLatin1String(THEME_LIGHT)) {
        *theme = DDciIcon::Light;
        return true;
    }

    if (name == QLatin1String(THEME_DARK)) {
        *theme = DDciIcon::Dark;
        return true;
    }

    return false;
}

// DCI 内部目录使用 '/' 作为路径分隔符
static inline QString joinPath(const QString &s1, const QString &s2) {
    return s1 + QLatin1Char('/') + s2;
}

void alpha8ImageDeleter(void *image) {
    delete (QImage *)image;
}

static QImage readImageData(const QByteArray &data, const QByteArray &format, qreal pixmapScale, bool isAlpha8Format)
{
    if (data.isEmpty())
        return QImage();

    QBuffer rawDataBuffer;
    rawDataBuffer.setData(data);
    rawDataBuffer.open(QBuffer::ReadOnly);

    QImageReader reader(&rawDataBuffer);
    if (!format.isEmpty())
        reader.setFormat(format);

    QImage image;
    QImage *imagePtr = &image;
    if (isAlpha8Format)
        imagePtr = new QImage();

    if (reader.canRead()) {
        bool scaled = false;
        int imageSize = qMax(reader.size().width(), reader.size().height());
        int scaledSize = qRound(pixmapScale * imageSize);
        if (!isAlpha8Format && reader.supportsOption(QImageIOHandler::ScaledSize)) {
            reader.setScaledSize(reader.size().scaled(scaledSize, scaledSize, Qt::KeepAspectRatio));
            scaled = true;
        }

        reader.read(imagePtr);
        if (isAlpha8Format) {
            QImage tt(imagePtr->bits(), imagePtr->width(), imagePtr->width(), imagePtr->bytesPerLine(),
                      QImage::Format_Alpha8, alpha8ImageDeleter, (void *)imagePtr);
            return tt.scaled(scaledSize, scaledSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }

        if (!scaled)
            image = image.scaled(scaledSize, scaledSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    } else {
        qWarning() << reader.errorString() << format;
    }

    return image;
}

static int findIconsByLowerBoundSize(const EntryNodeList &list, const int size)
{
    const auto compFun = [] (const EntryNode &n1, const EntryNode &n2) {
        return n1.iconSize < n2.iconSize;
    };
    EntryNode target;
    target.iconSize = size;
    auto neighbor = std::lower_bound(list.cbegin(), list.cend(), target, compFun);

    if (neighbor != list.cend())
        return neighbor - list.constBegin();
    return -1;
}

static int findMaxEntryPadding(const DDciIconEntry *entry)
{
    if (entry->scalableLayers.isEmpty())
        return 0;

    // Only take the first padding, because it has the same padding.
    auto scalables = entry->scalableLayers.first();
    if (scalables.layers.isEmpty())
        return 0;

    auto iter = std::max_element(scalables.layers.cbegin(), scalables.layers.cend(), [](const DDciIconEntry::ScalableLayer::Layer &n1,
                     const DDciIconEntry::ScalableLayer::Layer &n2) {
        return n1.padding < n2.padding;
    });

    return iter->padding;
}

static QRect alignedRect(Qt::LayoutDirection direction, Qt::Alignment alignment, const QSize &size, const QRect &rect)
{
    alignment = QGuiApplicationPrivate::visualAlignment(direction, alignment);
    int x = rect.x();
    int y = rect.y();
    int w = size.width();
    int h = size.height();
    if ((alignment & Qt::AlignVCenter) == Qt::AlignVCenter)
        y += rect.size().height()/2 - h/2;
    else if ((alignment & Qt::AlignBottom) == Qt::AlignBottom)
        y += rect.size().height() - h;
    if ((alignment & Qt::AlignRight) == Qt::AlignRight)
        x += rect.size().width() - w;
    else if ((alignment & Qt::AlignHCenter) == Qt::AlignHCenter)
        x += rect.size().width()/2 - w/2;

    return QRect(x, y, w, h);
}

DDciIconPrivate::~DDciIconPrivate()
{
    for (auto icon : icons)
        qDeleteAll(icon.entries);
}

DDciIconEntry *DDciIconPrivate::loadIcon(const QString &parentDir, const QString &imageDir)
{
    // Mode-Theme
    const QVector<QStringRef> &iconProps = imageDir.splitRef(QLatin1Char('.'));
    if (iconProps.count() != 2) // Error file name.
        return nullptr;

    DDciIcon::Mode mode;
    DDciIcon::Theme theme;
    if (!toMode(iconProps[0], &mode))
        return nullptr;
    if (!toTheme(iconProps[1], &theme))
        return nullptr;

    DDciIconEntry *icon = new DDciIconEntry();
    icon->mode = mode;
    icon->theme = theme;
    const QString &stateDir = joinPath(parentDir, imageDir);
    for (const QString &scaleString : dciFile->list(stateDir, true)) {
        bool ok = false;
        int scale = scaleString.toInt(&ok);
        // No restrictions on icons with scale ratio greater than 3.
        if (!ok)
            continue;
        DDciIconEntry::ScalableLayer scaleIcon;
        scaleIcon.imagePixelRatio = scale;
        const QString &path = joinPath(stateDir, scaleString);
        for (const QString &layerPath : dciFile->list(path, true)) {
            QVector<QStringRef> layerProps = layerPath.splitRef(QLatin1Char('.'));
            DDciIconEntry::ScalableLayer::Layer layer;
            EntryPropertyParser::doParse(&layer, layerProps);
            layer.data = dciFile->dataRef(joinPath(path, layerPath));
            // The sequence number starts with 1, convert it into index.
            scaleIcon.layers.insert(layer.prior - 1, layer);
        }
        icon->scalableLayers.append(scaleIcon);
    }

    return icon;
}

void DDciIconPrivate::loadIconList()
{
    const QStringList &listOfSize = dciFile->list(QLatin1String("/"), true);
    for (const QString &dir : listOfSize) {
        bool ok = false;
        int size = dir.toInt(&ok);
        if (!ok)
            continue;

        EntryNode node;
        node.iconSize = size;
        const QString &dirPath = joinPath(QLatin1String(), dir);
        for (const QString &imageDir : dciFile->list(dirPath, true)) {
            auto icon = loadIcon(dirPath, imageDir);
            if (!icon || icon->isNull())
                continue;
            icon->iconSize = size;
            node.entries << icon;
        }
        icons << std::move(node);
    }
}

void DDciIconPrivate::ensureLoaded()
{
    // TODO: Modified to resemble the addFile function in QIcon.
    if (!dciFile->isValid())
        return;
    loadIconList();
}

DDciIconEntry *DDciIconPrivate::tryMatchIcon(int iconSize, DDciIcon::Theme theme, DDciIcon::Mode mode, DDciIcon::IconMatchedFlags flags) const
{
    if (icons.isEmpty())
        return nullptr;

    auto neighborIndex = findIconsByLowerBoundSize(icons, iconSize);
    if (neighborIndex < 0) {
        neighborIndex = icons.size() - 1;
    }

    const auto &listOfSize = icons.at(neighborIndex);

    QVector<qint8> iconWeight;
    iconWeight.resize(listOfSize.entries.size());
    for (int i = 0; i < listOfSize.entries.size(); ++i) {
        qint8 weight = 0;
        const DDciIconEntry *icon = listOfSize.entries.at(i);

        if (icon->mode == mode) {
            weight += 2;
        } else if (flags.testFlag(DDciIcon::DontFallbackMode)
                   || icon->mode != DDciIcon::Normal) {
            weight = -1;
            continue;
        }
        if (icon->theme == theme) {
            weight += 1;
        } else {
            weight = -1;
            continue;
        }

        iconWeight[i] = weight;
    }

    const auto targetIcon = std::max_element(iconWeight.constBegin(), iconWeight.constEnd());
    if (*targetIcon > 0)
        return listOfSize.entries.at(targetIcon - iconWeight.constBegin());
    return nullptr;
}

void DDciIconPrivate::paint(QPainter *painter, const QRect &rect, qreal devicePixelRatio, Qt::Alignment alignment,
                            const DDciIconEntry *entry, const DDciIconPalette &palette, qreal pixmapScale) const
{
    qreal pixelRatio = devicePixelRatio;
    if (pixelRatio <= 0 && painter->device())
        pixelRatio = painter->device()->devicePixelRatio();

    typedef DDciIconEntry::ScalableLayer IconScalableData;
    auto pixelRatioCompare =  [](const IconScalableData &d1, const IconScalableData &d2) {
        return d1.imagePixelRatio < d2.imagePixelRatio;
    };

    IconScalableData targetData;
    targetData.imagePixelRatio = qCeil(pixelRatio);

    auto closestData = std::lower_bound(entry->scalableLayers.begin(), entry->scalableLayers.end(),
                                        targetData, pixelRatioCompare);

    if (closestData == entry->scalableLayers.end())
        closestData = std::max_element(entry->scalableLayers.begin(), entry->scalableLayers.end(), pixelRatioCompare);

    Q_ASSERT(closestData != entry->scalableLayers.end());
    targetData = *closestData;
    const QRect iconRect = alignedRect(painter->layoutDirection(), alignment, rect.size(), rect);
    for (auto layerIter = targetData.layers.begin(); layerIter != targetData.layers.end(); ++layerIter) {
        if (layerIter->data.isEmpty())
            continue;
        qreal deviceRatio = 1.0;
        if (qApp->testAttribute(Qt::AA_UseHighDpiPixmaps)) {
            deviceRatio = pixelRatio;
        }
        const QImage &layer = readImageData(layerIter->data, layerIter->format, pixmapScale * deviceRatio / targetData.imagePixelRatio, layerIter->isAlpha8Format);
        if (layer.isNull())
            continue;
        QColor fillColor;
        switch (layerIter->role) {
        case DDciIconPalette::Foreground:
            fillColor = palette.foreground();
            break;
        case DDciIconPalette::Background:
            fillColor = palette.background();
            break;
        case DDciIconPalette::HighlightForeground:
            fillColor = palette.highlightForeground();
            break;
        case DDciIconPalette::Highlight:
            fillColor = palette.highlight();
            break;
        default:
            break;
        }

        // ###(Chen Bin) Can't compose image when this image's format is Format_Alpha8.
        QImage tmp(layer.width(), layer.height(), QImage::Format_ARGB32_Premultiplied);
        tmp.fill(Qt::transparent);
        QPainter render;
        render.begin(&tmp);
        render.drawImage(tmp.rect(), layer);

        if (fillColor.isValid()) {
            fillColor = DGuiApplicationHelper::adjustColor(fillColor, layerIter->hue, layerIter->saturation,
                                                           layerIter->lightness, layerIter->red, layerIter->green,
                                                           layerIter->blue, layerIter->alpha);
            render.setCompositionMode(QPainter::CompositionMode_SourceIn);
            render.fillRect(layer.rect(), fillColor);
        }

        render.end();
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        QRect sourceRect(0, 0, layer.width(), layer.height());
        sourceRect.moveCenter(iconRect.center());
        QRect targetRect = sourceRect;
        if (iconRect.width() < sourceRect.width())
            targetRect = iconRect;

        // Use AA_UseHighDpiPixmaps will solve blurred icon.
        painter->drawImage(targetRect, tmp);
        painter->restore();
    }

}

bool DDciIconPrivate::hasPalette(DDciIconMatchResult result) const
{
    auto entry = static_cast<const DDciIconEntry *>(result);
    if (!entryIsValid(entry))
        return false;
    if (entry->scalableLayers.isEmpty())
        return false;
    auto scaledLayer = entry->scalableLayers.first();
    return std::any_of(scaledLayer.layers.cbegin(), scaledLayer.layers.cend(),
                [](const DDciIconEntry::ScalableLayer::Layer &layer) {
        return layer.role != DDciIconPalette::NoPalette;
    });
}

DDciIcon::DDciIcon()
    : d(new DDciIconPrivate())
{

}

DDciIcon::DDciIcon(const DDciFile *dciFile)
    : DDciIcon()
{
    d->dciFile.reset(dciFile);
    d->ensureLoaded();
}

DDciIcon::DDciIcon(const QString &fileName)
    : DDciIcon()
{
    d->dciFile.reset(new DDciFile(fileName));
    d->ensureLoaded();
}

DDciIcon::DDciIcon(const QByteArray &data)
    : DDciIcon()
{
    d->dciFile.reset(new DDciFile(data));
    d->ensureLoaded();
}

DDciIcon::DDciIcon(const DDciIcon &other)
    : d(other.d)
{ }

DDciIcon &DDciIcon::operator=(const DDciIcon &other) noexcept
{
    d = other.d;
    return *this;
}

DDciIcon::~DDciIcon() {}

DDciIcon::DDciIcon(DDciIcon &&other) noexcept
    : d(other.d)
{ other.d = nullptr; }

DDciIcon &DDciIcon::operator=(DDciIcon &&other) noexcept
{ swap(other); return *this; }

bool DDciIcon::isNull() const
{
    return d->icons.isEmpty();
}

DDciIconMatchResult DDciIcon::matchIcon(int size, Theme theme, Mode mode, IconMatchedFlags flags) const
{
    return d->tryMatchIcon(size, theme, mode, flags);
}

int DDciIcon::actualSize(DDciIconMatchResult result) const
{
    auto entry = static_cast<const DDciIconEntry *>(result);
    if (!d->entryIsValid(entry))
        return -1;
    return entry->iconSize;
}

int DDciIcon::actualSize(int size, DDciIcon::Theme theme, DDciIcon::Mode mode) const
{
    auto entry = d->tryMatchIcon(size, theme, mode);
    return actualSize(entry);
}

QList<int> DDciIcon::availableSizes(DDciIcon::Theme theme, DDciIcon::Mode mode) const
{
    if (d->icons.isEmpty())
        return {};
    QList<int> sizes;
    std::for_each(d->icons.begin(), d->icons.end(), [theme, mode, &sizes](const EntryNode &node) {
        auto it = std::find_if(node.entries.begin(), node.entries.end(), [theme, mode](const DDciIconEntry *entry) {
            if (entry->mode == mode && entry->theme == theme)
                return true;
            return false;
        });
        if (it != node.entries.end())
            sizes.append((*it)->iconSize);
    });
    return sizes;
}

bool DDciIcon::isSupportedAttribute(DDciIconMatchResult result, IconAttibute attr) const
{
    switch (attr) {
    case HasPalette:
        return d->hasPalette(result);
    default:
        break;
    }

    return false;
}

QPixmap DDciIcon::pixmap(qreal devicePixelRatio, int iconSize, DDciIcon::Theme theme, DDciIcon::Mode mode, const DDciIconPalette &palette) const
{
    auto entry = d->tryMatchIcon(iconSize, theme, mode);
    return pixmap(devicePixelRatio, iconSize, entry, palette);
}

QPixmap DDciIcon::pixmap(qreal devicePixelRatio, int iconSize, DDciIconMatchResult result, const DDciIconPalette &palette) const
{
    auto entry = static_cast<const DDciIconEntry *>(result);
    if (!d->entryIsValid(entry))
        return QPixmap();

    if (iconSize <= 0)
        iconSize = entry->iconSize;

    Q_ASSERT_X((iconSize > 0), "DDciIcon::generatePixmap", "You must specify the icon size.");
    const qreal pixmapScale = iconSize * 1.0 / entry->iconSize;

    qreal deviceRatio = 1.0;
    if (qApp->testAttribute(Qt::AA_UseHighDpiPixmaps)) {
        deviceRatio = devicePixelRatio;
    }
    const int pixmapSize = qRound((findMaxEntryPadding(entry) * 2 + entry->iconSize) * pixmapScale * deviceRatio);

    QPixmap pixmap(pixmapSize, pixmapSize);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::NoBrush);
    d->paint(&painter, pixmap.rect(), devicePixelRatio, Qt::AlignCenter, entry, palette, pixmapScale);
    painter.end();
    pixmap.setDevicePixelRatio(devicePixelRatio);
    return pixmap;
}

void DDciIcon::paint(QPainter *painter, const QRect &rect, qreal devicePixelRatio, DDciIcon::Theme theme, DDciIcon::Mode mode,
                     Qt::Alignment alignment, const DDciIconPalette &palette) const
{
    int iconSize = qMax(rect.width(), rect.height());
    auto entry = d->tryMatchIcon(iconSize, theme, mode);
    return paint(painter, rect, devicePixelRatio, entry, alignment, palette);
}

void DDciIcon::paint(QPainter *painter, const QRect &rect, qreal devicePixelRatio, DDciIconMatchResult result, Qt::Alignment alignment, const DDciIconPalette &palette) const
{
    auto entry = static_cast<const DDciIconEntry *>(result);
    if (!d->entryIsValid(entry))
        return;
    int iconSize = qMax(rect.width(), rect.height());
    const qreal pixmapScale = iconSize * 1.0 / (entry->iconSize + findMaxEntryPadding(entry) * 2);
    d->paint(painter, rect, devicePixelRatio, alignment, entry, palette, pixmapScale);
}

DDciIcon DDciIcon::fromTheme(const QString &name)
{
    DDciIcon icon;

    QString iconName = name;
    // FIX uengine appname is empty, will cause qt_assert
    if (!QCoreApplication::applicationName().isEmpty() &&  !DSGApplication::id().isEmpty()) {
        // allow the icon theme to override the icon for a given application
        iconName.prepend(DSGApplication::id() + "/");
    }

    QString iconPath;
    QString iconThemeName =DGuiApplicationHelper::instance()->applicationTheme()->iconThemeName();
    if (auto cached = DIconTheme::cached()) {
        iconPath = cached->findDciIconFile(iconName, iconThemeName);
    } else {
        iconPath = DIconTheme::findDciIconFile(iconName, iconThemeName);
    }

    if (!iconPath.isEmpty())
        icon = DDciIcon(iconPath);

    return icon;
}

DDciIcon DDciIcon::fromTheme(const QString &name, const DDciIcon &fallback)
{
    DDciIcon icon = fromTheme(name);

    if (icon.isNull() || icon.availableSizes(Light).isEmpty() || icon.availableSizes(Dark).isEmpty())
        return fallback;

    return icon;
}

#ifndef QT_NO_DATASTREAM
QDataStream &operator<<(QDataStream &s, const DDciIcon &icon)
{
    if (icon.isNull())
        return s << QByteArray();
    auto dciFile = icon.d->dciFile;
    const QByteArray &data = dciFile->toData();
    s << data;
    return s;
}

QDataStream &operator>>(QDataStream &s, DDciIcon &icon)
{
    QByteArray data;
    s >> data;
    icon = DDciIcon(data);
    return s;
}
#endif

DGUI_END_NAMESPACE
