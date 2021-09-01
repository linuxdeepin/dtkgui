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

#include <DObjectPrivate>
#include <DDciFile>

#include <private/qguiapplication_p.h>

#include <QDir>
#include <QPainter>
#include <QBuffer>
#include <QImageReader>

DCORE_USE_NAMESPACE
DGUI_BEGIN_NAMESPACE

#define TYPE_TEXT "text"
#define TYPE_ACTION "action"
#define TYPE_ICON "icon"

#define MODE_NORMAL "normal"
#define MODE_DISABLED "disabled"
#define MODE_HOVER "hover"
#define MODE_PRESSED "pressed"

#define THEME_LIGHT "light"
#define THEME_DARK "dark"

#define FOREGROUND_FILE_NAME "foreground"
#define BACKGROUND_FILE_NAME "background"

struct IconNode {
    int iconSize;
    QVector<DDciIcon::IconPointer> icons;
};
using IconNodeList = QVector<IconNode>;

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
        std::copy(other.nodeOfType, other.nodeOfType + DDciIcon::TypeCount, nodeOfType);
    }

    ~DDciIconPrivate() { }

    void load();
    static void paint(const DDciIcon::IconPointer &icon, DDciIcon::Mode mode, int iconSize,
                      qreal devicePixelRatio, QPainter *painter, const QRect &rect,
                      Qt::Alignment alignment = Qt::AlignCenter);

    // 每一种类型的数组内的图标按大小排序
    IconNodeList nodeOfType[DDciIcon::TypeCount];
    QSharedPointer<const DDciFile> dciFile;
};

static inline bool toType(const QStringRef &name, DDciIcon::Type *type) {
    if (name == QLatin1String(TYPE_TEXT)) {
        *type = DDciIcon::TextType;
        return true;
    }

    if (name == QLatin1String(TYPE_ACTION)) {
        *type = DDciIcon::ActionType;
        return true;
    }

    if (name == QLatin1String(TYPE_ICON)) {
        *type = DDciIcon::IconType;
        return true;
    }

    return false;
}

static inline QLatin1String toString(const DDciIcon::Type type) {
    if (type == DDciIcon::TextType)
        return QLatin1String(TYPE_TEXT);
    if (type == DDciIcon::ActionType)
        return QLatin1String(TYPE_ACTION);
    if (type == DDciIcon::IconType)
        return QLatin1String(TYPE_ICON);
    return {};
}

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

