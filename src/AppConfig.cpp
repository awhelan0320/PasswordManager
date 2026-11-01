
#include "AppConfig.h"

#include <QCoreApplication>
#include <QDir>
#include <QSettings>
#include <QStandardPaths>
#include <iostream>

namespace
{
QString determineConfigFilePath()
{
    const QString environmentPath =
        qEnvironmentVariable("PASSWORDMANAGER_CONFIG");

    if (!environmentPath.isEmpty()) {
        return QDir::cleanPath(environmentPath);
    }

    // Default to the application's standard configuration directory.
    const QString configDirectory =
        QStandardPaths::writableLocation(
            QStandardPaths::AppConfigLocation);

    return configDirectory + "/PasswordManager.conf";
}
}

AppConfig::AppConfig()
    : configFilePath_{determineConfigFilePath()}
{
    std::cout << "Config file path: " << configFilePath_.toStdString() << std::endl;
}

AppConfig& AppConfig::getInstance()
{
    static AppConfig instance;
    return instance;
}

QString AppConfig::getConfigFilePath() const
{
    return configFilePath_;
}

QString AppConfig::getDatabasePath() const
{
    qDebug() << "Config file path:" << configFilePath_;
    qDebug() << "File exists:" << QFileInfo::exists(configFilePath_);
    qDebug() << "Absolute path:"
             << QFileInfo(configFilePath_).absoluteFilePath();

    QSettings settings{
        configFilePath_,
        QSettings::IniFormat
    };

    qDebug() << "QSettings status:" << settings.status();
    qDebug() << "All keys:" << settings.allKeys();
    qDebug() << "Database/path:"
             << settings.value("Database/path").toString();

    return settings.value("Database/path").toString();
}

void AppConfig::setDatabasePath(const QString& path)
{
    QSettings settings{
        configFilePath_,
        QSettings::IniFormat
    };

    settings.setValue("Database/path", path);
    settings.sync();
}

QString AppConfig::getGoogleOAuthClientId() const
{
    QSettings settings{
        configFilePath_,
        QSettings::IniFormat
    };
    std::cout << "Google OAuth Client ID from config: " << settings.value("GoogleOAuth/ClientId").toString().toStdString() << std::endl;    
    return settings.value("GoogleOAuth/ClientId").toString();
}

void AppConfig::setGoogleOAuthClientId(const QString& clientId)
{
    QSettings settings{
        configFilePath_,
        QSettings::IniFormat
    };

    settings.setValue("GoogleOAuth/ClientId", clientId);
    settings.sync();
}

// QString AppConfig::getGoogleOAuthClientSecret() const
// {
//     QSettings settings(configFilePath_, QSettings::IniFormat);

//     return settings.value("GoogleOAuth/ClientSecret").toString();
// }

QString AppConfig::getGoogleOAuthClientSecret() const
{
    QSettings settings(configFilePath_, QSettings::IniFormat);

    const QString secret =
        settings.value("GoogleOAuth/ClientSecret").toString();

    qDebug() << "Config file:" << settings.fileName();
    qDebug() << "Settings status:" << settings.status();
    qDebug() << "Client secret found:" << !secret.isEmpty();

    return secret;
}