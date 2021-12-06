#include <bsa/BsaArchive.h>
#include <QtConcurrent/QtConcurrent>
#include <utils/FileUtils.h>

//******************************************************************************
// Constructors
//******************************************************************************
BsaArchive::BsaArchive() {
    mReadingStream.setByteOrder(QDataStream::LittleEndian);
}

BsaArchive::~BsaArchive() {
    mArchiveFile.close();
}

//******************************************************************************
// Getters/setters
//******************************************************************************
QString BsaArchive::getArchiveFilePath() const {
    return mArchiveFile.fileName();
}

QString BsaArchive::getArchiveFileName() const {
    return QFileInfo(mArchiveFile).fileName();
}

QVector<BsaFile> BsaArchive::getFiles() const {
    return mFiles;
}

bool BsaArchive::isOpened() const {
    return mOpened;
}

bool BsaArchive::isModified() const {
    return isOpened() &&
           (mFiles.size() != mOriginalFileNumber ||
            any_of(mFiles.begin(), mFiles.end(), [](const BsaFile &file) { return file.isNew() || file.updated(); }));
}

qint64 BsaArchive::size() const {
    auto sizeReduce = [](qint64 &result, const qint64 &current) -> void { result += current; };
    std::function<qint64(const BsaFile &current)> sizeMap = [](const BsaFile &current) {
        return qint64(current.updated() ? current.updateFileSize() : current.size());
    };
    return QtConcurrent::blockingMappedReduced<qint64>(
            mFiles, sizeMap, sizeReduce);
}

quint16 BsaArchive::fileNumber() const {
    return mFiles.size();
}

//**************************************************************************
// Methods
//**************************************************************************
void BsaArchive::openArchive(const QString &filePath) {
    if (this->isOpened()) {
        throw Status(-1, QStringLiteral("An archive is already opened"));
    }
    mArchiveFile.setFileName(filePath);
    if (!mArchiveFile.open(QIODevice::ReadOnly)) {
        throw Status(-1, QString("Could not open the file in read mode : %1")
                .arg(filePath));
    }
    // Getting total file fileSize
    qint64 archiveSize = mArchiveFile.size();
    // Reading file number
    mReadingStream.setDevice(&mArchiveFile);
    mArchiveFile.seek(0);
    mReadingStream >> mOriginalFileNumber;
    // Reading files name and fileSize from file table
    int fileTableSize = FILETABLE_ENTRY_SIZE * mOriginalFileNumber;
    mArchiveFile.seek(archiveSize - fileTableSize);
    quint32 offset = 2;
    char name[14];
    for (quint16 i(0); i < mOriginalFileNumber; i++) {
        quint32 fileSize = 0;
        if (mArchiveFile.atEnd()) {
            throw Status(-1, QString("Reached end of file while reading infos of file %1 of %2")
                    .arg(i + 1).arg(mOriginalFileNumber));
        }
        if (mReadingStream.readRawData(&name[0], 14) < 14) {
            throw Status(-1, QString("Could not read file name of file %1 of %2")
                    .arg(i + 1).arg(mOriginalFileNumber));
        }
        mReadingStream >> fileSize;
        mFiles.append(BsaFile(fileSize, offset, QString(&name[0])));
        offset += fileSize;
    }
    // Checking archive fileSize and integrity
    auto totalSizeFromFiles = size();
    totalSizeFromFiles += 2 + fileTableSize;
    if (totalSizeFromFiles != archiveSize) {
        throw Status(-1, QString("The archive seems corrupted (actual fileSize : %1, expected fileSize : %2")
                .arg(archiveSize).arg(totalSizeFromFiles));
    }
    // Sorting file list by name
    sort(mFiles.begin(), mFiles.end());
    // Archive has been read and ok -> opened
    mOpened = true;
    emit archiveOpened(true);
    emit fileListModified(mFiles);
}

void BsaArchive::closeArchive() {
    if (!this->isOpened()) {
        throw Status(-1, QStringLiteral("Cannot close : archive not opened"));
    }
    if (mArchiveFile.openMode() != QIODevice::NotOpen) {
        mArchiveFile.close();
    }
    mReadingStream.setDevice(nullptr);
    mFiles.clear();
    mOriginalFileNumber = 0;
    mOpened = false;
    emit fileListModified(mFiles);
    emit archiveClosed(true);
}

