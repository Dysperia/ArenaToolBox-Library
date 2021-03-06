#ifndef BSATOOL_DFA_H
#define BSATOOL_DFA_H

#include <QImage>
#include <assets/Palette.h>
#include <assets/Img.h>

/**
 * @brief Describe the DFA image format
 *
 * The file describes an animation according to the following.
 * The header is composed of these information :
 * - Number of frames : 2 bytes
 * - Offset X : 2 bytes
 * - Offset Y : 2 bytes
 * - Width : 2 bytes
 * - Height : 2 bytes
 * Then follows the first frame :
 * - compressed data length : 2 bytes
 * - rle compressed first frame data
 * Then follows the other frames data stored as differential with first frame :
 * - differential data size : 2 bytes
 * - chunk number : 2 bytes
 * - chunks :
 *      - start offset : 2 bytes
 *      - update pixel number : 2 bytes
 *      - pixels
 */
class Dfa {
public:
    //**************************************************************************
    // Constructors
    //**************************************************************************
    /**
     * @brief constructor of an empty Dfa
     */
    Dfa() = default;
    /**
     * @brief constructor of Dfa
     * @param data data of the file
     * @throw Status if the cfa could not be loaded correctly
     */
    explicit Dfa(const QVector<char> &data, Palette palette = Palette());

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
    void initFromStreamAndPalette(QDataStream &dataStream);
};


#endif //BSATOOL_DFA_H
