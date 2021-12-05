#ifndef BSATOOL_ARCHIVE_H
#define BSATOOL_ARCHIVE_H

#include <bsa/BsaFile.h>
#include <error/Status.h>
#include <QVector>
#include <QDataStream>
#include <QFile>
#include <QObject>

/**
 * @brief Describe a BSA archive
 *
 * The archive is built on the following pattern:
 * - File number: 2 bytes (max file number: 65 535)
 * - Files data put directly one after an other
 * - File table: 18 bytes for each
 *   - 14 bytes for the name (max usable character number for file name: 13 since it is a zero string)
 *   - 4 bytes for the file size (max file size: 4 294 967 295 bytes)
 *
 * Datas are written in little endian
 */
class BsaArchive: public QObject
{
    Q_OBJECT
signals:
    /**
     * @brief signal sent when the archive is fully closed
     * @param opened always false;
     */
    void archiveClosed(bool opened);

    /**
     * @brief signal sent when the archive is fully opened
     * @param opened always true;
     */
    void archiveOpened(bool opened);

    /**
     * @brief signal sent when the files in the archive are modified
     * @param fileList updated file list;
     */
    void fileListModified(QVector<BsaFile> fileList);

    /**
     * @brief signal sent when a file in the archive is modified
     * @param file updated file;
     */
    void fileModified(BsaFile file);

public:
    //**************************************************************************
    // Statics
    //**************************************************************************

    /**
     * @brief Size of a filetable entry: 18 bytes
     */
    const static int FILETABLE_ENTRY_SIZE = 18;

    //**************************************************************************
    // Constructors/Destructor
    //**************************************************************************
    /**
     * @brief Archive constructor
     */
    BsaArchive();

    /**
     * @brief Archive destructor
     */
    ~BsaArchive() override;

    //**************************************************************************
    // Getters/setters
    //**************************************************************************
    [[nodiscard]]QString getArchiveFilePath() const;

    [[nodiscard]]QString getArchiveFileName() const;

    [[nodiscard]]QVector<BsaFile> getFiles() const;

    [[nodiscard]]bool isOpened() const;

    [[nodiscard]]bool isModified() const;

    [[nodiscard]]qint64 size() const;

    [[nodiscard]]quint16 fileNumber() const;

    //**************************************************************************
    // Methods
    //**************************************************************************
    /**
     * @brief open the given archive
     * @param filePath the filepath to the archive
     * @throw status if the archive is already opened, source is unreadable or corrupted
     */
    void openArchive(const QString &filePath);

    /**
     * @brief close this archive and restore state to a not opened archive
     * @throw status if the archive is not opened
     */
    void closeArchive();

    /**
     * @brief retrieve the data of the given file
     * @param file the file to read
     * @return the file data (the external file data in case of an updated or new file)
     * @throw Status if the file is not in the archive or is (partially or fully) unreadable
     */
    QVector<char> getFileData(const BsaFile &file);


    /**
     * @brief extract a file (the external file data in case of an updated or new file)
     * @param destinationFolder destination folder of the file
     * @param file file to extract
     * @throw Status if the file is not in the archive, is unreadable or cannot be written
     */
    void extractFile(const QString &destinationFolder, const BsaFile &file);

    /**
     * @brief delete a file
     * @param file file to delete
     * @return the deleted file
     * @throw Status if the file is not in the archive
     */
    BsaFile deleteFile(const BsaFile &file);

    /**
     * @brief add to (update an existing file of) the archive
     * @param filePath path the new (updated) file
     * @return the file created (updated)
     * @throw Status if the filepath cannot be read or the filename is longer than 13 characters
     */
    BsaFile addOrUpdateFile(const QString &filePath);

    /**
     * @brief cancel the update operation pending on a file. Nothing is done if the file is not new or updated.
     * A new file will be deleted
     * @param file the file to rollback
     * @return the file with its state restored to original or the deleted file if it was new
     * @throw Status if the file is not in the archive
     */
    BsaFile revertChanges(const BsaFile &file);

    /**
     * @brief create a new empty archive
     * @throw status if the archive is already opened
     */
    void createNewArchive();

    /**
     * @brief save the archive to the given file path
     * @param filePath path to the save file
     * @throw status if failure to save data or archive closed
     */
    void saveArchive(const QString &filePath);

private:
    //**************************************************************************
    // Attributes
    //**************************************************************************
    /**
     * @brief Opened state of the archive
     */
    bool mOpened{false};

    /**
     * @brief complete archive path with filename
     */
    QFile mArchiveFile{};

    /**
     * @brief List of the archive files
     */
    QVector<BsaFile> mFiles{};

    /**
     * @brief Original file number when opened
     */
    quint16 mOriginalFileNumber{0};

    /**
     * @brief file stream reading the archive file
     */
    QDataStream mReadingStream{};

    //**************************************************************************
    // Methods
    //**************************************************************************
    /**
     * @brief check and throw error if file is not in the archive or the archive is not opened
     * @param file the file to check
     * @return the index of the file in the list
     * @throw status if the archive is not opened or the file is not in it
     */
    int verifyArchiveOpenAndFileExists(const BsaFile &file);
};

#endif // BSATOOL_ARCHIVE_H
