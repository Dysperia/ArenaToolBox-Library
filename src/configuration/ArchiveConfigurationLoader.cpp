#include <QDir>
#include <configuration/ArchiveConfigurationLoader.h>


//**************************************************************************
// Constructors
//**************************************************************************
ArchiveConfigurationLoader::ArchiveConfigurationLoader() {
    updateConfigurationList();
}

//**************************************************************************
// Methods
//**************************************************************************
const ArchiveConfiguration &ArchiveConfigurationLoader::loadConfiguration(const QString &name) {
    ArchiveConfiguration archive;
    archive.setName(name);
    archive.loadFromFile();
    mCurrent = archive;
    emit configurationLoaded(name);
    return mCurrent;
}

const QStringList &ArchiveConfigurationLoader::updateConfigurationList() {
    QString dirPath("configuration/");
    QDir confDir(dirPath);
    if (!confDir.exists() && !confDir.mkdir(dirPath)) {
        throw Status(-1, QString("Could not access / create the configuration directory"));
    }
    QStringList fileList = confDir.entryList({"*" + ArchiveConfiguration::CONFIGURATION_FILE_EXT}, QDir::Filter::Files, QDir::SortFlag::Name);
    mConfigurationList.clear();
    for (auto& filename : fileList) {
        int idx = filename.lastIndexOf(ArchiveConfiguration::CONFIGURATION_FILE_EXT);
        mConfigurationList.push_back(filename.left(idx));
    }
    return mConfigurationList;
}

//**************************************************************************
// Getters/setters
//**************************************************************************
const ArchiveConfiguration &ArchiveConfigurationLoader::getCurrent() const {
    return mCurrent;
}

const QStringList &ArchiveConfigurationLoader::getConfigurationList() const {
    return mConfigurationList;
}
