#include "AppConfig.h"

#include <QSettings>

AppConfig::AppConfig(const QString& configFile)
{
    QSettings settings{
        configFile,
        QSettings::IniFormat
    };

    databasePath_ =
        settings.value("Database/path").toString();
}

AppConfig& AppConfig::getInstance()
{
    static AppConfig instance;

    return instance;
}

const QString& AppConfig::getDatabasePath() const
{
    return databasePath_;
}

void AppConfig::setDatabasePath(const QString& path)
{
    databasePath_ = path;
}