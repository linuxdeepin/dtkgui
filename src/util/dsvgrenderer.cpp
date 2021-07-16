/*
 * Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <librsvg/rsvg.h>

#include "dsvgrenderer.h"
#include "dobject_p.h"

#include <QPainter>
#include <QFile>
#include <QDebug>
#include <QGuiApplication>
#include <QLibrary>

DCORE_USE_NAMESPACE

DGUI_BEGIN_NAMESPACE

class RSvg
{
public:
    RSvg()
    {
        // fix found lib was librsvg2-dev without version number
        rsvg = new QLibrary("rsvg-2", "2");

        if (!rsvg->load()) {
            delete rsvg;
            rsvg = nullptr;
            return;
        }

#define INIT_FUNCTION(Name) Name = reinterpret_cast<decltype (Name)>(rsvg->resolve(#Name)); Q_ASSERT(Name)

        INIT_FUNCTION(cairo_image_surface_create_for_data);
        INIT_FUNCTION(cairo_create);
        INIT_FUNCTION(cairo_scale);
        INIT_FUNCTION(cairo_translate);
        INIT_FUNCTION(cairo_destroy);
        INIT_FUNCTION(cairo_surface_destroy);
        INIT_FUNCTION(g_object_unref);
        INIT_FUNCTION(rsvg_handle_render_cairo);
        INIT_FUNCTION(rsvg_handle_render_cairo_sub);
        INIT_FUNCTION(rsvg_handle_get_dimensions_sub);
        INIT_FUNCTION(rsvg_handle_get_position_sub);
        INIT_FUNCTION(rsvg_handle_has_sub);
        INIT_FUNCTION(rsvg_handle_new_from_data);
        INIT_FUNCTION(rsvg_handle_get_dimensions);
    }

    static RSvg *instance() {
        static RSvg *global = new RSvg();
        return global;
    }

    bool isValid() const
    {
        return rsvg;
    }

    ~RSvg()
    {
        if (rsvg)
            delete rsvg;
    }

    cairo_surface_t *(*cairo_image_surface_create_for_data)(unsigned char *data, cairo_format_t format, int width, int height, int stride);
    cairo_t *(*cairo_create)(cairo_surface_t *target);
    void (*cairo_scale)(cairo_t *cr, double sx, double sy);
    void (*cairo_translate)(cairo_t *cr, double tx, double ty);
    void (*cairo_destroy)(cairo_t *cr);
    void (*cairo_surface_destroy)(cairo_surface_t *surface);
    void (*g_object_unref)(gpointer object);

    gboolean (*rsvg_handle_render_cairo)(RsvgHandle *handle, cairo_t *cr);
    gboolean (*rsvg_handle_render_cairo_sub)(RsvgHandle *handle, cairo_t *cr, const char *id);
    gboolean (*rsvg_handle_get_dimensions_sub)(RsvgHandle *handle, RsvgDimensionData *dimension_data, const char *id);
    gboolean (*rsvg_handle_get_position_sub)(RsvgHandle *handle, RsvgPositionData *position_data, const char *id);
    gboolean (*rsvg_handle_has_sub)(RsvgHandle *handle, const char *id);
    RsvgHandle *(*rsvg_handle_new_from_data)(const guint8 *data, gsize data_len, GError **error);
    void (*rsvg_handle_get_dimensions)(RsvgHandle *handle, RsvgDimensionData *dimension_data);

private:
    QLibrary *rsvg = nullptr;
};

class DSvgRendererPrivate : public DObjectPrivate
{
public:
    explicit DSvgRendererPrivate(DObject *qq);

    QImage getImage(const QSize &size, const QString &elementId) const;

    RsvgHandle *handle = nullptr;
    QSize defaultSize;

    mutable QRectF viewBox;
};

DSvgRendererPrivate::DSvgRendererPrivate(DObject *qq)
    : DObjectPrivate(qq)
{

}

QImage DSvgRendererPrivate::getImage(const QSize &size, const QString &elementId) const
{
    if (!RSvg::instance()->isValid())
        return QImage();

    QImage image(size, QImage::Format_ARGB32_Premultiplied);

    image.fill(Qt::transparent);

    cairo_surface_t *surface = RSvg::instance()->cairo_image_surface_create_for_data(image.bits(), CAIRO_FORMAT_ARGB32, image.width(), image.height(), image.bytesPerLine());
    cairo_t *cairo = RSvg::instance()->cairo_create(surface);
    RSvg::instance()->cairo_scale(cairo, image.width() / viewBox.width(), image.height() / viewBox.height());
    RSvg::instance()->cairo_translate(cairo, -viewBox.x(), -viewBox.y());

    if (elementId.isEmpty())
        RSvg::instance()->rsvg_handle_render_cairo(handle, cairo);
    else
        RSvg::instance()->rsvg_handle_render_cairo_sub(handle, cairo, elementId.toUtf8().constData());

    RSvg::instance()->cairo_destroy(cairo);
    RSvg::instance()->cairo_surface_destroy(surface);

    return image;
}

/*!
  \class Dtk::Gui::DSvgRenderer
  \inmodule dtkgui
  \brief 提供了将SVG文件的内容绘制到绘制设备上的方法.

  SVG图形可以在构造 DSvgRenderer 时加载，也可以稍后使用load（）函数加载。
  因为渲染是使用 QPainter 执行的，所以可以在 QPaintDevice 的任何子类上渲染SVG图形。
  如果加载了有效文件，则无论是在构造时还是以后某个时间，isValid（）都将返回true；否则将返回false。
  DSvgRenderer提供render（）插槽，用于使用给定的 QPainter 渲染当前文档或动画文档的当前帧
  \note 使用 DSvgRenderer 需要 librsvg库
 */

