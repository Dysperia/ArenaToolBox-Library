#ifndef BSATOOL_FILE_H
#define BSATOOL_FILE_H

#include <QString>

/**
 * @brief The File class
 *
 * Describe an archive file. A file with an offset of zero or one indicate an invalid
 * file since the minimum is 2 (bsa archive begins by two bytes for file number)
 */
class BsaFile
{
public:
    //**************************************************************************
    // Statics
    //**************************************************************************

    /**
     * @brief Invalid bsaFile used to return an error. Invalid because an offset
     * of 0 is impossible in a bsa archive: 2 bytes minimum
     */
    const static BsaFile INVALID_BSAFILE;

    //**************************************************************************
    // Constructors
    //**************************************************************************
    /**
     * @brief default constructor (used by QVector)
     */
    BsaFile();
    /**
     * @brief File constructor
     * @param size the file size
     * @param startOffsetInArchive offset at which the data of this file starts in the archive
     * @param fileName the file name
     * @throw Status if filename size is more than 13 characters
     */
    BsaFile(quint32 size,
         qint64 startOffsetInArchive,
         QString fileName);
    /**
     * @brief copy constructor
     */
    BsaFile(const BsaFile &bsaFile) = default;

    //**************************************************************************
    // Operators
    //**************************************************************************
    /**
     * @brief This function overloads operator<().
     * Operator return the result of the filename comparison
     */
    bool operator <(const BsaFile &bsaFile) const;
    /**
     * @brief This function overloads operator<=().
     * Operator return the result of the filename comparison
     */
    bool operator <=(const BsaFile &bsaFile) const;
    /**
     * @brief This function overloads operator>().
     * Operator return the result of the filename comparison
     */
    bool operator >(const BsaFile &bsaFile) const;
    /**
     * @brief This function overloads operator>=().
     * Operator return the result of the filename comparison
     */
    bool operator >=(const BsaFile &bsaFile) const;
    /**
     * @brief equal operator
     * Two files are considered equal if their filenames are
     */
    bool operator ==(const BsaFile &bsaFile) const;
    /**
     * @brief not equal operator
     * Two files are considered not equal if their filenames are not
     */
    bool operator !=(const BsaFile &bsaFile) const;

    //**************************************************************************
    // Methods
    //**************************************************************************
    /**
     * @brief return the file name extension
     * @return return this file name extension or an empty string if no '.' character is found in the file name
     */
    [[nodiscard]]QString getExtension() const;

    /**
     * @brief return true if the offset is greater or equals to two and filename is not empty
     * @return true if the file seems valid, false otherwise
     */
    [[nodiscard]]bool isValid() const;

    //**************************************************************************
    // Getters/setters
    //**************************************************************************
    /**
     * @brief file size
     */
    [[nodiscard]]quint32 size() const;
    /**
     * @brief start offset of the file data in archive
     */
    [[nodiscard]]qint64 startOffsetInArchive() const;
    /**
     * @brief file name
     */
    [[nodiscard]]QString fileName() const;
    /**
     * @brief true if the file is new and to add to the archive
     */
    [[nodiscard]]bool isNew() const;
    /**
     * @brief set to true if the file is new and to add to the archive
     */
    void setIsNew(bool isNew);
    /**
     * @brief true if the file is to be updated with a new version
     */
    [[nodiscard]]bool updated() const;
    /**
     * @brief set to true if the file is to be updated with a new version
     */
    void setUpdated(bool updated);
    /**
     * @brief the size of the update file
     */
    [[nodiscard]]quint32 updateFileSize() const;
    /**
     * @brief set the size of the update file
     */
    void setUpdateFileSize(quint32 updateFileSize);
    /**
     * @brief the complete path to the update or new file
     */
    [[nodiscard]]QString modifiedFilePath() const;
    /**
     * @brief set the complete path to the update or new file
     */
    void setModifiedFilePath(const QString &modifiedFilePath);

private:
    //**************************************************************************
    // Attributes
    //**************************************************************************
    /**
     * @brief file size
     */
    quint32 mSize{0};
    /**
     * @brief start offset of the file data in archive
     */
    qint64 mStartOffsetInArchive{0};
    /**
     * @brief file name
     */
    QString mFileName{};
    /**
     * @brief true if the file is new and to add to the archive
     */
    bool mIsNew{false};
    /**
     * @brief true if the file is to be updated with a new version
     */
    bool mUpdated{false};
    /**
     * @brief the size of the update file
     */
    quint32 mUpdateFileSize{0};
    /**
     * @brief the complete path to the update or new file
     */
    QString mModifiedFilePath{};
};

#endif // BSATOOL_FILE_H
