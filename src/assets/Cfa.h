#ifndef BSATOOL_CFA_H
#define BSATOOL_CFA_H

#include <QtGui/QImage>
#include <assets/Palette.h>


/**
 * @brief Describe the CFA image format
 *
 * The file describes an animation according to the following.
 * The header is composed of these information :
 * - Width : 2 bytes
 * - Height : 2 bytes
 * - Compressed width : 2 bytes
 * - Offset X : 2 bytes
 * - Offset Y : 2 bytes
 * - Bits per pixel : 1 byte
 * - Number of frames : 1 byte
 * - Header total size : 2 bytes
 * - Starting offset of frame data : 2 bytes * 30 possible frames (not first one). CFA probably max 31 frames
 * - Total file size : 2 bytes
 * - Essential color table : 1 byte * number of color indexes kept from real color table
 * Then follows the frames data compressed in RLE.
 */
class Cfa {

public:
    //**************************************************************************
    // Constructors
    //**************************************************************************
    /**
     * @brief constructor of an empty Cfa
     */
    Cfa() = default;
    /**
     * @brief constructor of Cfa
     * @param data data of the file
     * @throw Status if the cfa could not be loaded correctly
     */
    explicit Cfa(const QVector<char> &data, Palette palette = Palette());

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
     * @brief color palette
     */
    [[nodiscard]] Palette palette() const;
    /**
     * @brief set the color palette and update the qImage frame to use it
     */
    void setPalette(const Palette &palette);
    /**
     * @brief QImage versions of this animation's frames, mainly used for display
     */
    [[nodiscard]] QVector<QImage> qImages() const;

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
     * @brief color palette
     */
    Palette mPalette;
    /**
     * @brief frames data
     */
    QVector<QVector<char>> mFramesData{};
    /**
     * @brief QImage versions of this animation's frames, mainly used for display
     */
    QVector<QImage> mQImages{};

    //******************************************************************************
    // Methods
    //******************************************************************************
    /**
     * Init animation from the given stream and palette
     * @param stream containing animation data
     * @throw Status if the cfa could not be loaded correctly
     */
    void initFromStreamAndPalette(QDataStream &dataStream, const quint16 &dataSize);
};


#endif //BSATOOL_CFA_H