static inline QLatin1String toString(const DDciIcon::Mode mode) {
    switch (mode) {
    case DDciIcon::Normal:
        return QLatin1String(MODE_NORMAL);
    case DDciIcon::Disabled:
        return QLatin1String(MODE_DISABLED);
    case DDciIcon::Hover:
        return QLatin1String(MODE_HOVER);
    case DDciIcon::Pressed:
        return QLatin1String(MODE_PRESSED);
    default:
        break;
    }

    return {};
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

static inline QLatin1String toString(const DDciIcon::Theme theme) {
    if (theme == DDciIcon::Light)
        return QLatin1String(THEME_LIGHT);
    if (theme == DDciIcon::Dark)
        return QLatin1String(THEME_DARK);

    return {};
}

// DCI 内部目录使用 '/' 作为路径分隔符
static inline QString joinPath(const QString &s1, const QString &s2) {
    return s1 + QLatin1Char('/') + s2;
}

static QImage readImageData(const QByteArray &data, const QByteArray &format, int iconSize)
{
    if (data.isEmpty())
        return QImage();

    QBuffer rawDataBuffer;
    rawDataBuffer.setData(data);

    QImageReader reader(&rawDataBuffer);
    if (!format.isEmpty())
        reader.setFormat(format);

    QImage image;

    if (reader.canRead()) {
        reader.setScaledSize(reader.size().scaled(iconSize, iconSize, Qt::KeepAspectRatio));
        reader.read(&image);
    }

    return image;
}

static DDciIcon::IconPointer loadIcon(const QSharedPointer<const DDciFile> &file, const QString &parentDir,
                                      const QString &imageDir)
{
    // Mode-Theme-Format
    const QVector<QStringRef> &imageProps = imageDir.splitRef(QLatin1Char('.'));
    if (imageProps.count() != 3) // 文件名的格式无效
        return DDciIcon::IconPointer(nullptr);
    DDciIcon::IconPointer icon(new DDciIcon::Icon);

    if (!toMode(imageProps[0], &icon->mode))
        return DDciIcon::IconPointer(nullptr);
    if (!toTheme(imageProps[1], &icon->theme))
        return DDciIcon::IconPointer(nullptr);
    icon->format = imageProps.last().toLatin1();
    // 把缩放值当作数组下标使用，所以要限制它的大小
    // 标准中要求至少要支持3倍缩放
    icon->datas.reserve(3);

    // 加载此目录下图片文件
    bool foundForeground = false;
    const QString &path = joinPath(parentDir, imageDir);
    for (const QString &image : file->list(path, true)) {
        const QVector<QStringRef> &nameAndScale = image.splitRef('@');
        if (nameAndScale.count() != 2)
            continue; // ignore invalid file
        bool ok = false;
        int scale = nameAndScale.last().toInt(&ok);
        if (scale > icon->datas.capacity())
            continue; // too big
        DDciIcon::Icon::Data data;
        if (nameAndScale.first() == QLatin1String(FOREGROUND_FILE_NAME)) {
            foundForeground = true;
            data.foreground = file->dataRef(joinPath(path, image));
        } else if (nameAndScale.first() == QLatin1String(BACKGROUND_FILE_NAME)) {
            data.background = file->dataRef(joinPath(path, image));
        } else {
            continue;
        }
        data.imagePixelRatio = scale;
        icon->datas.append(data);
    }

    if (!foundForeground)
        return DDciIcon::IconPointer(nullptr);

    // 释放提前预支的内存空间
    icon->datas.squeeze();
    return icon;
}

static IconNodeList loadIconListForType(const QSharedPointer<const DDciFile> &dciFile,
                                        const QLatin1String &typeName,
                                        DDciIcon::Type type) {
    const QString baseDir = joinPath(QString(), typeName);
    const QStringList &listOfSize = dciFile->list(baseDir, true);
    IconNodeList list;

    for (const QString &dir : listOfSize) {
        bool ok = false;
        int size = dir.toInt(&ok);
        if (!ok) { // 忽略无法识别大小的目录
            continue;
        }

        IconNode node;
        node.iconSize = size;

        // 处理目录内的图片文件
        const QString &dirPath = joinPath(baseDir, dir);
        // 记录已经找到的 background 图片
        for (const QString &imageDir : dciFile->list(dirPath, true)) {
            auto icon = loadIcon(dciFile, dirPath, imageDir);
            if (!icon)
                continue;

            icon->iconSize = size;
            icon->type = type;
            // 根据 DSG 的标准要求，数字文件名是按数字大小排序，因此即便直接把 IconNode
            // 加到末尾即可保障列表内容是有序的
            node.icons << icon;
        }

        list << std::move(node);
    }

    return list;
}

void DDciIconPrivate::load()
{
    if (!dciFile->isValid())
        return;

    nodeOfType[DDciIcon::TextType] = loadIconListForType(dciFile, QLatin1String(TYPE_TEXT),
                                                         DDciIcon::TextType);
    nodeOfType[DDciIcon::ActionType] = loadIconListForType(dciFile, QLatin1String(TYPE_ACTION),
                                                           DDciIcon::ActionType);
    nodeOfType[DDciIcon::IconType] = loadIconListForType(dciFile, QLatin1String(TYPE_ICON),
                                                         DDciIcon::IconType);
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

void DDciIconPrivate::paint(const DDciIcon::IconPointer &icon, DDciIcon::Mode mode, int iconSize,
                            qreal devicePixelRatio, QPainter *painter, const QRect &rect, Qt::Alignment alignment)
{
    // 目前与 DBuildinIconEngine 绘制类似.
    qreal pixelRatio = devicePixelRatio;
    if (pixelRatio <= 0 && painter->device())
        pixelRatio = painter->device()->devicePixelRatioF();

    typedef DDciIcon::Icon::Data DDciIconRawData;
    auto pixelRatioCompare =  [](const DDciIconRawData &d1, const DDciIconRawData &d2) {
        return d1.imagePixelRatio < d2.imagePixelRatio;
    };

    DDciIconRawData targetData;
    targetData.imagePixelRatio = qRound(pixelRatio);

    auto closestData = std::lower_bound(icon->datas.begin(), icon->datas.end(),
                                        targetData, pixelRatioCompare);

    // 当前设备像素比超过数据的最大像素比，则选用最大的那个像素比元素
    if (closestData == icon->datas.end())
        closestData = std::max_element(icon->datas.begin(), icon->datas.end(), pixelRatioCompare);

    // 数据为空则数据失效
    Q_ASSERT(closestData != icon->datas.end());
    targetData = *closestData;

    const QRect iconRect = alignedRect(painter->layoutDirection(), alignment, QSize(iconSize, iconSize), rect);

    const QImage &background = readImageData(targetData.background, icon->format, iconSize);
    QImage foreground = readImageData(targetData.foreground, icon->format, iconSize);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    if (!background.isNull())
        painter->drawImage(iconRect, background);

    if (icon->type == DDciIcon::TextType || (icon->type == DDciIcon::ActionType && mode != DDciIcon::Normal)) {
        QPainter tmpPainter(&foreground);
        tmpPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        tmpPainter.fillRect(foreground.rect(), painter->pen().brush());
    }

    // TODO: Handle the target mode is different from the icon mode.
    painter->drawImage(iconRect, foreground);
    painter->restore();
}

static int findIconNodeByLowerBoundSize(const IconNodeList &list, const int size)
{
    const auto compFun = [] (const IconNode &n1, const IconNode &n2) {
        return n1.iconSize < n2.iconSize;
    };
    IconNode forTarget;
    forTarget.iconSize = size;
    auto neighbor = std::lower_bound(list.cbegin(), list.cend(), forTarget, compFun);

    if (neighbor != list.cend())
        return neighbor - list.constBegin();

    return -1;
}

DDciIcon::DDciIcon(Type ct)
    : d(new DDciIconPrivate())
    , currentIconType(ct)
{

}

DDciIcon::DDciIcon(const DDciFile *dciFile, Type ct)
    : DDciIcon(ct)
{
    d->dciFile.reset(dciFile);
    d->load();
}

DDciIcon::DDciIcon(const QString &fileName, Type ct)
    : DDciIcon(ct)
{
    d->dciFile.reset(new DDciFile(fileName));
    d->load();
}

DDciIcon::DDciIcon(const QByteArray &data, Type ct)
    : DDciIcon(ct)
{
    d->dciFile.reset(new DDciFile(data));
    d->load();
}

DDciIcon::DDciIcon(const DDciIcon &other)
    : d(other.d)
    , currentIconType(other.currentIconType)
{ }

DDciIcon &DDciIcon::operator=(const DDciIcon &other) noexcept
{
    d = other.d;
    currentIconType = other.currentIconType;
    return *this;
}

DDciIcon::~DDciIcon() {}

DDciIcon::DDciIcon(DDciIcon &&other) noexcept
    : d(other.d)
    , currentIconType(other.currentIconType)
{ other.d = nullptr; }

DDciIcon &DDciIcon::operator=(DDciIcon &&other) noexcept
{ swap(other); return *this; }

void DDciIcon::setCurrentType(DDciIcon::Type type)
{
    currentIconType = type;
}

DDciIcon::Type DDciIcon::currentType() const
{
    return currentIconType;
}

bool DDciIcon::isNull(DDciIcon::Type type) const
{
    if (type == DDciIcon::CurrentType)
        type = this->currentIconType;

    IconMatcher matcher = [type](const IconPointer &icon) -> bool {
        return (icon->type == type)
                && (icon->mode == DDciIcon::Normal)
                && (!icon->datas.isEmpty());
    };

    // 如果能找到一个normal状态下的图标，则表示该图标非空
    return !findIcon(matcher);
}

DDciIcon::IconPointer DDciIcon::findIcon(DDciIcon::IconMatcher matcher) const
{
    for (int i = 0; i < TypeCount; ++i) {
        const auto &iconList = d->nodeOfType[i];
        for (const auto &node : iconList) {
            for (const auto &icon : node.icons) {
                if (matcher(icon))
                    return icon;
            }
        }
    }

    return IconPointer(nullptr);
}

DDciIcon::IconPointer DDciIcon::findIcon(int iconSize, Theme theme,
                                         Mode mode, Type type, const QByteArray &requestFormat,
                                         MatchOptions options) const
{
    // 图标类型必须要匹配
    if (type >= TypeCount)
        return IconPointer(nullptr);

    if (type == CurrentType)
        type = currentIconType;

    const IconNodeList &listOfType = d->nodeOfType[type];
    if (listOfType.isEmpty())
        return IconPointer(nullptr);

    // 优先找到第一个比自己大的图标
    auto neighborIndex = findIconNodeByLowerBoundSize(listOfType, iconSize);

    if (neighborIndex < 0) { // 找不到则 fallback 最大的图标
        neighborIndex = listOfType.size() - 1;
    }

    const auto &listOfSize = listOfType.at(neighborIndex);
    // 用于记录列表中的 icon 在本次查找的条件下的权重
    QVector<qint8> iconWeight;
    iconWeight.resize(listOfSize.icons.size());
    // 优先全匹配所有的条件
    for (int i = 0; i < listOfSize.icons.size(); ++i) {
        qint8 weight = 0; // 初始化权重
        const IconPointer &icon = listOfSize.icons.at(i);

        /*
         * 当满足不同的匹配结果时，加权值根据此项匹配条件的优先级而定：
         * requestMode: 3
         * theme: 2
         * requestFormat: 1
         * 所有的 fallback 匹配：0
         * 不可接受的匹配：赋值 -1
         */

        if (icon->mode == mode) {
            weight += 3; // 确保 Mode 的匹配最优先
        } else if ((options & DDciIcon::ModeSensitive)
                   || icon->mode != Normal) {
            weight = -1;
            // 不可接受
            continue;
        }

        if (icon->theme == theme) {
            weight += 2;
        } else {
            weight = -1;
            // 不可接受
            continue;
        }

        if (icon->format == requestFormat) {
            weight += 1;
        } else if (options & FormatSensitive) {
            weight = -1;
            // 不可接受
            continue;
        }
        iconWeight.insert(i, weight);
    }

    const auto targetIcon = std::max_element(iconWeight.constBegin(), iconWeight.constEnd());
    Q_ASSERT(targetIcon);

    return listOfSize.icons.at(targetIcon - iconWeight.constBegin());
}

QPixmap DDciIcon::generatePixmap(const DDciIcon::IconPointer &icon, Mode mode, int iconSize, qreal devicePixelRatio, const QBrush &foreground)
{
    Q_CHECK_PTR(icon);
    QPixmap pixmap(iconSize, iconSize);
    pixmap.fill(Qt::transparent);
    pixmap.setDevicePixelRatio(devicePixelRatio);

    QPainter painter(&pixmap);
    painter.setPen(QPen(foreground, 1));
    painter.setBrush(Qt::NoBrush);

    DDciIcon::paint(icon, mode, iconSize, devicePixelRatio, &painter, pixmap.rect(), Qt::AlignCenter);
    return pixmap;
}

void DDciIcon::paint(const IconPointer &icon, Mode mode, int iconSize, qreal devicePixelRatio, QPainter *painter,
                     const QRect &rect, Qt::Alignment alignment)
{
    Q_CHECK_PTR(icon);
    DDciIconPrivate::paint(icon, mode, iconSize, devicePixelRatio,
                           painter, rect, alignment);
}

DGUI_END_NAMESPACE
