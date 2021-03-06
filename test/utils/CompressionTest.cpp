#include <QtTest/QtTest>
#include <utils/CompressionTest.h>
#include <utils/Compression.h>

void CompressionTest::testLZSSUncompression() {
    qInfo("Should uncompress the file and get the original data");
    QVector<char> uncompressedDataFromFile = readFile(QStringLiteral("ressources/uncompressedLZSS.data"));
    QVERIFY(!uncompressedDataFromFile.isEmpty());
    QVector<char> compressedDataFromFile = readFile(QStringLiteral("ressources/compressedLZSS.data"));
    QVERIFY(!compressedDataFromFile.isEmpty());
    QVector<char> uncompressedDataFromAlgorithm = Compression::uncompressLZSS(compressedDataFromFile);
    QVERIFY(!uncompressedDataFromAlgorithm.isEmpty());
    QCOMPARE(uncompressedDataFromAlgorithm == uncompressedDataFromFile, true);
}

void CompressionTest::testLZSSCompression() {
    qInfo("Should compress then uncompress the file and get the original data");
    QVector<char> uncompressedDataFromFile = readFile(QStringLiteral("ressources/uncompressedLZSS.data"));
    QVERIFY(!uncompressedDataFromFile.isEmpty());

    QVector<char> compressedThenUncompressedDataFromAlgorithm = Compression::uncompressLZSS(
            Compression::compressLZSS(uncompressedDataFromFile));
    QVERIFY(!compressedThenUncompressedDataFromAlgorithm.isEmpty());
    QCOMPARE(compressedThenUncompressedDataFromAlgorithm == uncompressedDataFromFile, true);
}

void CompressionTest::testDeflateUncompression() {
    qInfo("Should uncompress the file and get the original data");
    QVector<char> uncompressedDataFromFile = readFile(QStringLiteral("ressources/uncompressedDeflate.data"));
    QVERIFY(!uncompressedDataFromFile.isEmpty());
    QVector<char> compressedDataFromFile = readFile(QStringLiteral("ressources/compressedDeflate.data"));
    QVERIFY(!compressedDataFromFile.isEmpty());
    QVector<char> uncompressedDataFromAlgorithm = Compression::uncompressDeflate(compressedDataFromFile,
                                                                                 uncompressedDataFromFile.size());
    QVERIFY(!uncompressedDataFromAlgorithm.isEmpty());
    QCOMPARE(uncompressedDataFromAlgorithm == uncompressedDataFromFile, true);
}

void CompressionTest::testDeflateUncompressionWithReset() {
    qInfo("Should uncompress the file and get the original data");
    QVector<char> uncompressedDataFromFile = readFile(QStringLiteral("ressources/uncompressedDeflateWorstCase.data"));
    QVERIFY(!uncompressedDataFromFile.isEmpty());
    QVector<char> compressedDataFromFile = readFile(QStringLiteral("ressources/compressedDeflateWorstCase.data"));
    QVERIFY(!compressedDataFromFile.isEmpty());
    QVector<char> uncompressedDataFromAlgorithm = Compression::uncompressDeflate(compressedDataFromFile,
                                                                                 uncompressedDataFromFile.size());
    QVERIFY(!uncompressedDataFromAlgorithm.isEmpty());
    QCOMPARE(uncompressedDataFromAlgorithm == uncompressedDataFromFile, true);
}

void CompressionTest::testDeflateCompression() {
    qInfo("Should compress then uncompress the file and get the original data");
    QVector<char> uncompressedDataFromFile = readFile(QStringLiteral("ressources/uncompressedDeflate.data"));
    QVERIFY(!uncompressedDataFromFile.isEmpty());

    QVector<char> compressedDataFromAlgorithm = Compression::compressDeflate(uncompressedDataFromFile);
    QVERIFY(!compressedDataFromAlgorithm.isEmpty());

    QVector<char> compressedThenUncompressedDataFromAlgorithm = Compression::uncompressDeflate(
            compressedDataFromAlgorithm, uncompressedDataFromFile.size());
    QVERIFY(!compressedThenUncompressedDataFromAlgorithm.isEmpty());
    QCOMPARE(compressedThenUncompressedDataFromAlgorithm == uncompressedDataFromFile, true);
}

