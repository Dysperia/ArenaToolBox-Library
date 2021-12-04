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


//**************************************************************************
// Definitions
//**************************************************************************

// Static methods

QByteArray FileUtils::readDataFromFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        throw Status(-1, QString("Could not open the file in read mode : %1")
                .arg(filePath));
    }
    QFileInfo info(file);
    QByteArray retrievedData = file.readAll();
    if (info.size() != retrievedData.size()) {
        throw Status(-1, QString("Could not retrieve all the data got %1, expected %2")
                .arg(retrievedData.size(), int(info.size())));
    }
    return retrievedData;
}


#endif // BSATOOL_FILEUTILS_H
