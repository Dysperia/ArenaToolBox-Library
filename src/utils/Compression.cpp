#include <error/Status.h>
#include <deque>
#include <utils/Compression.h>
#include <utils/HuffmanTree.h>

// alias
typedef SlidingWindow<char, 4096> SWChar4096;

//**************************************************************************
// Methods
//**************************************************************************
QVector<char> Compression::uncompressLZSS(const QVector<char> &compressedData) {
    // deque to allow fast first element removal and random element access
    deque<char> compressDataDeque;
    for (const auto &byte : compressedData) {
        compressDataDeque.push_back(byte);
    }
    // init sliding window
    SWChar4096 window(false);
    for (int i(0); i < 0xFEE; ++i) {
        window.insert(0x20);
    }
    // flags to know what are the 8 next operations
    // Higher bits are used to know how many flags are remaining
    // Lower bits indicate a sequence copy from window if 0, copy the next incoming byte if 1
    quint16 flags(0);
    // uncompressed data
    QVector<char> uncompressedData;
    // uncompression process
    while (!compressDataDeque.empty()) {
        // shifting flags and getting next 8 if empty
        flags = flags >> 1u;
        if ((flags & 0xFF00u) == 0) {
            flags = BitsReader::getNextUnsignedByte(compressDataDeque) | 0xFF00u;
        }
        // need to insert next byte
        if ((flags & 0x01u) == 1) {
            char nextByte = BitsReader::getNextByte(compressDataDeque);
            uncompressedData.push_back(nextByte);
            // sliding window
            window.insert(nextByte);
        }
        // need to copy sequence from window
        else {
            quint8 byte1 = BitsReader::getNextUnsignedByte(compressDataDeque);
            quint8 byte2 = BitsReader::getNextUnsignedByte(compressDataDeque);
            quint8 length = (byte2 & 0x0Fu) + 3;
            quint16 startIndex = ((byte2 & 0xF0u) << 4u) | byte1;
            // copying sequence
            for (int offset = 0; offset < length; ++offset) {
                char uncompressByte = window.readAtIndex(startIndex + offset);
                uncompressedData.push_back(uncompressByte);
                // sliding window
                window.insert(uncompressByte);
            }
        }
    }
    return move(uncompressedData);
}

QVector<char> Compression::compressLZSS(const QVector<char> &uncompressData) {
    // deque to allow fast first element removal and random element access
    deque<char> uncompressDataDeque;
    for (const auto &byte : uncompressData) {
        uncompressDataDeque.push_back(byte);
    }
    // Max possible length for a duplicate
    // cannot be higher than 18 because length-3 should take at most 4 bits
    const quint8 max_duplicate_length(18);
    // init sliding window of maximum size since offset are encoded using at most 12 bits
    SWChar4096 window;
    for (int i(0); i < 0xFEE; ++i) {
        window.insert(0x20);
    }
    // compression buffer
    QVector<char> compressedBytesBuffer;
    // how many flags have been used
    int flagsNumber(0);
    // flags to know what are the 8 next operations
    // Higher bits are used to know how many flags are remaining
    // Lower bits indicate a sequence copy from window if 0, only the next byte if 1
    quint8 flags(0);
    // compressedData
    QVector<char> compressedData;
    while (!uncompressDataDeque.empty()) {
        // if flags full, need to write buffer
        if (flagsNumber == 8) {
            // Writing flags, then buffer
            compressedData.push_back(char(flags));
            flags = 0;
            flagsNumber = 0;
            for (char i : compressedBytesBuffer) {
                compressedData.push_back(i);
            }
            compressedBytesBuffer.clear();
        }
        // search for a duplicate
        const SWChar4096::DuplicateSearchResult duplicate = window.searchDuplicateInSlidingWindow(uncompressDataDeque,
                                                                                                   max_duplicate_length);
        // Writing compressed data to buffer
        if (duplicate.length > 2) {
            // next flag is 0
            flags = flags >> 1u;
            flagsNumber++;
            // encoding 4 bits length and 12 bits offset
            uint8_t byte1 = duplicate.startIndex & 0x00FFu;
            uint8_t byte2 = ((duplicate.startIndex & 0x0F00u) >> 4u) | (duplicate.length - 3u);
            // writing coordinates to copy
            compressedBytesBuffer.push_back(char(byte1));
            compressedBytesBuffer.push_back(char(byte2));
            // sliding window
            for (int i(0); i < duplicate.length; i++) {
                window.insert(uncompressDataDeque.front());
                uncompressDataDeque.pop_front();
            }
        } else {
            // next flag is 1
            flags = flags >> 1u;
            flags |= 0x80u;
            flagsNumber++;
            // writing next byte to copy
            const char &nextUncompressByte = uncompressDataDeque.front();
            compressedBytesBuffer.push_back(nextUncompressByte);
            // sliding window
            window.insert(nextUncompressByte);
            uncompressDataDeque.pop_front();
        }
    }
    // If less than 8 operations because end of file, need to flush the remaining buffer
    if (flagsNumber > 0) {
        flags = flags >> (8u - flagsNumber);
        compressedData.push_back(char(flags));
        for (char i : compressedBytesBuffer) {
            compressedData.push_back(i);
        }
    }
    return move(compressedData);
}

