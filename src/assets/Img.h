#ifndef BSATOOL_IMG_H
#define BSATOOL_IMG_H

#include <QImage>
#include <assets/Palette.h>

/**
 * @brief Describe the IMG image format
 *
 * If the image has an header it follows the next structure
 * - Offset X : 2 bytes
 * - Offset Y : 2 bytes
 * - Width : 2 bytes
 * - Height : 2 bytes
 * - Compression flags : 1 byte
 * - Palette flags : 1 byte
 * - Image data size : 2 bytes
 * Then follows the image data and, after, the palette data if any
 */
class Img
{
public:
    //**************************************************************************
    // Constructors
    //**************************************************************************
    /**
     * @brief constructor of an empty invalid IMG (null QImage)
     */
    Img() = default;
    /**
     * @brief constructor of IMG with parsing of the header. The well
     * initialization can be checked with the potentially Null status of the
     * QImage
     * @param imgData data of the IMG file
     * @param palette palette used to display the IMG
     * @throw Status if the img could not be loaded correctly
     */
    explicit Img(const QVector<char> &imgData, Palette palette = Palette());
    /**
     * @brief constructor of IMG with parsing of the header. The well
     * initialization can be checked with the potentially Null status of the
     * QImage
     * @param imgData data of the IMG file. The stream will advance
     * @param palette palette used to display the IMG
     * @throw Status if the img could not be loaded correctly
     */
    explicit Img(QDataStream &imgData, Palette palette = Palette());
    /**
     * @brief constructor of IMG without parsing of the header. The well
     * initialization can be checked with the potentially Null status of the
     * QImage
     * @param imgData data of the IMG file
     * @param width of the IMG file
     * @param height of the IMG file
     * @param palette palette used to display the IMG
     * @throw Status if the img could not be loaded correctly
     */
    Img(const QVector<char> &imgData, quint16 width, quint16 height, Palette palette = Palette());
    /**
     * @brief constructor of IMG without parsing of the header. The well
     * initialization can be checked with the potentially Null status of the
     * QImage
     * @param imgData data of the IMG file. The stream will advance
     * @param width of the IMG file
     * @param height of the IMG file
     * @param palette palette used to display the IMG
     * @throw Status if the img could not be loaded correctly
     */
    Img(QDataStream &imgData, quint16 width, quint16 height, Palette palette = Palette());

    //******************************************************************************
    // Methods
    //******************************************************************************

    /**
     * Return true if the img has an integrated palette
     */
    [[nodiscard]] bool hasIntegratedPalette() const;

    //**************************************************************************
    // Getters/setters
    //**************************************************************************
    /**
     * @brief offset X used to draw the image at the correct position on screen
     */
    [[nodiscard]] quint16 offsetX() const;
    /**
     * @brief offset Y used to draw the image at the correct position on screen
     */
    [[nodiscard]] quint16 offsetY() const;
    /**
     * @brief width of the image
     */
    [[nodiscard]] quint16 width() const;
    /**
     * @brief height of the image
     */
    [[nodiscard]] quint16 height() const;
    /**
     * @brief compression flag
     */
    [[nodiscard]] quint8 compressionFlag() const;
    /**
     * @brief palette flag
     */
    [[nodiscard]] quint8 paletteFlag() const;
    /**
     * @brief color palette
     */
    [[nodiscard]] Palette palette() const;
    /**
     * @brief set the color palette and update the qImage frame to use it
     */
    void setPalette(const Palette &palette);
    /**
     * @brief QImage version of this img, mainly used for display
     */
    [[nodiscard]] QImage qImage() const;

private:
    //**************************************************************************
    // Attributes
    //**************************************************************************
    /**
     * @brief offset X used to draw the image at the correct position on screen
     */
    quint16 mOffsetX{0};
    /**
     * @brief offset Y used to draw the image at the correct position on screen
     */
    quint16 mOffsetY{0};
    /**
     * @brief width of the image
     */
    quint16 mWidth{0};
    /**
     * @brief height of the image
     */
    quint16 mHeight{0};
    /**
     * @brief compression flag
     */
    quint8 mCompressionFlag{0};
    /**
     * @brief palette flag
     */
    quint8 mPaletteFlag{0};
    /**
     * @brief size of raw imgData (before uncompression)
     */
    quint16 mRawDataSize{0};
    /**
     * @brief color palette
     */
    Palette mPalette;
    /**
     * @brief image data
     */
    QVector<char> mImageData{};
    /**
     * @brief QImage version of this img, mainly used for display
     */
    QImage mQImage;

    //******************************************************************************
    // Methods
    //******************************************************************************
    /**
     * Validate image data by comparing pixel number and image size (height * width). If validation passed, a non null
     * QImage is created
     * @throw Status if image data is not of expected size
     */
    void validatePixelDataAndCreateImage();

    /**
     * Init image from the given stream and palette. If no header,
     * mOffsetX, mOffsetY, mWidth, mHeight, mCompressionFlag, mPaletteFlag, mRawDataSize
     * should be set before calling this function
     * @param stream containing image data. The stream will advance
     * @param palette color table to use
     * @param noHeader true if data has no header to read
     * @throw Status if could not be loaded correctly
     */
    void initFromStreamAndPalette(QDataStream &imgDataStream, Palette palette, bool noHeader = false);
};

#endif // BSATOOL_IMG_H
