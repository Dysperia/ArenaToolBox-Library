#include <utils/FileUtils.h>
#include <error/Status.h>

//**************************************************************************
// Statics
//**************************************************************************
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