void CompressionTest::testDeflateCompressionWithReset() {
    qInfo("Should compress then uncompress the file with reset and get the original data");
    QVector<char> uncompressedDataFromFile = readFile(QStringLiteral("ressources/uncompressedDeflateWorstCase.data"));
    QVERIFY(!uncompressedDataFromFile.isEmpty());

    QVector<char> compressedDataFromAlgorithm = Compression::compressDeflate(uncompressedDataFromFile);
    QVERIFY(!compressedDataFromAlgorithm.isEmpty());

    QVector<char> compressedThenUncompressedDataFromAlgorithm = Compression::uncompressDeflate(
            compressedDataFromAlgorithm, uncompressedDataFromFile.size());
    QVERIFY(!compressedThenUncompressedDataFromAlgorithm.isEmpty());
    QCOMPARE(compressedThenUncompressedDataFromAlgorithm == uncompressedDataFromFile, true);
}

void CompressionTest::testRLEByLineUncompression() {
    qInfo("Should uncompress the file and get the original data");
    QVector<char> uncompressedDataFromFile = readFile(QStringLiteral("ressources/uncompressedRLEByLine.data"));
    QVERIFY(!uncompressedDataFromFile.isEmpty());
    QVector<char> compressedDataFromFile = readFile(QStringLiteral("ressources/compressedRLEByLine.data"));
    QVERIFY(!compressedDataFromFile.isEmpty());
    QVector<char> uncompressedDataFromAlgorithm = Compression::uncompressRLEByLine(compressedDataFromFile, 61, 147);
    QVERIFY(!uncompressedDataFromAlgorithm.isEmpty());
    QCOMPARE(uncompressedDataFromAlgorithm == uncompressedDataFromFile, true);
}

void CompressionTest::testRLEByLineCompression() {
    qInfo("Should compress then uncompress the file and get the original data");
    QVector<char> uncompressedDataFromFile = readFile(QStringLiteral("ressources/uncompressedRLEByLine.data"));
    QVERIFY(!uncompressedDataFromFile.isEmpty());

    QVector<char> compressedThenUncompressedDataFromAlgorithm = Compression::uncompressRLEByLine(
            Compression::compressRLEByLine(uncompressedDataFromFile, 61, 147), 61, 147);
    QVERIFY(!compressedThenUncompressedDataFromAlgorithm.isEmpty());
    QCOMPARE(compressedThenUncompressedDataFromAlgorithm == uncompressedDataFromFile, true);
}

void CompressionTest::testEncryptionDecryption() {
    qInfo("Should decrypt the file and get the original data");
    QVector<char> decryptedDataFromFile = readFile(QStringLiteral("ressources/decryptedINF.data"));
    QVERIFY(!decryptedDataFromFile.isEmpty());
    QVector<char> encryptedDataFromFile = readFile(QStringLiteral("ressources/encryptedINF.data"));
    QVERIFY(!encryptedDataFromFile.isEmpty());
    QVector<char> decryptedDataFromAlgorithm = Compression::encryptDecrypt(encryptedDataFromFile);
    QVERIFY(!decryptedDataFromAlgorithm.isEmpty());
    QCOMPARE(decryptedDataFromAlgorithm == decryptedDataFromFile, true);

    qInfo("Should encrypt then decrypt the file and get the original data");
    QVector<char> encryptedThenDecryptedDataFromAlgorithm = Compression::encryptDecrypt(
            Compression::encryptDecrypt(decryptedDataFromFile));
    QVERIFY(!encryptedThenDecryptedDataFromAlgorithm.isEmpty());
    QCOMPARE(encryptedThenDecryptedDataFromAlgorithm == decryptedDataFromFile, true);
}

QVector<char> CompressionTest::readFile(const QString &fileName) {
    QFile file(fileName);
    file.open(QIODevice::ReadOnly);
    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);
    QVector<char> data(int(file.size()));
    stream.readRawData(data.data(), int(file.size()));
    file.close();
    return data;
}