QVector<char> Compression::uncompressDeflate(const QVector<char> &compressedData, const uint &uncompressedSize) {
    // init huffman tree
    HuffmanTree huffmanTree = HuffmanTree();
    // deque to allow fast first element removal and random element access
    deque<char> compressDataDeque;
    for (const auto &byte : compressedData) {
        compressDataDeque.push_back(byte);
    }
    // init sliding window
    SWChar4096 window(false);
    for (int i(0); i < 4036; ++i) {
        window.insert(0x20);
    }
    // uncompressed data
    QVector<char> uncompressedData;
    // bits reader to manage reading of incoming bits from compressed data
    BitsReader bitsReader(compressDataDeque);
    // decompressing data from source
    while (uncompressedData.size() < uncompressedSize) {
        // searching leaf value in tree from input. The real value is the leaf value minus 627
        quint16 colorOrNbToCopy = huffmanTree.findLeaf(bitsReader) - 627;
        // single byte copy
        if (colorOrNbToCopy < 256) {
            quint8 colorByte = colorOrNbToCopy & 0x00FFu;
            uncompressedData.push_back(char(colorByte));
            window.insert(char(colorByte));
        }
        // copy string from window
        else {
            // Reading index for offset tables
            quint8 offsetTableIdx = bitsReader.getBits();
            bitsReader.removeBits(8);
            // init offset low bits
            quint16 offsetToCopyLowBits = offsetTableIdx;
            // init offset high bits
            quint16 offsetToCopyHighBits = (OFFSET_HIGH_BITS[offsetTableIdx] & 0x00FFu) << 6u;
            // getting missing bits and added them if needed to get the full offset low bits
            quint16 nbBitsToReadAndAdd = (NB_BITS_MISSING_IN_OFFSET_LOW_BITS[offsetTableIdx] & 0x00FFu) - 2u;
            for (; nbBitsToReadAndAdd > 0; nbBitsToReadAndAdd--) {
                quint8 bits = bitsReader.getBits();
                bitsReader.removeBits(1);
                offsetToCopyLowBits = (offsetToCopyLowBits << 1u) + (bits >> 7u);
            }
            // getting offset from high and low bits
            quint16 offsetFromCurrentPosition = (offsetToCopyLowBits & 0x003Fu) | offsetToCopyHighBits;
            // string start position in window
            quint16 copyPosition = (window.getMCurrentInsertPosition() - offsetFromCurrentPosition - 1) & 0x0FFFu;
            // getting length from leaf value (minus 256 because 256 color leaves before length leaves)
            // the length value stored in leaves is the length - 3
            quint16 nbToCopy = colorOrNbToCopy - 256 + 3;
            // string copy
            for (quint16 i(0); i < nbToCopy; i++) {
                quint8 colorByte = window.readAtIndex(copyPosition + i);
                uncompressedData.push_back(char(colorByte));
                window.insert(char(colorByte));
            }
        }
    }
    return move(uncompressedData);
}

