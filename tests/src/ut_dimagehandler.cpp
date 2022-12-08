// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test.h"
#include "dimagehandler.h"

#include <QLibrary>
#include <QFile>
#include <QUrl>
#include <QPainter>

#include <QDebug>

DGUI_USE_NAMESPACE

class TDImageHandler : public DTest
{
public:
    static void SetUpTestCase();
    static void TearDownTestCase();

    static int tmpImageWidth;
    static int tmpImageHeight;
    static QString tmpFileName;

protected:
    void SetUp();
    void TearDown();

    DImageHandler *handler;
    bool canLoadFreeImage;
};

int TDImageHandler::tmpImageWidth = 300;
int TDImageHandler::tmpImageHeight = 200;
QString TDImageHandler::tmpFileName = QString("/tmp/TDImageHandler_shared_test.png");

void TDImageHandler::SetUpTestCase()
{
    QImage image(tmpImageWidth, tmpImageHeight, QImage::Format_ARGB32);
    image.fill(Qt::red);
    image.save(tmpFileName);
}

void TDImageHandler::TearDownTestCase()
{
    if (QFile::exists(tmpFileName)) {
        QFile::remove(tmpFileName);
    }
}

void TDImageHandler::SetUp()
{
    QLibrary freeImage("freeimage", "3");

    if (!freeImage.isLoaded()) {
        canLoadFreeImage = freeImage.load();
        if (canLoadFreeImage) {
            freeImage.unload();
        }
    }

    handler = new DImageHandler;
}

void TDImageHandler::TearDown()
{
    delete handler;
}

TEST_F(TDImageHandler, testSetFileName)
{
    handler->setFileName(tmpFileName);
    ASSERT_EQ(tmpFileName, handler->fileName());
}

TEST_F(TDImageHandler, testReadImage)
{
    handler->setFileName(tmpFileName);
    QImage image = handler->readImage();
    ASSERT_EQ(QSize(tmpImageWidth, tmpImageHeight), image.size());
}

TEST_F(TDImageHandler, testThumbnail)
{
    handler->setFileName(tmpFileName);
    QImage image = handler->thumbnail(QSize(20, 20), Qt::IgnoreAspectRatio);
    ASSERT_EQ(QSize(20, 20), image.size());
}

TEST_F(TDImageHandler, testImageSize)
{
    ASSERT_EQ(QSize(0, 0), handler->imageSize());

    handler->setFileName(tmpFileName);
    ASSERT_EQ(QSize(tmpImageWidth, tmpImageHeight), handler->imageSize());
}

TEST_F(TDImageHandler, testImageSizeWithSVG)
{
    QByteArray svgCode("<svg version=\"1.1\"> <rect width=\"300\" height=\"300\" /> </svg>");
    QString tmpFilePath("/tmp/TDImageHandler_testImageSizeWithSVG.svg");
    QFile tmpFile(tmpFilePath);
    ASSERT_TRUE(tmpFile.open(QFile::WriteOnly));
    tmpFile.write(svgCode);
    tmpFile.close();

    handler->setFileName(tmpFilePath);
    EXPECT_EQ(QSize(300, 300), handler->imageSize());

    tmpFile.remove();
}

TEST_F(TDImageHandler, testFindAllMetaData)
{
    handler->setFileName(tmpFileName);
    if (canLoadFreeImage) {
        ASSERT_FALSE(handler->findAllMetaData().isEmpty());
    } else {
        ASSERT_TRUE(handler->findAllMetaData().isEmpty());
    }
}

TEST_F(TDImageHandler, testClearCache)
{
    ASSERT_TRUE(handler->readImage().isNull());
    ASSERT_FALSE(handler->lastError().isEmpty());
    handler->clearCache();

    ASSERT_TRUE(handler->lastError().isEmpty());
}

TEST_F(TDImageHandler, testSaveImage)
{
    QString tmpSaveFileName("/tmp/TDImageHandler_testSaveImage.jpg");
    handler->setFileName(tmpFileName);
    ASSERT_TRUE(handler->saveImage(tmpSaveFileName, "JPG"));
    ASSERT_TRUE(QFile::exists(tmpSaveFileName));

    QFile::remove(tmpSaveFileName);
}

TEST_F(TDImageHandler, testRotateImage)
{
    QImage image(300, 200, QImage::Format_ARGB32);
    image.fill(Qt::red);
    ASSERT_TRUE(handler->rotateImage(image, 90));
    ASSERT_EQ(QSize(200, 300), image.size());
}

TEST_F(TDImageHandler, testRotateImageFile)
{
    ASSERT_TRUE(handler->rotateImageFile(tmpFileName, 90));
    handler->setFileName(tmpFileName);
    ASSERT_EQ(QSize(tmpImageHeight, tmpImageWidth), handler->imageSize());

    ASSERT_TRUE(handler->rotateImageFile(tmpFileName, -90));
    handler->clearCache();
    ASSERT_EQ(QSize(tmpImageWidth, tmpImageHeight), handler->imageSize());
}

TEST_F(TDImageHandler, testIsReadable)
{
    ASSERT_FALSE(handler->isReadable());
    handler->setFileName(tmpFileName);
    ASSERT_TRUE(handler->isReadable());
}

TEST_F(TDImageHandler, testIsWriteable)
{
    ASSERT_FALSE(handler->isWriteable());
    handler->setFileName(tmpFileName);
    ASSERT_TRUE(handler->isWriteable());
}

TEST_F(TDImageHandler, testIsRotatable)
{
    ASSERT_FALSE(handler->isRotatable());
    handler->setFileName(tmpFileName);
    ASSERT_TRUE(handler->isRotatable());
}

TEST_F(TDImageHandler, testDetectImageFormat)
{
    QString suffix = DImageHandler::detectImageFormat(tmpFileName);
    ASSERT_EQ(suffix, QString("PNG"));
}

TEST_F(TDImageHandler, testColorFilter)
{
    QImage image(300, 300, QImage::Format_ARGB32);
    image.fill(Qt::red);
    ASSERT_NE(image, DImageHandler::oldColorFilter(image));
    ASSERT_NE(image, DImageHandler::warmColorFilter(image));
    ASSERT_NE(image, DImageHandler::coolColorFilter(image));
    ASSERT_NE(image, DImageHandler::grayScaleColorFilter(image));
    ASSERT_NE(image, DImageHandler::antiColorFilter(image));
    ASSERT_NE(image, DImageHandler::metalColorFilter(image));
}

TEST_F(TDImageHandler, testFlip)
{
    QColor oldColor(Qt::red);
    QColor newColor(Qt::blue);
    QImage image(300, 300, QImage::Format_ARGB32);
    image.fill(oldColor);
    image.setPixelColor(0, 0, newColor);

    QImage horizontalImage = DImageHandler::flipHorizontal(image);
    ASSERT_NE(image, horizontalImage);
    ASSERT_EQ(image, horizontalImage.mirrored(true, false));

    QImage verticalImage = DImageHandler::flipVertical(image);
    ASSERT_NE(image, verticalImage);
    ASSERT_EQ(image, verticalImage.mirrored(false, true));
}

TEST_F(TDImageHandler, testReplacePointColor)
{
    QColor oldColor(Qt::red);
    QColor newColor(Qt::blue);
    QImage image(300, 300, QImage::Format_ARGB32);
    image.fill(oldColor);

    QImage replaceImage = DImageHandler::replacePointColor(image, oldColor, newColor);
    ASSERT_FALSE(replaceImage.isNull());
    ASSERT_EQ(replaceImage.pixelColor(0, 0), newColor);
}