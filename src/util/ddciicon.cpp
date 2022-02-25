/*
 * Copyright (C) 2021 UnionTech Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "ddciicon.h"
#include "dguiapplicationhelper.h"

#include <DObjectPrivate>
#include <DDciFile>

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
        int imagePixelRatio;
        struct Layer {
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


    int iconSize;
    DDciIcon::Mode mode;
    DDciIcon::Theme theme;
    QVector<ScalableLayer> scalableLayers;
    inline bool isNull() const { return scalableLayers.isEmpty(); }
};

struct EntryNode {
    int iconSize;
    QVector<DDciIconEntry> entries;
};
using EntryNodeList = QVector<EntryNode>;

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

    ~DDciIconPrivate() { }

    DDciIconEntry loadIcon(const QString &parentDir, const QString &imageDir);
    void loadIconList();
    void ensureLoaded();

    DDciIconEntry tryMatchIcon(int iconSize, DDciIcon::Theme theme, DDciIcon::Mode mode) const;
    void paint(QPainter *painter, const QRect &rect, qreal devicePixelRatio, Qt::Alignment alignment,
               const DDciIconEntry &entry, const DDciIconPalette &palette, qreal pixmapScale) const;

    QSharedPointer<const DDciFile> dciFile;
    EntryNodeList icons;
};

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

static int findMaxEntryPadding(const DDciIconEntry &entry)
{
    if (entry.scalableLayers.isEmpty())
        return 0;

    // Only take the first padding, because it has the same padding.
    auto scalables = entry.scalableLayers.first();
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

DDciIconEntry DDciIconPrivate::loadIcon(const QString &parentDir, const QString &imageDir)
{
    // Mode-Theme
    const QVector<QStringRef> &iconProps = imageDir.splitRef(QLatin1Char('.'));
    if (iconProps.count() != 2) // Error file name.
        return DDciIconEntry();
    DDciIconEntry icon;

    if (!toMode(iconProps[0], &icon.mode))
        return DDciIconEntry();
    if (!toTheme(iconProps[1], &icon.theme))
        return DDciIconEntry();

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
            int prior = layerProps.first().toInt();
            if (prior == -1)
                continue;  // error priority.
            DDciIconEntry::ScalableLayer::Layer layer;
            const QString alpha8OrFormat = layerProps.last().toLatin1();
            if (alpha8OrFormat.compare(ALPHA8STRING, Qt::CaseInsensitive) == 0) {
                layer.isAlpha8Format = true;
                layer.format = layerProps.at(layerProps.length() - 2).toLatin1();
                layerProps.removeLast();
            } else {
                layer.format = alpha8OrFormat.toLatin1();
            }

            if (layerProps.length() > 2) {
                // prior.palette.format || prior.padding.palette.format
                QStringRef paletteString;
                QStringRef paletteOrPaddingString = layerProps.at(1);
                if (layerProps.length() == 4)
                    paletteString = layerProps.at(2);

                if (paletteOrPaddingString.endsWith(QLatin1Char('p'))) {
                    // padding
                    layer.padding = paletteOrPaddingString.left(paletteOrPaddingString.length() - 1).toShort();
                } else {
                    paletteString = paletteOrPaddingString;
                }

                if (paletteString.contains(QLatin1Char('_'))) {
                    const QVector<QStringRef> paletteProps = paletteString.split(QLatin1Char('_'));
                    Q_ASSERT(paletteProps.length() == 8);
                    int role = paletteProps.at(0).toInt();
                    if (role < DDciIconPalette::NoPalette || role > DDciIconPalette::PaletteCount)
                        role = DDciIconPalette::NoPalette;

                    layer.role = DDciIconPalette::PaletteRole(role);
                    layer.hue = paletteProps.at(1).toShort();
                    layer.saturation = paletteProps.at(2).toShort();
                    layer.lightness = paletteProps.at(3).toShort();
                    layer.red = paletteProps.at(4).toShort();
                    layer.green = paletteProps.at(5).toShort();
                    layer.blue = paletteProps.at(6).toShort();
                    layer.alpha = paletteProps.at(7).toShort();
                } else {
                    // Only palette role.
                    int role = paletteString.toInt();
                    if (role < DDciIconPalette::NoPalette || role > DDciIconPalette::PaletteCount)
                        role = DDciIconPalette::NoPalette;
                    layer.role = DDciIconPalette::PaletteRole(role);
                }
            }
            layer.data = dciFile->dataRef(joinPath(path, layerPath));
            // The sequence number starts with 1, convert it into index.
            scaleIcon.layers.insert(prior - 1, layer);
        }
        icon.scalableLayers.append(scaleIcon);
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
            if (icon.isNull())
                continue;
            icon.iconSize = size;
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

DDciIconEntry DDciIconPrivate::tryMatchIcon(int iconSize, DDciIcon::Theme theme, DDciIcon::Mode mode) const
{
    if (icons.isEmpty())
        return DDciIconEntry();

    auto neighborIndex = findIconsByLowerBoundSize(icons, iconSize);
    if (neighborIndex < 0) {
        neighborIndex = icons.size() - 1;
    }

    const auto &listOfSize = icons.at(neighborIndex);
    QVector<qint8> iconWeight;
    iconWeight.resize(listOfSize.entries.size());

    for (int i = 0; i < listOfSize.entries.size(); ++i) {
        qint8 weight = 0;
        const DDciIconEntry &icon = listOfSize.entries.at(i);

        if (icon.mode == mode) {
            weight += 2;
        } else if (icon.mode != DDciIcon::Normal) {
            weight = -1;
            continue;
        }
        if (icon.theme == theme) {
            weight += 1;
        } else {
            weight = -1;
            continue;
        }

        iconWeight.insert(i, weight);
    }

    const auto targetIcon = std::max_element(iconWeight.constBegin(), iconWeight.constEnd());
    if (*targetIcon > 0)
        return listOfSize.entries.at(targetIcon - iconWeight.constBegin());
    return DDciIconEntry();
}

void DDciIconPrivate::paint(QPainter *painter, const QRect &rect, qreal devicePixelRatio, Qt::Alignment alignment,
                            const DDciIconEntry &entry, const DDciIconPalette &palette, qreal pixmapScale) const
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

    auto closestData = std::lower_bound(entry.scalableLayers.begin(), entry.scalableLayers.end(),
                                        targetData, pixelRatioCompare);

    if (closestData == entry.scalableLayers.end())
        closestData = std::max_element(entry.scalableLayers.begin(), entry.scalableLayers.end(), pixelRatioCompare);

    Q_ASSERT(closestData != entry.scalableLayers.end());
    targetData = *closestData;
    const QRect iconRect = alignedRect(painter->layoutDirection(), alignment, rect.size(), rect);
    for (auto layerIter = targetData.layers.begin(); layerIter != targetData.layers.end(); ++layerIter) {
        if (layerIter->data.isEmpty())
            continue;
        const QImage &layer = readImageData(layerIter->data, layerIter->format, pixmapScale * pixelRatio / targetData.imagePixelRatio, layerIter->isAlpha8Format);
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
        painter->drawImage(sourceRect, tmp);
        painter->restore();
    }
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

int DDciIcon::actualSize(int size, DDciIcon::Theme theme, DDciIcon::Mode mode) const
{
    auto entry = d->tryMatchIcon(size, theme, mode);
    if (!entry.isNull())
        return entry.iconSize;
    return -1;
}

QList<int> DDciIcon::availableSizes(DDciIcon::Theme theme, DDciIcon::Mode mode) const
{
    if (d->icons.isEmpty())
        return {};
    QList<int> sizes;
    std::for_each(d->icons.begin(), d->icons.end(), [theme, mode, &sizes](const EntryNode &node) {
        auto it = std::find_if(node.entries.begin(), node.entries.end(), [theme, mode](const DDciIconEntry &entry) {
            if (entry.mode == mode && entry.theme == theme)
                return true;
            return false;
        });
        if (it != node.entries.end())
            sizes.append(it->iconSize);
    });
    return sizes;
}

QPixmap DDciIcon::pixmap(qreal devicePixelRatio, int iconSize, DDciIcon::Theme theme, DDciIcon::Mode mode, const DDciIconPalette &palette) const
{
    auto entry = d->tryMatchIcon(iconSize, theme, mode);
    if (entry.isNull())
        return QPixmap();

    if (iconSize <= 0)
        iconSize = entry.iconSize;
    Q_ASSERT_X((iconSize > 0), "DDciIcon::generatePixmap", "You must specify the icon size.");
    const qreal pixmapScale = iconSize * 1.0 / entry.iconSize;
    const int pixmapSize = qRound((findMaxEntryPadding(entry) * 2 + entry.iconSize) * pixmapScale * devicePixelRatio);

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
    if (entry.isNull())
        return;
    const qreal pixmapScale = iconSize * 1.0 / (entry.iconSize + findMaxEntryPadding(entry) * 2);
    d->paint(painter, rect, devicePixelRatio, alignment, entry, palette, pixmapScale);
}

DGUI_END_NAMESPACE