QVector<char> Compression::compressDeflate(const QVector<char> &uncompressedData) {
    // init huffman tree
    HuffmanTree huffmanTree = HuffmanTree();
    // deque to allow fast first element removal and random element access
    deque<char> uncompressDataDeque;
    for (const auto &byte : uncompressedData) {
        uncompressDataDeque.push_back(byte);
    }
    // init sliding window
    SWChar4096 window;
    for (int i(0); i < 4036; ++i) {
        window.insert(0x20);
    }
    // Max possible length for a duplicate
    // cannot be higher than 60 because the tree doesn't allow for more
    const quint8 max_duplicate_length(60);
    // compressed data
    QVector<char> compressedData;
    // bits writer to manage writing of produced bits
    BitsWriter bitsWriter(compressedData);
    // decompressing data from source
    while (!uncompressDataDeque.empty()) {
        // search for a duplicate
        const SWChar4096::DuplicateSearchResult duplicate = window.searchDuplicateInSlidingWindow(uncompressDataDeque,
                                                                                                   max_duplicate_length);
        // string copy
        if (duplicate.length > 2) {
            // computing offset from current insert position to save in compressed data
            quint16 offsetFromCurrentPosition =
                    (window.getMCurrentInsertPosition() - duplicate.startIndex - 1) & 0x0FFFu;
            // offset low bits
            quint16 offsetToCopyLowBits = offsetFromCurrentPosition & 0x003Fu;
            // offset high bits
            quint16 offsetToCopyHighBits = offsetFromCurrentPosition >> 6u;
            // Getting index
            quint16 tableIdx = 1000u; // 1000 is an impossible value  : index is in range 0-255
            for (quint16 i(0); i < 256 && tableIdx == 1000u; i++) {
                if (OFFSET_HIGH_BITS[i] == offsetToCopyHighBits) {
                    tableIdx = i; // first possible index
                }
            }
            quint8 nbBitsToGetFromStream = NB_BITS_MISSING_IN_OFFSET_LOW_BITS[tableIdx] - 2u;
            tableIdx += offsetToCopyLowBits >> nbBitsToGetFromStream; // real index
            // Writing data
            huffmanTree.writePathForLeaf(bitsWriter, duplicate.length - 3 + 256 + 627);
            bitsWriter.addBits(tableIdx, 8);
            quint8 nbGarbageBits = NB_BITS_IN_BYTE - nbBitsToGetFromStream;
            quint8 offsetBitsToGetFromStream = offsetFromCurrentPosition << nbGarbageBits;
            bitsWriter.addBits(offsetBitsToGetFromStream, nbBitsToGetFromStream);
            for (quint16 i(0); i < duplicate.length; i++) {
                window.insert(uncompressDataDeque.front());
                uncompressDataDeque.pop_front();
            }
        }
            // single byte copy
        else {
            quint8 colorByte = uncompressDataDeque.front();
            huffmanTree.writePathForLeaf(bitsWriter, colorByte + 627);
            window.insert(char(colorByte));
            uncompressDataDeque.pop_front();
        }
    }
    bitsWriter.flush();
    return move(compressedData);
}

QVector<char>
Compression::uncompressRLEByLine(const QVector<char> &compressedData, const uint &width, const uint &height) {
    // deque to allow fast first element removal and random element access
    deque<char> compressDataDeque;
    for (auto &byte : compressedData) {
        compressDataDeque.push_back(byte);
    }
    // uncompressed data
    QVector<char> uncompressedData;
    // For each line of pixels
    for (int line(0); line < height; line++) {
        // while not done with this line
        uint bytesLeftToProduce = width;
        while (bytesLeftToProduce > 0) {
            // getting number of bytes for this operation
            quint8 counter = BitsReader::getNextUnsignedByte(compressDataDeque);
            // stream of same colors
            if (counter >= 128) {
                counter = (counter & 0x7Fu) + 1u;
                char color = BitsReader::getNextByte(compressDataDeque);
                for (int i(0); i < counter; i++) {
                    uncompressedData.push_back(color);
                }
            }
                // stream of different colors
            else {
                counter++;
                for (int i(0); i < counter; i++) {
                    uncompressedData.push_back(BitsReader::getNextByte(compressDataDeque));
                }
            }
            bytesLeftToProduce -= counter;
        }
    }
    return move(uncompressedData);
}

