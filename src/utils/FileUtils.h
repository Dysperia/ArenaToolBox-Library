#ifndef BSATOOL_FILEUTILS_H
#define BSATOOL_FILEUTILS_H

#include <QFile>
#include <QFileInfo>

using namespace std;

/**
 * Utils class providing various methods to deal reading file
 */
class FileUtils {
private:
    //**************************************************************************
    // Constructors
    //**************************************************************************
    FileUtils() = default;

public:
    //**************************************************************************
    // Static Methods
    //**************************************************************************

    /**
     * Read data from file
     * @param filePath path to the file to read data from
     * @return the read data
     * @throw Status if data is not readable or not able to read it all
     */
    static QByteArray readDataFromFile(const QString &filePath);
};

#endif // BSATOOL_FILEUTILS_H
