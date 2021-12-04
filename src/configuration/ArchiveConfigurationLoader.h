#ifndef BSATOOL_ARCHIVECONFIGURATIONLOADER_H
#define BSATOOL_ARCHIVECONFIGURATIONLOADER_H

#include <QString>
#include <error/Status.h>
#include <configuration/ArchiveConfiguration.h>
#include <QObject>

class ArchiveConfigurationLoader : public QObject {
Q_OBJECT
public:
    //**************************************************************************
    // Constructors
    //**************************************************************************
    /**
     * @brief default constructor
     */
    ArchiveConfigurationLoader();

    //**************************************************************************
    // Getters/setters
    //**************************************************************************
    [[nodiscard]] const ArchiveConfiguration &getCurrent() const;

    [[nodiscard]] const QStringList &getConfigurationList() const;

public slots:
    //**************************************************************************
    // Slots
    //**************************************************************************
    /**
     * Load a configuration from the given name and set it as current
     * @param name of the configuration
     * @return the current configuration
     * @throw status if the given configuration cannot be loaded
     */
    const ArchiveConfiguration &loadConfiguration(const QString &name);

    /**
     * Update existing configuration list
     * @return the configuration list
     * @throw status if the configuration cannot be loaded
     */
    const QStringList &updateConfigurationList();

private:
    //**************************************************************************
    // Attributes
    //**************************************************************************
    /**
     * @brief currently loaded configuration
     */
    ArchiveConfiguration mCurrent;
    /**
     * @brief list of existing configuration
     */
    QStringList mConfigurationList;
};


#endif //BSATOOL_ARCHIVECONFIGURATIONLOADER_H
