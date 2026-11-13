// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TREELANDBACKGROUNDEFFECTCLIENTEXTENSION_H
#define TREELANDBACKGROUNDEFFECTCLIENTEXTENSION_H

#include "qwayland-ext-background-effect-v1.h"

#include <dtkgui_global.h>
#include <QRegion>
#include <private/qwaylanddisplay_p.h>

#include <QtWaylandClient/QWaylandClientExtension>
#include <QWaylandClientExtensionTemplate>

DGUI_BEGIN_NAMESPACE

class BackgroundEffectManager : public QWaylandClientExtensionTemplate<BackgroundEffectManager>,
                               public QtWayland::ext_background_effect_manager_v1
{
    Q_OBJECT
public:
    static BackgroundEffectManager *instance();

    [[nodiscard]] inline bool isSupported() const { return m_isSupported; }
    [[nodiscard]] inline bool hasBlurCapability() const { return m_hasBlurCapability; }

Q_SIGNALS:
    void blurCapabilityChanged();

private:
    void addListener();
    void removeListener();

    explicit BackgroundEffectManager();
    static void handleListenerGlobal(void *data, wl_registry *registry, uint32_t id, const QString &interface, uint32_t version);

protected:
    void ext_background_effect_manager_v1_capabilities(uint32_t flags) override;

private:
    QtWaylandClient::QWaylandDisplay *m_waylandDisplay = nullptr;
    bool m_isSupported;
    bool m_hasBlurCapability;
};

class BackgroundEffectSurface : public QWaylandClientExtensionTemplate<BackgroundEffectSurface>,
                               public QtWayland::ext_background_effect_surface_v1
{
    Q_OBJECT
public:
    explicit BackgroundEffectSurface(struct ::ext_background_effect_surface_v1 *surface);
    ~BackgroundEffectSurface() override;

    void setBlurRegion(const QRegion &region, QtWaylandClient::QWaylandDisplay *display);
};

DGUI_END_NAMESPACE

#endif // TREELANDBACKGROUNDEFFECTCLIENTEXTENSION_H
