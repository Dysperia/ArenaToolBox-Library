#ifndef BSATOOL_COMPRESSIONTEST_H
#define BSATOOL_COMPRESSIONTEST_H

#include <QObject>

class CompressionTest  : public QObject
{
    Q_OBJECT

private slots:
    /**
     * @brief test LZSS uncompression
     */
    static void testLZSSUncompression();
    /**
     * @brief test LZSS compression
     */
    static void testLZSSCompression();
    /**
     * @brief test deflate uncompression
     */
    static void testDeflateUncompression();
    static void testDeflateUncompressionWithReset();
    /**
     * @brief test deflate compression
     */
    static void testDeflateCompression();
    static void testDeflateCompressionWithReset();
    /**
     * @brief test RLE by line uncompression
     */
    static void testRLEByLineUncompression();
    /**
     * @brief test RLE by line compression
     */
    static void testRLEByLineCompression();
    /**
     * @brief test encryption decryption
     */
    static void testEncryptionDecryption();

public:
    [[nodiscard]] static QVector<char> readFile(const QString &fileName) ;
};


#endif //BSATOOL_COMPRESSIONTEST_H
