#include <utils/Compression.h>
#include <error/Status.h>
#include <assets/Img.h>
#include <utils/StreamUtils.h>

using namespace std;

//******************************************************************************
// Constructors
//******************************************************************************
Img::Img(const QVector<char> &imgData, Palette palette) {
    QDataStream stream = QDataStream(QByteArray((imgData.constData()), imgData.size()));
    initFromStreamAndPalette(stream, std::move(palette));
}

Img::Img(QDataStream &imgData, Palette palette) {
    initFromStreamAndPalette(imgData, std::move(palette));
}

Img::Img(const QVector<char> &imgData, quint16 width, quint16 height, Palette palette) :
        mWidth(width), mHeight(height), mRawDataSize(width * height) {
    QDataStream stream = QDataStream(QByteArray((imgData.constData()), imgData.size()));
    initFromStreamAndPalette(stream, std::move(palette), true);
}

Img::Img(QDataStream &imgData, quint16 width, quint16 height, Palette palette) :
        mWidth(width), mHeight(height), mRawDataSize(width * height) {
    initFromStreamAndPalette(imgData, std::move(palette), true);
}

//******************************************************************************
// Methods
//******************************************************************************
bool Img::hasIntegratedPalette() const {
    return mPaletteFlag & 1u;
}

//******************************************************************************
// Getters/setters
//******************************************************************************
quint16 Img::offsetX() const {
    return mOffsetX;
}

quint16 Img::offsetY() const {
    return mOffsetY;
}

quint16 Img::width() const {
    return mWidth;
}

quint16 Img::height() const {
    return mHeight;
}

quint8 Img::compressionFlag() const {
    return mCompressionFlag;
}

quint8 Img::paletteFlag() const {
    return mPaletteFlag;
}

Palette Img::palette() const {
    return mPalette;
}

void Img::setPalette(const Palette &palette) {
    mPalette = palette;
    mQImage.setColorTable(mPalette.getColorTable());
}

QImage Img::qImage() const {
    return mQImage;
}

//******************************************************************************
// Methods
//******************************************************************************
void Img::validatePixelDataAndCreateImage() {
    if (mWidth * mHeight != mImageData.size()) {
        mQImage = QImage();
        throw Status(-1, "This image contained too much or too few pixels for its size");
    } else {
        mQImage = QImage(reinterpret_cast<uchar *>(mImageData.data()), mWidth, mHeight, mWidth,
                         QImage::Format_Indexed8);
    }
}

void Img::initFromStreamAndPalette(QDataStream &imgDataStream, Palette palette, bool noHeader) {
    try {
        if (!noHeader) {
            StreamUtils::verifyStream(imgDataStream, 12);
            imgDataStream.setByteOrder(QDataStream::LittleEndian);
            imgDataStream >> mOffsetX;
            imgDataStream >> mOffsetY;
            imgDataStream >> mWidth;
            imgDataStream >> mHeight;
            imgDataStream >> mCompressionFlag;
            imgDataStream >> mPaletteFlag;
            imgDataStream >> mRawDataSize;
        }
        StreamUtils::verifyStream(imgDataStream, mRawDataSize);
        QVector<char> rawData(mCompressionFlag == 0x08 ? mRawDataSize - 2 : mRawDataSize);
        if (mCompressionFlag == 0x00) {
            StreamUtils::readDataFromStream(imgDataStream, rawData, mRawDataSize);
            mImageData = rawData;
            validatePixelDataAndCreateImage();
        } else if (mCompressionFlag == 0x02) {
            StreamUtils::readDataFromStream(imgDataStream, rawData, mRawDataSize);
            mImageData = Compression::uncompressRLEByLine(rawData, mWidth, mHeight);
            validatePixelDataAndCreateImage();
        } else if (mCompressionFlag == 0x04) {
            StreamUtils::readDataFromStream(imgDataStream, rawData, mRawDataSize);
            mImageData = Compression::uncompressLZSS(rawData);
            validatePixelDataAndCreateImage();
        } else if (mCompressionFlag == 0x08) {
            quint16 uncompressedSize = 0;
            imgDataStream >> uncompressedSize;
            StreamUtils::readDataFromStream(imgDataStream, rawData, mRawDataSize - 2);
            mImageData = Compression::uncompressDeflate(rawData, uncompressedSize);
            validatePixelDataAndCreateImage();
        } else {
            throw Status(-1, QStringLiteral("This image compression is not supported : ") +
                             QString::number(mCompressionFlag));
        }
    }
    catch (Status &e) {
        throw Status(-1, "Unable to load img data : " + e.message());
    }
    // palette setup. Integrated preferred if exists
    if (!mQImage.isNull()) {
        if (hasIntegratedPalette()) {
            bool streamLongEnough = StreamUtils::isStreamAtLeastThisSize(imgDataStream, 768);
            if (streamLongEnough) {
                QVector<char> paletteDescription(768);
                imgDataStream.readRawData(paletteDescription.data(), 768);
                mPalette = Palette(paletteDescription, true);
                mQImage.setColorTable(mPalette.getColorTable());
            } else {
                mQImage.setColorTable(mPalette.getColorTable());
                throw Status(-1, "Integrated palette could not be read");
            }
        } else {
            mPalette = std::move(palette);
            mQImage.setColorTable(mPalette.getColorTable());
        }
    }
}
