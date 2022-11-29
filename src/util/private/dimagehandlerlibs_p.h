// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DIMAGEHANDLERLIBS_P_H
#define DIMAGEHANDLERLIBS_P_H

#include <QHash>
#include <QMutex>
#include <QImage>

#include <FreeImage.h>

class QLibrary;

enum ExifImageOrientation {
    Undefined,
    TopLeft,
    TopRight,
    BottomRight,
    BottomLeft,
    LeftTop,
    RightTop,
    RightBottom,
    LeftBottom,
};

class DLibFreeImage
{
public:
    DLibFreeImage();
    ~DLibFreeImage();

    bool isValid();

    QHash<QString, QString> findMetaData(FREE_IMAGE_MDMODEL model, FIBITMAP *dib);
    QHash<QString, QString> findAllMetaData(const QString &fileName);
    FIBITMAP *readFileToFIBITMAP(const QString &fileName, int flags = 0, FREE_IMAGE_FORMAT fif = FIF_UNKNOWN);
    bool writeFIBITMAPToFile(FIBITMAP *dib, const QString &fileName, int flags = 0);
    QImage FIBITMAPToQImage(FIBITMAP *dib) const;

    ExifImageOrientation imageOrientation(const QString &fileName);
    bool rotateImageFile(const QString &fileName, int angle, QString &errorString);

    FIBITMAP *(*FreeImage_Load)(FREE_IMAGE_FORMAT fif, const char *filename, int flags);
    void (*FreeImage_Unload)(FIBITMAP *dib);
    bool (*FreeImage_Save)(FREE_IMAGE_FORMAT fif, FIBITMAP *dib, const char *filename, int flags);
    bool (*FreeImage_FIFSupportsReading)(FREE_IMAGE_FORMAT fif);
    FREE_IMAGE_FORMAT (*FreeImage_GetFileType)(const char *filename, int size);
    FREE_IMAGE_FORMAT (*FreeImage_GetFIFFromFilename)(const char *filename);
    FREE_IMAGE_TYPE (*FreeImage_GetImageType)(FIBITMAP *dib);

    unsigned (*FreeImage_GetBPP)(FIBITMAP *dib);
    unsigned (*FreeImage_GetWidth)(FIBITMAP *dib);
    unsigned (*FreeImage_GetHeight)(FIBITMAP *dib);
    unsigned (*FreeImage_GetRedMask)(FIBITMAP *dib);
    unsigned (*FreeImage_GetGreenMask)(FIBITMAP *dib);
    unsigned (*FreeImage_GetBlueMask)(FIBITMAP *dib);

    FIBITMAP *(*FreeImage_GetThumbnail)(FIBITMAP *dib);
    bool (*FreeImage_SetThumbnail)(FIBITMAP *dib, FIBITMAP *thumbnail);
    void (*FreeImage_ConvertToRawBits)(uint8_t *bits,
                                       FIBITMAP *dib,
                                       int pitch,
                                       unsigned bpp,
                                       unsigned red_mask,
                                       unsigned green_mask,
                                       unsigned blue_mask,
                                       bool topdown);

    unsigned (*FreeImage_GetMetadataCount)(FREE_IMAGE_MDMODEL model, FIBITMAP *dib);
    FIMETADATA *(*FreeImage_FindFirstMetadata)(FREE_IMAGE_MDMODEL model, FIBITMAP *dib, FITAG **tag);
    bool (*FreeImage_FindNextMetadata)(FIMETADATA *mdhandle, FITAG **tag);
    void (*FreeImage_FindCloseMetadata)(FIMETADATA *mdhandle);

    const char *(*FreeImage_GetTagKey)(FITAG *tag);
    const void *(*FreeImage_GetTagValue)(FITAG *tag);
    const char *(*FreeImage_TagToString)(FREE_IMAGE_MDMODEL model, FITAG *tag, char *Make);

    FIBITMAP *(*FreeImage_Rotate)(FIBITMAP *dib, double angle, const void *bkcolor);

private:
    QLibrary *freeImage = nullptr;
    QMutex apiMutex;

    Q_DISABLE_COPY(DLibFreeImage)
};

#endif  // DIMAGEHANDLERLIBS_P_H