QVector<char>
Compression::compressRLEByLine(const QVector<char> &uncompressedData, const uint &width, const uint &height) {
    // deque to allow fast first element removal and random element access
    deque<char> uncompressDataDeque;
    for (const auto &byte : uncompressedData) {
        uncompressDataDeque.push_back(byte);
    }
    // compressed data
    QVector<char> compressedData;
    // For each line of pixels
    for (int line(0); line < height; line++) {
        // while not done with this line
        uint bytesLeftToConsume = width;
        while (bytesLeftToConsume > 0) {
            // if only one byte remaining
            if (bytesLeftToConsume == 1) {
                quint8 counterValueToWrite(0);
                compressedData.push_back(char(counterValueToWrite));
                compressedData.push_back(BitsReader::getNextByte(uncompressDataDeque));
                bytesLeftToConsume--;
            }
                // need to explore the sequence
            else {
                quint8 counter(0);
                // need at least two bytes in data to explore sequence
                if (uncompressDataDeque.size() < 2) {
                    throw Status(-1, QStringLiteral("Unexpected end of data"));
                }
                // stream of different color
                if (uncompressDataDeque[counter] != uncompressDataDeque[counter + 1]) {
                    // computing sequence length
                    while (uncompressDataDeque.size() - counter >= 2 &&
                           uncompressDataDeque[counter] != uncompressDataDeque[counter + 1] &&
                           counter < 128 &&
                           bytesLeftToConsume - counter > 0) {
                        counter++;
                    }
                    // adding last byte of this line if possible because line per line
                    if (counter < 128 && bytesLeftToConsume - counter == 1 && !uncompressDataDeque.empty()) {
                        counter++;
                    }
                    // Writing compressed data and consuming uncompressed
                    compressedData.push_back(char(counter - 1));
                    for (int i(0); i < counter; i++) {
                        compressedData.push_back(BitsReader::getNextByte(uncompressDataDeque));
                    }
                    bytesLeftToConsume -= counter;
                }
                    // stream of same color
                else {
                    // computing sequence length
                    while (uncompressDataDeque.size() - counter > 0 &&
                           uncompressDataDeque[0] == uncompressDataDeque[counter] &&
                           counter < 128 &&
                           bytesLeftToConsume - counter > 0) {
                        counter++;
                    }
                    // Writing compressed data and consuming uncompressed
                    compressedData.push_back(char((counter - 1u) | 0x80u));
                    compressedData.push_back(uncompressDataDeque[0]);
                    for (int i(0); i < counter; i++) {
                        uncompressDataDeque.pop_front();
                    }
                    bytesLeftToConsume -= counter;
                }
            }
        }
    }
    return move(compressedData);
}

QVector<char> Compression::uncompressRLE(const QVector<char> &compressedData, const uint &uncompressedSize) {
    return uncompressRLEByLine(compressedData, uncompressedSize, 1);
}

QVector<char> Compression::compressRLE(const QVector<char> &uncompressedData) {
    return compressRLEByLine(uncompressedData, uncompressedData.size(), 1);
}

QVector<char> Compression::encryptDecrypt(const QVector<char> &data, QVector<quint8> cryptKey) {
    // deque to allow fast first element removal and random element access
    deque<char> dataDeque;
    for (const auto &byte : data) {
        dataDeque.push_back(byte);
    }
    // encryption cycles through cryptKey
    int cryptKeyIndex(0);
    // counter resets after 256 operations
    quint8 counter(0);
    // output
    QVector<char> cryptData;
    cryptData.reserve(int(dataDeque.size()));
    // encryption / decryption process
    while (!dataDeque.empty()) {
        // real key to XOR against
        quint8 effectiveKey = counter + cryptKey[cryptKeyIndex];
        // pushing new encrypted / decrypted byte
        cryptData.push_back(char(BitsReader::getNextUnsignedByte(dataDeque) ^ effectiveKey));
        // preparing next step
        counter++;
        cryptKeyIndex = (cryptKeyIndex + 1) % cryptKey.size();
    }
    return move(cryptData);
}