QVector<char> BsaArchive::getFileData(const BsaFile &file) {
    int idx = verifyArchiveOpenAndFileExists(file);
    auto &internFile = mFiles.at(idx);
    QVector<char> data;
    // External file
    if (internFile.isNew() || internFile.updated()) {
        QByteArray byteArray = FileUtils::readDataFromFile(file.modifiedFilePath());
        data = QVector<char>(byteArray.begin(), byteArray.end());
    }
        // Read from archive
    else {
        if (!mArchiveFile.seek(internFile.startOffsetInArchive())) {
            throw Status(-1, QStringLiteral("The file data is unreadable"));
        }
        data = QVector<char>(int(internFile.size()));
        int bytesRead = mReadingStream.readRawData(data.data(), int(internFile.size()));
        if (internFile.size() != bytesRead) {
            throw Status(-1, QString("Could not retrieve all the data got %1, expected %2")
                    .arg(bytesRead, int(internFile.size())));
        }
    }
    return data;
}

void BsaArchive::extractFile(const QString &destinationFolder, const BsaFile &file) {
    int idx = verifyArchiveOpenAndFileExists(file);
    auto &internFile = mFiles.at(idx);
    QFile saveFile(destinationFolder + QDir::separator() + internFile.fileName());
    if (!saveFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        throw Status(-1, QString("Could not open the file in write mode : %1")
                .arg(saveFile.fileName()));
    }
    QDataStream writeStream(&saveFile);
    writeStream.setByteOrder(QDataStream::LittleEndian);
    QVector<char> data = getFileData(internFile);
    int bytesWritten = writeStream.writeRawData(data.constData(), data.size());
    if (bytesWritten < data.size()) {
        throw Status(-1, QString("Only %1 bytes of %2 written to file %3")
                .arg(bytesWritten)
                .arg(data.size())
                .arg(saveFile.fileName()));
    }
    saveFile.flush();
    saveFile.close();
}

BsaFile BsaArchive::deleteFile(const BsaFile &file) {
    int idx = verifyArchiveOpenAndFileExists(file);
    BsaFile removedFile = mFiles.takeAt(idx);
    emit fileListModified(mFiles);
    return removedFile;
}

BsaFile BsaArchive::addOrUpdateFile(const QString &filePath) {
    QFile newFile(filePath);
    // New file should exist and be readable for size
    if (!newFile.exists() || !newFile.open(QIODevice::ReadOnly)) {
        throw Status(-1, QString("The file %1 doesn't exist or is not readable").arg(filePath));
    }
    auto newFileSize = static_cast<quint32>(newFile.size());
    QString newFileName = QFileInfo(newFile).fileName().toUpper();
    newFile.close();
    // Checking if file already exists in archive
    BsaFile newBsaFile(newFileSize, 2, newFileName);
    newBsaFile.setIsNew(true);
    newBsaFile.setModifiedFilePath(filePath);
    int idx = mFiles.indexOf(newBsaFile);
    // new File
    if (idx == -1) {
        mFiles.append(newBsaFile);
        sort(mFiles.begin(), mFiles.end());
        emit fileListModified(mFiles);
        return newBsaFile;
    }
        // File already exists
    else {
        auto &internFile = mFiles[idx];
        // updating an already new file
        if (internFile.isNew()) {
            mFiles.replace(idx, newBsaFile);
            emit fileModified(newBsaFile);
            return newBsaFile;
        }
            // updating normal file
        else {
            internFile.setUpdated(true);
            internFile.setModifiedFilePath(filePath);
            internFile.setUpdateFileSize(newFileSize);
            emit fileModified(internFile);
            return internFile;
        }
    }
}

BsaFile BsaArchive::revertChanges(const BsaFile &file) {
    int idx = verifyArchiveOpenAndFileExists(file);
    auto &internFile = mFiles[idx];
    // Noting to be done if file not to delete
    if (!internFile.isNew()) {
        return deleteFile(internFile);
    }
    // Updating file state
    internFile.setUpdated(false);
    internFile.setModifiedFilePath(QString());
    internFile.setUpdateFileSize(0);
    emit fileModified(internFile);
    return internFile;
}

