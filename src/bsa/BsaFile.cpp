#include <utility>
#include <bsa/BsaFile.h>
#include <error/Status.h>

//******************************************************************************
// Statics
//******************************************************************************
const BsaFile BsaFile::INVALID_BSAFILE(0, 0, "INVALID"); // NOLINT(cert-err58-cpp)

//******************************************************************************
// Constructors
//******************************************************************************

BsaFile::BsaFile() :
        mSize(BsaFile::INVALID_BSAFILE.size()), mStartOffsetInArchive(BsaFile::INVALID_BSAFILE.startOffsetInArchive()),
        mFileName(BsaFile::INVALID_BSAFILE.fileName()) {
}

BsaFile::BsaFile(quint32 size,
                 qint64 startOffsetInArchive,
                 QString fileName) :
        mSize(size), mStartOffsetInArchive(startOffsetInArchive), mFileName(std::move(fileName)) {
    if (mFileName.size() > 13) {
        throw Status(-1, QString("The filename %1 is too long (maximum allowed : 13 characters").arg(mFileName));
    }
}

//**************************************************************************
// Operators
//**************************************************************************

bool BsaFile::operator<(const BsaFile &bsaFile) const {
    return mFileName < bsaFile.mFileName;
}

bool BsaFile::operator<=(const BsaFile &bsaFile) const {
    return mFileName <= bsaFile.mFileName;
}

bool BsaFile::operator>(const BsaFile &bsaFile) const {
    return mFileName > bsaFile.mFileName;
}

bool BsaFile::operator>=(const BsaFile &bsaFile) const {
    return mFileName >= bsaFile.mFileName;
}

bool BsaFile::operator==(const BsaFile &bsaFile) const {
    return mFileName == bsaFile.mFileName;
}

bool BsaFile::operator!=(const BsaFile &bsaFile) const {
    return mFileName != bsaFile.mFileName;
}

//**************************************************************************
// Methods
//**************************************************************************
QString BsaFile::getExtension() const {
    int idx = mFileName.lastIndexOf('.');
    if (idx == -1 || idx == mFileName.length()) {
        return {};
    } else {
        return mFileName.mid(idx + 1);
    }
}

bool BsaFile::isValid() const {
    return mStartOffsetInArchive >= 2 && !mFileName.isEmpty();
}

//******************************************************************************
// Getters/setters
//******************************************************************************
quint32 BsaFile::size() const {
    return mSize;
}

qint64 BsaFile::startOffsetInArchive() const {
    return mStartOffsetInArchive;
}

QString BsaFile::fileName() const {
    return mFileName;
}

bool BsaFile::isNew() const {
    return mIsNew;
}

void BsaFile::setIsNew(bool isNew) {
    mIsNew = isNew;
}

bool BsaFile::updated() const {
    return mUpdated;
}

void BsaFile::setUpdated(bool updated) {
    mUpdated = updated;
}

quint32 BsaFile::updateFileSize() const {
    return mUpdateFileSize;
}

void BsaFile::setUpdateFileSize(quint32 updateFileSize) {
    mUpdateFileSize = updateFileSize;
}

QString BsaFile::modifiedFilePath() const {
    return mModifiedFilePath;
}

void BsaFile::setModifiedFilePath(const QString &modifiedFilePath) {
    mModifiedFilePath = modifiedFilePath;
}