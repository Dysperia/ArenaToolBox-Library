#ifndef BSATOOL_STREAMUTILS_H
#define BSATOOL_STREAMUTILS_H

#include <QDataStream>

using namespace std;

/**
 * Utils class providing various methods to deal with data streams
 */
class StreamUtils {
private:
    //**************************************************************************
    // Constructors
    //**************************************************************************
    StreamUtils() = default;

public:
    //**************************************************************************
    // Static Methods
    //**************************************************************************
    /**
     * Validate the length of the stream
     * @param stream to check
     * @return true if the stream is at least this size
     */
    static bool isStreamAtLeastThisSize(QDataStream &stream, int byteNumber);
    /**
     * Validate the length of the stream and throw a Status instance if not long enough
     * @param stream to check
     * @throw Status if not long enough
     */
    static void verifyStream(QDataStream &stream, int byteNumber);

    /**
     * Read data from stream
     * @param imgDataStream stream to read
     * @param rawData data destination
     * @param size byte number to read
     * @return true if success, false otherwise
     */
    static bool readDataFromStream(QDataStream &imgDataStream, QVector<char> &rawData, quint16 size);
};


//**************************************************************************
// Definitions
//**************************************************************************

// Static methods

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


#endif // BSATOOL_STREAMUTILS_H