DSvgRenderer::DSvgRenderer(QObject *parent)
    : QObject(parent)
    , DObject(*new DSvgRendererPrivate(this))
{

}

DSvgRenderer::DSvgRenderer(const QString &filename, QObject *parent)
    : DSvgRenderer(parent)
{
    load(filename);
}

DSvgRenderer::DSvgRenderer(const QByteArray &contents, QObject *parent)
    : DSvgRenderer(parent)
{
    load(contents);
}

DSvgRenderer::~DSvgRenderer()
{
    D_D(DSvgRenderer);

    if (d->handle) {
        Q_ASSERT(RSvg::instance()->isValid());
        RSvg::instance()->g_object_unref(d->handle);
    }
}

bool DSvgRenderer::isValid() const
{
    D_DC(DSvgRenderer);

    return d->handle;
}

QSize DSvgRenderer::defaultSize() const
{
    D_DC(DSvgRenderer);

    return d->defaultSize;
}

QRect DSvgRenderer::viewBox() const
{
    D_DC(DSvgRenderer);

    return d->handle ? d->viewBox.toRect() : QRect();
}

QRectF DSvgRenderer::viewBoxF() const
{
    D_DC(DSvgRenderer);

    return d->handle ? d->viewBox : QRectF();
}

void DSvgRenderer::setViewBox(const QRect &viewbox)
{
    setViewBox(QRectF(viewbox));
}

void DSvgRenderer::setViewBox(const QRectF &viewbox)
{
    D_D(DSvgRenderer);

    if (d->handle)
        d->viewBox = viewbox;
}

QRectF DSvgRenderer::boundsOnElement(const QString &id) const
{
    D_DC(DSvgRenderer);

    if (!d->handle)
        return QRectF();

    const QByteArray &id_data = id.toUtf8();

    RsvgDimensionData dimension_data;

    if (!RSvg::instance()->rsvg_handle_get_dimensions_sub(d->handle, &dimension_data, id_data.constData()))
        return QRectF();

    RsvgPositionData pos_data;

    if (!RSvg::instance()->rsvg_handle_get_position_sub(d->handle, &pos_data, id_data.constData()))
        return QRectF();

    return QRectF(pos_data.x, pos_data.y, dimension_data.width, dimension_data.height);
}

bool DSvgRenderer::elementExists(const QString &id) const
{
    D_DC(DSvgRenderer);

    if (!d->handle)
        return false;

    return RSvg::instance()->rsvg_handle_has_sub(d->handle, id.toUtf8().constData());
}

QImage DSvgRenderer::toImage(const QSize sz, const QString &elementId) const
{
    Q_D(const DSvgRenderer);

    return d->getImage(sz, elementId);
}

bool DSvgRenderer::load(const QString &filename)
{
    QFile file(filename);

    if (file.open(QIODevice::ReadOnly)) {
        return load(file.readAll());
    }

    return false;
}

bool DSvgRenderer::load(const QByteArray &contents)
{
    D_D(DSvgRenderer);

    if (!RSvg::instance()->isValid())
        return false;

    if (d->handle) {
        RSvg::instance()->g_object_unref(d->handle);
        d->handle = nullptr;
    }

    GError *error = nullptr;
    d->handle = RSvg::instance()->rsvg_handle_new_from_data((const guint8*)contents.constData(), contents.length(), &error);

    if (error) {
        qWarning("DSvgRenderer::load: %s", error->message);
        g_error_free(error);

        return false;
    }

    RsvgDimensionData rsvg_data;

    RSvg::instance()->rsvg_handle_get_dimensions(d->handle, &rsvg_data);

    d->defaultSize.setWidth(rsvg_data.width);
    d->defaultSize.setHeight(rsvg_data.height);
    d->viewBox = QRectF(QPointF(0, 0), d->defaultSize);

    return true;
}

void DSvgRenderer::render(QPainter *p)
{
    render(p, QString(), QRectF());
}

void DSvgRenderer::render(QPainter *p, const QRectF &bounds)
{
    render(p, QString(), bounds);
}

void DSvgRenderer::render(QPainter *p, const QString &elementId, const QRectF &bounds)
{
    D_D(DSvgRenderer);

    if (!d->handle)
        return;

    p->save();

    const QImage image = d->getImage(QSize(p->device()->width(), p->device()->height()), elementId);

    if (bounds.isEmpty())
        p->drawImage(0, 0, image);
    else
        p->drawImage(bounds, image);

    p->restore();
}

DGUI_END_NAMESPACE
