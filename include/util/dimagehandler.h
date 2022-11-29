// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DIMAGEHANDLER_H
#define DIMAGEHANDLER_H

#include <dtkgui_global.h>
#include <DObject>

#include <QObject>
#include <QImage>

DGUI_BEGIN_NAMESPACE

class DImageHandlerPrivate;
class DImageHandler : public QObject, public DTK_CORE_NAMESPACE::DObject
{
    Q_OBJECT

public:
    explicit DImageHandler(QObject *parent = nullptr);
    ~DImageHandler();

    void setFileName(const QString &fileName);
    QString fileName() const;

    QImage readImage();
    QImage thumbnail(const QSize &size, Qt::AspectRatioMode mode);
    QSize imageSize();
    QHash<QString, QString> findAllMetaData();
    void clearCache();

    bool saveImage(const QString &fileName, const QString &format = QString());
    bool rotateImage(QImage &image, int angle);
    bool rotateImageFile(const QString &fileName, int angle);

    bool isReadable() const;
    bool isWriteable() const;
    bool isRotateable() const;

    QString lastError() const;

    static QStringList supportFormats();
    static QString detectImageFormat(const QString &fileName);

private:
    D_DECLARE_PRIVATE(DImageHandler)
    Q_DISABLE_COPY(DImageHandler)
};

DGUI_END_NAMESPACE

#endif  // DIMAGEHANDLER_H
