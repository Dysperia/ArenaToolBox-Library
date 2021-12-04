#include <configuration/ArchiveConfiguration.h>
#include <QJsonArray>
#include <QFile>
#include <QJsonDocument>

//**************************************************************************
// Methods
//**************************************************************************
void ArchiveConfiguration::read(const QJsonObject &json)
{
    if (json.contains("textureWidth") && json["textureWidth"].isDouble()) {
        mTextureWidth = json["textureWidth"].toInt();
    }
    if (json.contains("defaultPalette") && json["defaultPalette"].isString()) {
        mDefaultPaletteName = json["defaultPalette"].toString();
    }
    if (json.contains("name") && json["name"].isString()) {
        mName = json["name"].toString();
    }
    if (json.contains("files") && json["files"].isArray()) {
        QJsonArray filesArray = json["files"].toArray();
        mFiles.clear();
        mFiles.reserve(filesArray.size());
        for (auto && fileJson : filesArray) {
            QJsonObject fileJsonObject = fileJson.toObject();
            FileConfiguration file;
            file.read(fileJsonObject);
            mFiles.append(file);
        }
    }
}

void ArchiveConfiguration::write(QJsonObject &json) const
{
    json["textureWidth"] = mTextureWidth;
    json["defaultPalette"] = mDefaultPaletteName;
    json["name"] = mName;
    QJsonArray fileArray;
    for (const FileConfiguration &file : mFiles) {
        QJsonObject fileJsonObject;
        file.write(fileJsonObject);
        fileArray.append(fileJsonObject);
    }
    json["files"] = fileArray;
}

void ArchiveConfiguration::loadFromFile() {
    QString filePath("configuration/" + mName + CONFIGURATION_FILE_EXT);
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        throw Status(-1, QString("Could not open the file in read mode : %1")
                .arg(filePath));
    }
    QByteArray retrievedData = file.readAll();
    QJsonDocument loadDoc(QJsonDocument::fromJson(retrievedData));
    read(loadDoc.object());
    mDefaultPalette = Palette("configuration/" + mName + "/" + mDefaultPaletteName);
}

void ArchiveConfiguration::saveToFile() const {
    QString filePath("configuration/" + mName + CONFIGURATION_FILE_EXT);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        throw Status(-1, QString("Could not open the file in write mode : %1")
                .arg(filePath));
    }
    QJsonObject jsonData;
    write(jsonData);
    if (file.write(QJsonDocument(jsonData).toJson()) == -1) {
        throw Status(-1, QString("Could not save configuration"));
    }
}

bool ArchiveConfiguration::hasConfigurationForFile(const BsaFile &file) const {
    return any_of(mFiles.begin(), mFiles.end(),
                  [file](const FileConfiguration& fileConf){ return file.fileName() == fileConf.getFilename(); });
}

const FileConfiguration& ArchiveConfiguration::getConfigurationForFile(const BsaFile &file) const {
    for (auto &fileConf : mFiles) {
        if (file.fileName() == fileConf.getFilename()) {
            return fileConf;
        }
    }
    throw Status(-1, "No configuration file found");
}

QString ArchiveConfiguration::getPathForResource(const QString &name) const {
    return "configuration/" + mName + "/" + name;
}

//**************************************************************************
// Getters/setters
//**************************************************************************
quint16 ArchiveConfiguration::getTextureWidth() const {
    return mTextureWidth;
}

void ArchiveConfiguration::setTextureWidth(quint16 textureWidth) {
    mTextureWidth = textureWidth;
}

const QString &ArchiveConfiguration::getDefaultPaletteName() const {
    return mDefaultPaletteName;
}

void ArchiveConfiguration::setDefaultPaletteName(const QString &defaultPaletteName) {
    mDefaultPaletteName = defaultPaletteName;
}

const Palette &ArchiveConfiguration::getDefaultPalette() const {
    return mDefaultPalette;
}

const QString &ArchiveConfiguration::getName() const {
    return mName;
}

void ArchiveConfiguration::setName(const QString &name) {
    mName = name;
}

QVector<FileConfiguration> &ArchiveConfiguration::getFiles() {
    return mFiles;
}
