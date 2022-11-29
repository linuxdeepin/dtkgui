// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dimagehandler.h"
#include "private/dimagehandlerlibs_p.h"

#include <DObjectPrivate>

#include <QSet>
#include <QHash>
#include <QFileInfo>
#include <QImageReader>

#define SAVE_QUAITY_VALUE 100

struct SupportFormats
{
    QHash<QString, int> freeImageFormats;
    QHash<QString, int> movieFormats;
    QStringList libRawFormats;
    QStringList qtSupportFormats;
    QStringList saveableFormats;
    QStringList qtRotatableFormats;

    QStringList supportFormats;

    SupportFormats();
};

Q_GLOBAL_STATIC(SupportFormats, SupportFormatsInstance)
Q_GLOBAL_STATIC(DLibFreeImage, DLibFreeImageInstance)

SupportFormats::SupportFormats()
{
    // LibFreeImage support formats
    freeImageFormats["BMP"] = FIF_BMP;
    freeImageFormats["ICO"] = FIF_ICO;
    freeImageFormats["JPG"] = FIF_JPEG;
    freeImageFormats["JPE"] = FIF_JPEG;
    freeImageFormats["JPS"] = FIF_JPEG;
    freeImageFormats["JPEG"] = FIF_JPEG;
    freeImageFormats["JNG"] = FIF_JNG;
    freeImageFormats["KOALA"] = FIF_KOALA;
    freeImageFormats["KOA"] = FIF_KOALA;
    freeImageFormats["LBM"] = FIF_LBM;
    freeImageFormats["IFF"] = FIF_LBM;
    freeImageFormats["MNG"] = FIF_MNG;
    freeImageFormats["PBM"] = FIF_PBM;
    freeImageFormats["PBMRAW"] = FIF_PBMRAW;
    freeImageFormats["PCD"] = FIF_PCD;
    freeImageFormats["PCX"] = FIF_PCX;
    freeImageFormats["PGM"] = FIF_PGM;
    freeImageFormats["PGMRAW"] = FIF_PGMRAW;
    freeImageFormats["PNG"] = FIF_PNG;
    freeImageFormats["PPM"] = FIF_PPM;
    freeImageFormats["PPMRAW"] = FIF_PPMRAW;
    freeImageFormats["RAS"] = FIF_RAS;
    freeImageFormats["TGA"] = FIF_TARGA;
    freeImageFormats["TARGA"] = FIF_TARGA;
    freeImageFormats["TIFF"] = FIF_TIFF;
    freeImageFormats["TIF"] = FIF_TIFF;
    freeImageFormats["WBMP"] = FIF_WBMP;
    freeImageFormats["PSD"] = FIF_PSD;
    freeImageFormats["CUT"] = FIF_CUT;
    freeImageFormats["XBM"] = FIF_XBM;
    freeImageFormats["XPM"] = FIF_XPM;
    freeImageFormats["DDS"] = FIF_DDS;
    freeImageFormats["GIF"] = FIF_GIF;
    // Warning: Build fails in arch linux. 
    // In libfreeimage.h from arch linux freeimage3.18.0-15 package: 
    // The G3 fax format plugin is deliberately disabled in the Fedora build of
    // FreeImage as it requires that FreeImage uses a private copy of libtiff
    // which is a no no because of security reasons.
#if 0
    freeImageFormats["FAX"] = FIF_FAXG3;
    freeImageFormats["G3"] = FIF_FAXG3;
#endif
    freeImageFormats["SGI"] = FIF_SGI;
    freeImageFormats["EXR"] = FIF_EXR;
    freeImageFormats["PCT"] = FIF_PICT;
    freeImageFormats["PIC"] = FIF_PICT;
    freeImageFormats["PICT"] = FIF_PICT;
    freeImageFormats["PIC"] = FIF_PICT;
    freeImageFormats["WEBP"] = FIF_WEBP;
    freeImageFormats["JXR"] = FIF_JXR;
    movieFormats["MNG"] = FIF_MNG;
    movieFormats["GIF"] = FIF_GIF;
    movieFormats["WEBP"] = FIF_WEBP;

    // LibRaw support images
    libRawFormats << "DNG"
                  << "RAF"
                  << "CR2"
                  << "ORF"
                  << "RAW"
                  << "MRW"
                  << "NEF"
                  << "TIF"
                  << "TIFF";

    // For some formats, Qt will support after loading image plugins.
    qtSupportFormats << "BMP"
                     << "JPG"
                     << "JPEG"
                     << "JPS"
                     << "JPE"
                     << "PNG"
                     << "PBM"
                     << "PGM"
                     << "PPM"
                     << "PNM"
                     << "WBMP"
                     << "WEBP"
                     << "SVG"
                     << "ICNS"
                     << "GIF"
                     << "MNG"
                     << "TIF"
                     << "TIFF"
                     << "BMP"
                     << "XPM"
                     << "MEF"
                     << "ICO"
                     << "JP2";

    saveableFormats << "BMP"
                    << "JPG"
                    << "JPEG"
                    << "JPS"
                    << "JPE"
                    << "PNG"
                    << "PGM"
                    << "PPM"
                    << "PNM"
                    << "TGA"
                    << "XPM"
                    << "ICO"
                    << "JNG"
#ifndef ISSW_64
                    << "WBMP"
#endif
                    << "RAS";

    // For those formats, Qt process faster than FreeImage.
    qtRotatableFormats << "ICNS"
                       << "JPG"
                       << "JPEG"
                       << "PNG"
                       << "BMP";

    QSet<QString> formats;
    // Check libFreeImage load succeeded.
    if (DLibFreeImageInstance()->isValid()) {
        for (auto itr = freeImageFormats.begin(); itr != freeImageFormats.end(); ++itr) {
            formats.insert(itr.key());
        }

        for (auto itr = movieFormats.begin(); itr != movieFormats.end(); ++itr) {
            formats.insert(itr.key());
        }
    }

    // TODO: Add libRaw support formats.

    if (formats.isEmpty()) {
        supportFormats = qtSupportFormats;
    } else {
        // Add qt default support formats.
        for (const QString &format : qtSupportFormats) {
            formats.insert(format);
        }

        supportFormats = formats.values();
    }
}

