#include <utils/StreamUtils.h>
#include <error/Status.h>

//**************************************************************************
// Statics
//**************************************************************************
bool StreamUtils::isStreamAtLeastThisSize(QDataStream &stream, int byteNumber) {
    if (stream.atEnd() || stream.status() != QDataStream::Status::Ok) {
        return false;
    }
    stream.startTransaction();
    int read = stream.skipRawData(byteNumber);
    stream.rollbackTransaction();
    stream.resetStatus();
    return read == byteNumber;
}

void StreamUtils::verifyStream(QDataStream &stream, int byteNumber) {
    if (!isStreamAtLeastThisSize(stream, byteNumber)) {
        throw Status(-1, QStringLiteral("Data is too short or not readable"));
    }
}

bool StreamUtils::readDataFromStream(QDataStream &imgDataStream, QVector<char> &rawData, quint16 size) {
    return imgDataStream.readRawData(rawData.data(), size) == size;
}
