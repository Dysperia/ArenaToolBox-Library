#include <QtCore/QDataStream>
#include <error/Status.h>
#include <QFile>
#include <assets/Palette.h>
#include <assets/Img.h>
#include <utils/StreamUtils.h>
#include <utils/FileUtils.h>

Palette::Palette()
{
    for (int i=0; i<256; i++) {
        mColorTable.append(qRgb(i,i,i));
    }
}

Palette::Palette(const QVector<char> &rgbs, bool sixBitsColor) {
    QDataStream imgDataStream(QByteArray((rgbs.constData()), rgbs.size()));
    readDataFromStream(imgDataStream, sixBitsColor);
}

Palette::Palette(const QString &filePath, bool sixBitsColor) {
    QByteArray retrievedData = FileUtils::readDataFromFile(filePath);
    QDataStream imgDataStream(retrievedData);
    // header skip
    if (imgDataStream.skipRawData(8) != 8) {
        throw Status(-1, QString("Palette file does not contain enough data : %1")
                .arg(filePath));
    }
    readDataFromStream(imgDataStream, sixBitsColor);
}

QVector<QRgb> Palette::getColorTable() const
{
    return mColorTable;
}

void Palette::readDataFromStream(QDataStream &imgDataStream, bool sixBitsColor) {
    if (!StreamUtils::isStreamAtLeastThisSize(imgDataStream, 768)) {
        throw Status(-1, QStringLiteral("Palette description does not contain 256 colors"));
    }
    mColorTable.clear();
    quint8 r, g, b;
    for (int i=0; i<256; i++) {
        imgDataStream >> r >> g >> b;
        if (sixBitsColor) {
            r = r << 2u;
            g = g << 2u;
            b = b << 2u;
        }
        mColorTable.append(qRgb(r, g, b));
    }
}