void BsaArchive::createNewArchive() {
    if (this->isOpened()) {
        throw Status(-1, QStringLiteral("Cannot create archive: already opened"));
    }
    // init empty archive data
    mArchiveFile.setFileName("");
    mFiles.clear();
    mReadingStream.setDevice(nullptr);
    mOriginalFileNumber = 0;
    mOpened = true;
    emit archiveOpened(true);
    emit fileListModified(mFiles);
}

void BsaArchive::saveArchive(const QString &filePath) {
    if (!this->isOpened()) {
        throw Status(-1, QStringLiteral("Cannot save archive: not opened"));
    }
    QFile saveFile(filePath + ".tmp");
    if (!saveFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        throw Status(-1, QString("Cannot save archive: could not write temporary file %1")
                .arg(saveFile.fileName()));
    }
    QDataStream saveStream(&saveFile);
    saveStream.setByteOrder(QDataStream::LittleEndian);
    size_t totalFileSize(0);
    quint16 nbFileToSave(mFiles.size());
    try {
        // Writing header
        saveStream << nbFileToSave;
        // Writing files data
        for (quint16 i(0); i < quint16(mFiles.size()); i++) {
            const BsaFile &file = mFiles.at(i);
            // Retrieve file data
            QVector<char> fileData = std::move(getFileData(file));
            int dataSize = int(file.updated() ? file.updateFileSize() : file.size());
            // Writing file data
            int bytesWritten = saveStream.writeRawData(fileData.data(), dataSize);
            // Error writing data
            if (bytesWritten != dataSize) {
                throw Status(-1, QString("Error while writing data for file %1. Got only %2 bytes of %3")
                        .arg(file.fileName()).arg(bytesWritten)
                        .arg(dataSize));
            }
            totalFileSize += bytesWritten;
        }
        // Writing file table
        char nullString[14] = {0};
        for (quint16 i(0); i < quint16(mFiles.size()); i++) {
            const BsaFile &file = mFiles.at(i);
            // Writing file name and padding to 14 bytes
            int nameBytes = saveStream.writeRawData(file.fileName().toStdString().c_str(),
                                                    file.fileName().size());
            int paddingBytes = saveStream.writeRawData(&nullString[0],
                                                       14 - file.fileName().size());
            // Error writing data
            if (nameBytes + paddingBytes != 14) {
                throw Status(-1, QString("Error while writing data in file table for file %1")
                        .arg(file.fileName()));
            }
            // Writing file size
            quint32 dataSize = file.updated() ? file.updateFileSize() : file.size();
            saveStream << dataSize;
        }
    } catch (Status &e) {
        saveFile.close();
        saveFile.remove();
        throw Status(-1, "Unable to save archive data : " + e.message());
    }
    // Flushing and closing
    saveFile.flush();
    saveFile.close();
    // Checking temporary saved archive integrity before writing final file
    size_t expectedSize = 2 + totalFileSize + nbFileToSave * FILETABLE_ENTRY_SIZE;
    saveFile.open(QIODevice::ReadOnly);
    qint64 savedSize = saveFile.size();
    saveFile.close();
    if (expectedSize != savedSize) {
        saveFile.remove();
        throw Status(-1, QString("Temporary file not properly saved: saved size: %1, expected: %2. Nothing done")
                .arg(savedSize).arg(expectedSize));
    }
    // Writing final file
    QFile finalFile(filePath);
    // Trying to delete existing if exists
    if (finalFile.exists()) {
        if (!finalFile.remove()) {
            throw Status(-1, QString("Could not delete existing file %1. Temporary saved archive can be found at %2")
                    .arg(finalFile.fileName(), saveFile.fileName()));
        }
    }
    // Renaming temporary
    if (!saveFile.rename(finalFile.fileName())) {
        throw Status(-1, QString("Could not rename temporary saved archive %1 to %2. Saved archive can be found at %1")
                .arg(saveFile.fileName(), finalFile.fileName()));
    }
    // Reloading Archive
    closeArchive();
    openArchive(filePath);
}

int BsaArchive::verifyArchiveOpenAndFileExists(const BsaFile &file) {
    if (!this->isOpened()) {
        throw Status(-1, QStringLiteral("The archive is not opened"));
    }
    int idx = mFiles.indexOf(file);
    if (idx == -1) {
        throw Status(-1, QStringLiteral("The file is not in the archive"));
    }
    return idx;
}
