// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "private/dimagehandlerlibs_p.h"

#include <QLibrary>
#include <QFileInfo>
#include <QDateTime>
#include <QImageReader>

DLibFreeImage::DLibFreeImage()
{
    freeImage = new QLibrary("freeimage", "3");

    if (!freeImage->load()) {
        delete freeImage;
        freeImage = nullptr;
        return;
    }

    auto initFunctionError = [this]() {
        freeImage->unload();
        delete freeImage;
        freeImage = nullptr;
    };

#define INIT_FUNCTION(Name)                                                                                                      \
    Name = reinterpret_cast<decltype(Name)>(freeImage->resolve(#Name));                                                          \
    if (!Name) {                                                                                                                 \
        initFunctionError();                                                                                                     \
        return;                                                                                                                  \
    }

    INIT_FUNCTION(FreeImage_Load);
    INIT_FUNCTION(FreeImage_Unload);
    INIT_FUNCTION(FreeImage_Save);
    INIT_FUNCTION(FreeImage_FIFSupportsReading);
    INIT_FUNCTION(FreeImage_GetFileType);
    INIT_FUNCTION(FreeImage_GetFIFFromFilename);
    INIT_FUNCTION(FreeImage_GetImageType);

    INIT_FUNCTION(FreeImage_GetBPP);
    INIT_FUNCTION(FreeImage_GetWidth);
    INIT_FUNCTION(FreeImage_GetHeight);
    INIT_FUNCTION(FreeImage_GetRedMask);
    INIT_FUNCTION(FreeImage_GetGreenMask);
    INIT_FUNCTION(FreeImage_GetBlueMask);

    INIT_FUNCTION(FreeImage_GetThumbnail);
    INIT_FUNCTION(FreeImage_SetThumbnail);
    INIT_FUNCTION(FreeImage_ConvertToRawBits);

    INIT_FUNCTION(FreeImage_GetMetadataCount);
    INIT_FUNCTION(FreeImage_FindFirstMetadata);
    INIT_FUNCTION(FreeImage_FindNextMetadata);
    INIT_FUNCTION(FreeImage_FindCloseMetadata);

    INIT_FUNCTION(FreeImage_GetTagKey);
    INIT_FUNCTION(FreeImage_GetTagValue);
    INIT_FUNCTION(FreeImage_TagToString);
    INIT_FUNCTION(FreeImage_Rotate);
}

DLibFreeImage::~DLibFreeImage()
{
    if (freeImage) {
        delete freeImage;
    }
}

bool DLibFreeImage::isValid()
{
    return freeImage;
}

QHash<QString, QString> DLibFreeImage::findMetaData(FREE_IMAGE_MDMODEL model, FIBITMAP *dib)
{
    QHash<QString, QString> data;
    if (freeImage) {
        if (FreeImage_GetMetadataCount(model, dib)) {
            FITAG *tag = nullptr;
            FIMETADATA *mdhandle = nullptr;
            mdhandle = FreeImage_FindFirstMetadata(model, dib, &tag);
            if (mdhandle) {
                do {
                    QString value;

                    // Warning: FreeImage_TagToString is not thread-safe, add mutex protection.
                    apiMutex.lock();
                    value = QString(FreeImage_TagToString(model, tag, nullptr));
                    apiMutex.unlock();

                    data.insert(FreeImage_GetTagKey(tag), value);
                } while (FreeImage_FindNextMetadata(mdhandle, &tag));
                FreeImage_FindCloseMetadata(mdhandle);
            }
        }
    }

    return data;
}

QHash<QString, QString> DLibFreeImage::findAllMetaData(const QString &fileName)
{
    FIBITMAP *dib = readFileToFIBITMAP(fileName, FIF_LOAD_NOPIXELS);
    QHash<QString, QString> admMap;
    admMap.unite(findMetaData(FIMD_EXIF_MAIN, dib));
    admMap.unite(findMetaData(FIMD_EXIF_EXIF, dib));
    admMap.unite(findMetaData(FIMD_EXIF_GPS, dib));
    admMap.unite(findMetaData(FIMD_EXIF_MAKERNOTE, dib));
    admMap.unite(findMetaData(FIMD_EXIF_INTEROP, dib));
    admMap.unite(findMetaData(FIMD_IPTC, dib));

    QFileInfo info(fileName);
    if (admMap.contains("DateTime")) {
        QDateTime time = QDateTime::fromString(admMap["DateTime"], "yyyy:MM:dd hh:mm:ss");
        admMap.insert("DateTimeOriginal", time.toString("yyyy/MM/dd hh:mm"));
    } else {
        admMap.insert("DateTimeOriginal", info.lastModified().toString("yyyy/MM/dd HH:mm"));
    }
    admMap.insert("DateTimeDigitized", info.lastModified().toString("yyyy/MM/dd HH:mm"));

    // The value of width and height might incorrect
    QImageReader reader(fileName);
    int w = reader.size().width();
    w = w > 0 ? w : static_cast<int>(FreeImage_GetWidth(dib));
    int h = reader.size().height();
    h = h > 0 ? h : static_cast<int>(FreeImage_GetHeight(dib));
    admMap.insert("Dimension", QString::number(w) + "x" + QString::number(h));
    admMap.insert("FileName", info.fileName());
    admMap.insert("FileFormat", info.suffix());

    static auto formatDataSize = [](int size) -> QString {
        static const QStringList suffix = {"B", "KB", "MB", "GB", "TB", "PB"};
        int level = 0;
        double bytes = size;
        while (bytes >= 1024 && level < (suffix.length() - 1)) {
            bytes /= 1024;
            ++level;
        }
        return QString("%1%2").arg(bytes, 0, 'f', 1).arg(suffix[level]);
    };
    admMap.insert("FileSize", formatDataSize(info.size()));

    FreeImage_Unload(dib);
    return admMap;
}

FIBITMAP *DLibFreeImage::readFileToFIBITMAP(const QString &fileName, int flags, FREE_IMAGE_FORMAT fif)
{
    QByteArray b = fileName.toUtf8();
    const char *pc = b.data();

    // Detect file format.
    if (FIF_UNKNOWN == fif) {
        fif = FreeImage_GetFileType(pc, 0);
        if (FIF_UNKNOWN == fif) {
            fif = FreeImage_GetFIFFromFilename(pc);
        }
    }
    
    if ((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif)) {
        FIBITMAP *dib = FreeImage_Load(fif, pc, flags);
        return dib;
    }
    return nullptr;
}

bool DLibFreeImage::writeFIBITMAPToFile(FIBITMAP *dib, const QString &fileName, int flags)
{
    FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
    bool bSuccess = false;
    const QByteArray ba = fileName.toUtf8();
    const char *pc = ba.data();
    // Try to guess the file format from the file extension
    fif = FreeImage_GetFIFFromFilename(pc);
    FREE_IMAGE_FORMAT realfif = FIF_UNKNOWN;

    if (fif == FIF_UNKNOWN) {
        realfif = FreeImage_GetFileType(pc, 0);
    }

    if (fif != FIF_UNKNOWN) {
        bSuccess = FreeImage_Save(fif, dib, pc, flags);
    } else if (realfif != FIF_UNKNOWN) {
        bSuccess = FreeImage_Save(realfif, dib, pc, flags);
    }

    return bSuccess;
}

QImage DLibFreeImage::FIBITMAPToQImage(FIBITMAP *dib) const
{
    if (!dib || FreeImage_GetImageType(dib) == FIT_UNKNOWN)
        return QImage(0, 0, QImage::Format_Invalid);
    int width = static_cast<int>(FreeImage_GetWidth(dib));
    int height = static_cast<int>(FreeImage_GetHeight(dib));
    int depth = static_cast<int>(FreeImage_GetBPP(dib));

    switch (depth) {
        case 1: {
            QImage result(width, height, QImage::Format_Mono);
            FreeImage_ConvertToRawBits(result.scanLine(0), dib, result.bytesPerLine(), 1, 0, 0, 0, true);
            return result;
        }
        case 4: {
            // NOTE: QImage do not support 4-bit, convert it to 8-bit
            QImage result(width, height, QImage::Format_Indexed8);
            FreeImage_ConvertToRawBits(result.scanLine(0), dib, result.bytesPerLine(), 8, 0, 0, 0, true);
            return result;
        }
        case 8: {
            QImage result(width, height, QImage::Format_Indexed8);
            FreeImage_ConvertToRawBits(result.scanLine(0), dib, result.bytesPerLine(), 8, 0, 0, 0, true);
            return result;
        }
        case 16:
            if ((FreeImage_GetRedMask(dib) == FI16_555_RED_MASK) && (FreeImage_GetGreenMask(dib) == FI16_555_GREEN_MASK) &&
                (FreeImage_GetBlueMask(dib) == FI16_555_BLUE_MASK)) {
                // 5-5-5
                QImage result(width, height, QImage::Format_RGB555);
                FreeImage_ConvertToRawBits(result.scanLine(0),
                                           dib,
                                           result.bytesPerLine(),
                                           16,
                                           FI16_555_RED_MASK,
                                           FI16_555_GREEN_MASK,
                                           FI16_555_BLUE_MASK,
                                           true);
                return result;
            } else {
                // 5-6-5
                QImage result(width, height, QImage::Format_RGB16);
                FreeImage_ConvertToRawBits(result.scanLine(0),
                                           dib,
                                           result.bytesPerLine(),
                                           16,
                                           FI16_565_RED_MASK,
                                           FI16_565_GREEN_MASK,
                                           FI16_565_BLUE_MASK,
                                           true);
                return result;
            }
        case 24: {
            QImage result(width, height, QImage::Format_RGB32);
            FreeImage_ConvertToRawBits(result.scanLine(0),
                                       dib,
                                       result.bytesPerLine(),
                                       32,
                                       FI_RGBA_RED_MASK,
                                       FI_RGBA_GREEN_MASK,
                                       FI_RGBA_BLUE_MASK,
                                       true);
            return result;
        }
        case 32: {
            QImage result(width, height, QImage::Format_ARGB32);
            FreeImage_ConvertToRawBits(result.scanLine(0),
                                       dib,
                                       result.bytesPerLine(),
                                       32,
                                       FI_RGBA_RED_MASK,
                                       FI_RGBA_GREEN_MASK,
                                       FI_RGBA_BLUE_MASK,
                                       true);
            return result;
        }
        case 48:
        case 64:
        case 96:
        case 128:
        default:
            break;
    }

    return QImage(0, 0, QImage::Format_Invalid);
}

ExifImageOrientation DLibFreeImage::imageOrientation(const QString &fileName)
{
    ExifImageOrientation oreintation = Undefined;

    FIBITMAP *dib = readFileToFIBITMAP(fileName, FIF_LOAD_NOPIXELS);
    // Metadata may be null sometimes, check if metadata exist.
    if (0 == FreeImage_GetMetadataCount(FIMD_EXIF_MAIN, dib)) {
        FreeImage_Unload(dib);
        return oreintation;
    }

    FITAG *tag = nullptr;
    FIMETADATA *mdhandle = nullptr;
    mdhandle = FreeImage_FindFirstMetadata(FIMD_EXIF_MAIN, dib, &tag);
    if (mdhandle) {
        do {
            if (qstrcmp(FreeImage_GetTagKey(tag), "Orientation") == 0) {
                oreintation = ExifImageOrientation(*static_cast<const WORD *>(FreeImage_GetTagValue(tag)));
                break;
            }
        } while (FreeImage_FindNextMetadata(mdhandle, &tag));
        FreeImage_FindCloseMetadata(mdhandle);
    }

    FreeImage_Unload(dib);
    return oreintation;
}

bool DLibFreeImage::rotateImageFile(const QString &fileName, int angle, QString &errorString)
{
    FIBITMAP *dib = readFileToFIBITMAP(fileName);
    if (nullptr == dib) {
        errorString = "Unsupported format";
        return false;
    }

    FIBITMAP *rotateRes = FreeImage_Rotate(dib, -angle, nullptr);
    if (rotateRes) {
        // Regenerate thumbnail if it's exits
        // Image formats that currently support thumbnail saving are
        // JPEG (JFIF formats), EXR, TGA and TIFF.
        if (FreeImage_GetThumbnail(dib)) {
            FIBITMAP *thumb = FreeImage_GetThumbnail(dib);
            FIBITMAP *rotateThumb = FreeImage_Rotate(thumb, -angle, nullptr);

            FreeImage_SetThumbnail(rotateRes, rotateThumb);
            FreeImage_Unload(rotateThumb);
        }
    }

    FREE_IMAGE_FORMAT f = FreeImage_GetFIFFromFilename(fileName.toUtf8().data());
    if (f == FIF_UNKNOWN) {
        FreeImage_Unload(dib);
        FreeImage_Unload(rotateRes);
        errorString = "Rotate image format error";
        return false;
    }

    if (!writeFIBITMAPToFile(rotateRes, fileName)) {
        FreeImage_Unload(dib);
        FreeImage_Unload(rotateRes);
        errorString = "Rotate image save failed, unknown format";
        return false;
    }

    FreeImage_Unload(dib);
    FreeImage_Unload(rotateRes);
    return true;
}
