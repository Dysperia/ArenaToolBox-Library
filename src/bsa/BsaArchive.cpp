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

quint16 BsaArchive::getFileNumber() const
{
    return mFileNumber;
}

qint64 BsaArchive::getSize() const
{
    return mSize;
}

qint64 BsaArchive::getModifiedSize() const
{
    return mModifiedSize;
}

QVector<BsaFile> BsaArchive::getFiles() const
{
    return mFiles;
}

bool BsaArchive::isOpened() const {
    return mOpened;
}

bool BsaArchive::isModified() const
{
    return mModified;
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
    // Getting total file size
    mSize = mArchiveFile.size();
    mModifiedSize = mSize;
    mReadingStream.setDevice(&mArchiveFile);
    // Reading file number
    mArchiveFile.seek(0);
    mReadingStream >> mFileNumber;
    // Reading files name and size from file table
    int fileTableSize = FILETABLE_ENTRY_SIZE * mFileNumber;
    mArchiveFile.seek(mSize - fileTableSize);
    quint32 offset = 2;
    char name[14];
    quint32 size = 0;
    for (quint16 i(0); i < mFileNumber; i++) {
        if (mArchiveFile.atEnd()) {
            return Status(-1, QString("Reached end of file while reading infos of file %1 of %2")
                         .arg(i+1).arg(mFileNumber));
        }
        if (mReadingStream.readRawData(&name[0], 14) < 14) {
            return Status(-1, QString("Could not read file name of file %1 of %2")
                         .arg(i+1).arg(mFileNumber));
        }
        mReadingStream >> size;
        mFiles.append(BsaFile(size, offset, QString(&name[0])));
        offset += size;
    }
    // Checking archive size and integrity
    auto sizeReduce = [](qint64 &result, const qint64 &current) -> void { result += current; };
    auto totalSizeFromFiles = QtConcurrent::blockingMappedReduced<qint64>(
            mFiles, &BsaFile::size, sizeReduce);
    totalSizeFromFiles += 2 + fileTableSize;
    if (totalSizeFromFiles != mSize) {
        return Status(-1, QString("The archive seems corrupted (actual size : %1, expected size : %2")
                     .arg(mSize).arg(totalSizeFromFiles));
    }
    // Sorting file list by name
    sort(mFiles.begin(), mFiles.end());
    // Archive has been read and ok -> opened
    mOpened = true;
    emit archiveOpened(true);
    emit fileListModified(mFiles);
}

Status BsaArchive::closeArchive()
{
    if (!mOpened) {
        return Status(-1, QStringLiteral("Cannot close : archive not opened"));
    }
    if (mArchiveFile.openMode() != QIODevice::NotOpen) {
        mArchiveFile.close();
    }
    mReadingStream.setDevice(nullptr);
    mOpened = false;
    mModified = false;
    mSize = 0;
    mModifiedSize = 0;
    mFileNumber = 0;
    mFiles.clear();
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

BsaFile BsaArchive::addFile(const QString &filePath)
{
    QFile newFile(filePath);
    // New file should exist and be readable for size
    if (!newFile.exists() || !newFile.open(QIODevice::ReadOnly)) {
        throw Status(-1, QString("The file %1 doesn't exist or is not readable").arg(filePath));
    }
    auto newFileSize = static_cast<quint32>(newFile.size());
    newFile.close();
    // Checking if file already exists in archive
    BsaFile newBsaFile(newFileSize, 2, newFileName);
    newBsaFile.setIsNew(true);
    newBsaFile.setNewFilePath(filePath);
    // Updating archive state
    mFiles.append(newBsaFile);
    mFileNumber++;
    mModifiedSize += newFileSize;
    mModifiedSize += FILETABLE_ENTRY_SIZE;
    mModified = true;
    emit fileListModified(mFiles);
    return newBsaFile;
}

BsaFile BsaArchive::revertChanges(const BsaFile &file) {
    int idx = verifyArchiveOpenAndFileExists(file);
    auto &internFile = mFiles[idx];
    // Noting to be done if file not to delete
    if (!internFile.isNew()) {
        return deleteFile(internFile);
    }
    // Updating file state
    internFile.setToDelete(false);
    // Updating archive state
    mModifiedSize += (internFile.updated() ? internFile.updateFileSize() : internFile.size());
    mModifiedSize += FILETABLE_ENTRY_SIZE;
    updateIsModified();
    emit fileModified(internFile);
    return internFile;
}

BsaFile BsaArchive::cancelUpdateFile(const BsaFile &file)
{
    Status status = verifyIndexOpenOrNewErrors(file);
    if (status.status() < 0) {
        return BsaFile::INVALID_BSAFILE;
    }
    auto &internFile = mFiles[file.index()];
    // Noting to be done if file not updated
    if (!internFile.updated()) {
        return internFile;
    }
    // Updating file state
    internFile.setUpdated(false);
    internFile.setUpdateFilePath("");
    internFile.setUpdateFileSize(0);
    // Updating archive state
    mModifiedSize += internFile.size();
    mModifiedSize -= internFile.updateFileSize();
    updateIsModified();
    emit fileModified(internFile);
    return internFile;
}

Status BsaArchive::createNewArchive()
{
    if (mOpened) {
        return Status(-1, QStringLiteral("Cannot create archive: already opened"));
    }
    // init empty archive data
    mArchiveFile.setFileName("");
    mFileNumber = 0;
    mFiles.clear();
    mModified = false;
    mModifiedSize = 2;
    mOpened = true;
    mReadingStream.setDevice(nullptr);
    mSize = 2;
    emit archiveOpened(true);
    emit fileListModified(mFiles);
}

Status BsaArchive::saveArchive(const QString &filePath)
{
    if (!mOpened) {
        return Status(-1, QStringLiteral("Cannot save archive: not opened"));
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