DCORE_USE_NAMESPACE
DGUI_BEGIN_NAMESPACE

QString detectImageFormatInternal(const QString &fileName, FREE_IMAGE_FORMAT &format);

class DImageHandlerPrivate : public DObjectPrivate
{
    D_DECLARE_PUBLIC(DImageHandler)
public:
    enum ImageOption { Readable = 0x1, Wirteable = 0x2, Rotateable = 0x4, SupportFreeImage = 0x8 };
    Q_DECLARE_FLAGS(ImageOptions, ImageOption);

    explicit DImageHandlerPrivate(DImageHandler *qq);

    bool detectFileReadable(const QString &detectFileName) const;
    bool detectFileWriteable(const QString &detectFileName) const;

    bool loadStaticImageFromFile(const QString &fileName, QImage &image);
    bool rotateImageFile(const QString &fileName, int angle);
    bool rotateImage(QImage &image, int angle);

    void adjustImageToRealOrientation(QImage &image, ExifImageOrientation orientation);

    QString fileName;
    ImageOptions options;
    QImage cachedImage;

    QString lastError;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(DImageHandlerPrivate::ImageOptions)

DImageHandlerPrivate::DImageHandlerPrivate(DImageHandler *qq)
    : DObjectPrivate(qq)
{
}

bool DImageHandlerPrivate::detectFileReadable(const QString &detectFileName) const
{
    if (detectFileName.isEmpty()) {
        return false;
    }

    if (DLibFreeImageInstance()->isValid()) {
        FREE_IMAGE_FORMAT format = DLibFreeImageInstance()->FreeImage_GetFIFFromFilename(detectFileName.toUtf8().data());
        if (FIF_UNKNOWN != format) {
            QString formatString = SupportFormatsInstance()->freeImageFormats.key(format);
            if (SupportFormatsInstance()->supportFormats.contains(formatString)) {
                return true;
            }
        }
    }

    QString fileSuffix = QFileInfo(detectFileName).suffix().toUpper();
    return SupportFormatsInstance()->supportFormats.contains(fileSuffix);
}

bool DImageHandlerPrivate::detectFileWriteable(const QString &detectFileName) const
{
    if (detectFileName.isEmpty()) {
        return false;
    }

    if (DLibFreeImageInstance()->isValid()) {
        FREE_IMAGE_FORMAT format = DLibFreeImageInstance()->FreeImage_GetFIFFromFilename(detectFileName.toUtf8().data());
        if (FIF_UNKNOWN != format) {
            QString formatString = SupportFormatsInstance()->freeImageFormats.key(format);
            if (SupportFormatsInstance()->saveableFormats.contains(formatString)) {
                return true;
            }
        }
    }

    QString fileSuffix = QFileInfo(detectFileName).suffix().toUpper();
    return SupportFormatsInstance()->saveableFormats.contains(fileSuffix);
}

bool DImageHandlerPrivate::loadStaticImageFromFile(const QString &fileName, QImage &image)
{
    D_Q(DImageHandler);

    QFileInfo fileInfo(fileName);
    if (0 == fileInfo.size()) {
        lastError = QString("Error file!");
        return false;
    }

    FREE_IMAGE_FORMAT fifFormat = FIF_UNKNOWN;
    QString fileFormat = detectImageFormatInternal(fileName, fifFormat);
    if (DLibFreeImageInstance()->isValid()) {
        // Using free image meta type.
        QHash<QString, QString> dataMap = DLibFreeImageInstance()->findAllMetaData(fileName);
        fileFormat = dataMap.value("FileFormat").toUpper();
    }

    // For some formats, need use Qt image reader load, to aviod some errors on different hardware architectures.
    bool usingQImage = (FIF_RAW == fifFormat) || ((FIF_PICT == fifFormat) && ("PCT" != fileFormat));

    if (usingQImage || SupportFormatsInstance()->qtSupportFormats.contains(fileFormat)) {
        QImageReader reader(fileName);
        reader.setAutoTransform(true);

        if (reader.imageCount() > 0 || "ICNS" != fileFormat) {
            image = reader.read();
            if (!image.isNull()) {
                return true;
            } else {
                // Try loading image with a forced format.
                reader.setFormat(fileFormat.toLower().toUtf8());
                image = reader.read();
                if (!image.isNull()) {
                    return true;
                } else {
                    lastError = QString("Load image by qt failed, use format: %1").arg(fileFormat);
                    return false;
                }
            }
        }
    } else if (DLibFreeImageInstance()->isValid()) {
        if (FIF_UNKNOWN != fifFormat || SupportFormatsInstance()->freeImageFormats.contains(fileFormat)) {
            if (FIF_UNKNOWN == fifFormat) {
                fifFormat = FREE_IMAGE_FORMAT(SupportFormatsInstance()->freeImageFormats.value(fileFormat, FIF_UNKNOWN));
            }

            static const int MAX_JP2_SUPPORT_SIZE = 40960000;
            if (FIF_JP2 == fifFormat && fileInfo.size() > MAX_JP2_SUPPORT_SIZE) {
                lastError = QString("Load image failed, JP2 image size to big, format: %1").arg(fileFormat);
                return false;
            }

            // Using FreeImage load.
            FIBITMAP *dib = DLibFreeImageInstance()->FreeImage_Load(fifFormat, fileName.toUtf8().data(), 0);
            if (!dib) {
                lastError = QString("Load image failed, format: %1").arg(fileFormat);
                return false;
            }

            image = DLibFreeImageInstance()->FIBITMAPToQImage(dib);
            if (image.isNull()) {
                DLibFreeImageInstance()->FreeImage_Unload(dib);
                lastError = QString("Convert to QImage failed: %1").arg(fileFormat);
                return false;
            }

            DLibFreeImageInstance()->FreeImage_Unload(dib);
            return true;
        }
    }

    lastError = QString("Unsupport image format: %1").arg(fileFormat);
    return false;
}

bool DImageHandlerPrivate::rotateImageFile(const QString &fileName, int angle)
{
    D_Q(DImageHandler);

    if (0 != (angle % 90)) {
        lastError = QString("Unsupported angle.");
        return false;
    }

    QString format = q->detectImageFormat(fileName);
    if (SupportFormatsInstance()->qtRotatableFormats.contains(format)) {
        QImage image(fileName);

        // Some image formats support use EXIF info store orientation info.
        if (DLibFreeImageInstance()->isValid()) {
            // Using libFreeImage to get EXIF information.
            ExifImageOrientation orientation = DLibFreeImageInstance()->imageOrientation(fileName);
            adjustImageToRealOrientation(image, orientation);
        }

        if (rotateImage(image, angle)) {
            image.save(fileName, format.toLatin1().data(), SAVE_QUAITY_VALUE);
            return true;
        }
        return false;
    }

    if (DLibFreeImageInstance()->isValid()) {
        return DLibFreeImageInstance()->rotateImageFile(fileName, angle, lastError);
    }

    lastError = QString("Unsupported format: %1").arg(format);
    return false;
}

bool DImageHandlerPrivate::rotateImage(QImage &image, int angle)
{
    if (image.isNull() || 0 == (angle % 90)) {
        lastError = QString("Image is null or rotate angle not base of 90.");
        return false;
    }

    QImage imageCopy(image);
    if (!imageCopy.isNull()) {
        QTransform rotateMatrix;
        rotateMatrix.rotate(angle);
        image = imageCopy.transformed(rotateMatrix, Qt::SmoothTransformation);
        return true;
    }

    lastError = QString("Image is null.");
    return false;
}

void DImageHandlerPrivate::adjustImageToRealOrientation(QImage &image, ExifImageOrientation orientation)
{
    switch (orientation) {
            // TopLeft unnecessary rotate/flip
        case TopRight:  // Horizontal flip
            image = image.mirrored(true, false);
            break;
        case BottomRight:
            rotateImage(image, 180);
            break;
        case BottomLeft:  // Vertical flip
            image = image.mirrored(false, true);
            break;
        case LeftTop:  // Clockwise 90 degrees and horizontal flip
            rotateImage(image, 90);
            image = image.mirrored(true, false);
            break;
        case RightTop:
            rotateImage(image, 90);
            break;
        case RightBottom:  // Clockwise 90 degrees and vertical flip
            rotateImage(image, 90);
            image = image.mirrored(false, true);
            break;
        case LeftBottom:  // Counterclockwise 90 degrees
            rotateImage(image, -90);
            break;
        default:
            break;
    }
}

DImageHandler::DImageHandler(QObject *parent)
    : QObject(parent)
    , DObject(*new DImageHandlerPrivate(this))
{
}

DImageHandler::~DImageHandler() {}

void DImageHandler::setFileName(const QString &fileName)
{
    D_D(DImageHandler);
    d->fileName = fileName;
    d->options &= 0;
    clearCache();

    if (!d->fileName.isEmpty()) {
        d->options.setFlag(DImageHandlerPrivate::Readable, d->detectFileReadable(d->fileName));
        if (d->detectFileWriteable(d->fileName)) {
            d->options.setFlag(DImageHandlerPrivate::Wirteable);
            d->options.setFlag(DImageHandlerPrivate::Rotateable);
        }
    }
}

QString DImageHandler::fileName() const
{
    D_DC(DImageHandler);
    return d->fileName;
}

QImage DImageHandler::readImage()
{
    D_D(DImageHandler);

    if (!isReadable()) {
        return QImage();
    }

    if (!d->cachedImage.isNull()) {
        return d->cachedImage;
    }

    d->loadStaticImageFromFile(d->fileName, d->cachedImage);
    return d->cachedImage;
}

QImage DImageHandler::thumbnail(const QSize &size, Qt::AspectRatioMode mode)
{
    D_D(DImageHandler);
    if (d->cachedImage.isNull()) {
        d->loadStaticImageFromFile(d->fileName, d->cachedImage);
    }

    return d->cachedImage.scaled(size, mode);
}

QSize DImageHandler::imageSize()
{
    D_D(DImageHandler);
    if (isReadable() && d->cachedImage.isNull()) {
        d->loadStaticImageFromFile(d->fileName, d->cachedImage);
    }

    return d->cachedImage.size();
}

QHash<QString, QString> DImageHandler::findAllMetaData()
{
    D_D(DImageHandler);
    if (!isReadable()) {
        return {};
    }

    if (DLibFreeImageInstance()->isValid()) {
        return DLibFreeImageInstance()->findAllMetaData(d->fileName);
    }
    return {};
}

bool DImageHandler::saveImage(const QString &fileName, const QString &format)
{
    D_D(DImageHandler);

    QString realFormat = format.toUpper();
    if (realFormat.isEmpty()) {
        // Detect save file format.
        FREE_IMAGE_FORMAT fifFormat = FIF_UNKNOWN;
        QString fileFormat = detectImageFormatInternal(fileName, fifFormat);
        if (FIF_UNKNOWN != fifFormat) {
            realFormat = SupportFormatsInstance()->freeImageFormats.key(fifFormat);
        }
    }

    if (!SupportFormatsInstance()->saveableFormats.contains(realFormat.toUpper())) {
        d->lastError = QString("Unsupport image save format: %1").arg(realFormat);
        return false;
    }

    if (d->cachedImage.isNull()) {
        if (!d->loadStaticImageFromFile(d->fileName, d->cachedImage)) {
            return false;
        }
    }

    if (!d->cachedImage.save(fileName, realFormat.toUtf8().data(), SAVE_QUAITY_VALUE)) {
        d->lastError = QString("Save image by qt failed, format: %1").arg(realFormat);
        return false;
    }
    return true;
}

bool DImageHandler::rotateImage(QImage &image, int angle)
{
    D_D(DImageHandler);
    return d->rotateImage(image, angle);
}

bool DImageHandler::rotateImageFile(const QString &fileName, int angle)
{
    D_D(DImageHandler);
    return d->rotateImageFile(fileName, angle);
}

bool DImageHandler::isReadable() const
{
    D_DC(DImageHandler);
    return d->options.testFlag(DImageHandlerPrivate::Readable);
}

bool DImageHandler::isWriteable() const
{
    D_DC(DImageHandler);
    return d->options.testFlag(DImageHandlerPrivate::Wirteable);
}

bool DImageHandler::isRotateable() const
{
    D_DC(DImageHandler);
    return d->options.testFlag(DImageHandlerPrivate::Rotateable);
}

void DImageHandler::clearCache()
{
    D_D(DImageHandler);
    d->cachedImage = QImage();
    d->lastError.clear();
}

QString DImageHandler::lastError() const
{
    return d_func()->lastError;
}

QStringList DImageHandler::supportFormats()
{
    return SupportFormatsInstance()->supportFormats;
}

QString detectImageFormatInternal(const QString &fileName, FREE_IMAGE_FORMAT &format)
{
    QFileInfo info(fileName);
    QString fileSuffix = info.suffix().toUpper();
    QByteArray tempPath = fileName.toUtf8();
    format = FIF_UNKNOWN;

    // Check load lib FreeImage succeeded.
    if (DLibFreeImageInstance()->isValid()) {
        format = DLibFreeImageInstance()->FreeImage_GetFileType(tempPath.data(), 0);
        if (FIF_UNKNOWN != format && format != SupportFormatsInstance()->freeImageFormats[fileSuffix]) {
            fileSuffix = SupportFormatsInstance()->freeImageFormats.key(format);
        }

        if (format == FIF_TIFF) {
            fileSuffix = "TIFF";
        }
    }

    if (fileSuffix.isEmpty()) {
        QFile file(fileName);
        if (!file.open(QFile::ReadOnly)) {
            return QString("");
        }

        const QByteArray data = file.read(64);

        // Check bmp file.
        if (data.startsWith("BM")) {
            return "BMP";
        }

        // Check dds file.
        if (data.startsWith("DDS")) {
            return "DDS";
        }

        // Check gif file.
        if (data.startsWith("GIF8")) {
            return "GIF";
        }

        // Check Max OS icons file.
        if (data.startsWith("icns")) {
            return "ICNS";
        }

        // Check jpeg file.
        if (data.startsWith("\xff\xd8")) {
            return "JPG";
        }

        // Check mng file.
        if (data.startsWith("\x8a\x4d\x4e\x47\x0d\x0a\x1a\x0a")) {
            return "MNG";
        }

        // Check net pbm file (BitMap).
        if (data.startsWith("P1") || data.startsWith("P4")) {
            return "PBM";
        }

        // Check pgm file (GrayMap).
        if (data.startsWith("P2") || data.startsWith("P5")) {
            return "PGM";
        }

        // Check ppm file (PixMap).
        if (data.startsWith("P3") || data.startsWith("P6")) {
            return "PPM";
        }

        // Check png file.
        if (data.startsWith("\x89PNG\x0d\x0a\x1a\x0a")) {
            return "PNG";
        }

        // Check svg file.
        if (data.indexOf("<svg") > -1) {
            return "SVG";
        }

        // Check tiff file.
        if (data.startsWith("MM\x00\x2a") || data.startsWith("II\x2a\x00")) {
            // big-endian, little-endian.
            return "TIFF";
        }

        // Check webp file.
        if (data.startsWith("RIFFr\x00\x00\x00WEBPVP")) {
            return "WEBP";
        }

        // Check xbm file.
        if (data.indexOf("#define max_width ") > -1 && data.indexOf("#define max_height ") > -1) {
            return "XBM";
        }

        // Check xpm file.
        if (data.startsWith("/* XPM */")) {
            return "XPM";
        }

        return QString("");
    }

    return fileSuffix;
}

QString DImageHandler::detectImageFormat(const QString &fileName)
{
    FREE_IMAGE_FORMAT format = FIF_UNKNOWN;
    return detectImageFormatInternal(fileName, format);
}

DGUI_END_NAMESPACE
