
#include "AppConfig.h"

#include <QCoreApplication>
#include <QDir>
#include <QSettings>
#include <QStandardPaths>
#include <QFileInfo>
#include <iostream>
#include "GoogleTokenClient.h"

#include <QSettings>

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

AppConfig::AppConfig(const QString& configFilePath)
    : configFilePath_(configFilePath)
{
    std::cout << "Config file path: " << configFilePath_.toStdString() << std::endl;
    std::cout << "Config file exists: "
          << std::boolalpha
          << QFileInfo::exists(configFilePath_)
          << std::endl;    
    std::cout << "Absolute path: " << QFileInfo(configFilePath_).absoluteFilePath().toStdString() << std::endl;
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

bool AppConfig::saveOAuthTokens(const OAuthToken& token)
{
    QSettings settings(configFilePath_, QSettings::IniFormat);

    settings.setValue(
        "GoogleOAuth/AccessToken",
        token.accessToken);

    // Google may omit the refresh token on subsequent authorizations.
    // Preserve an existing refresh token in that situation.
    if (!token.refreshToken.isEmpty())
    {
        settings.setValue(
            "GoogleOAuth/RefreshToken",
            token.refreshToken);
    }

    settings.setValue(
        "GoogleOAuth/TokenType",
        token.tokenType);

    if (!token.scope.isEmpty())
    {
        settings.setValue(
            "GoogleOAuth/Scope",
            token.scope);
    }

    settings.setValue(
        "GoogleOAuth/ExpiresAt",
        token.expiresAt.toUTC().toString(Qt::ISODateWithMs));

    settings.sync();

    return settings.status() == QSettings::NoError;
}

QString AppConfig::getAccessToken() const
{
    QSettings settings(configFilePath_, QSettings::IniFormat);
    return settings.value("GoogleOAuth/AccessToken").toString();
}

QString AppConfig::getRefreshToken() const
{
    QSettings settings(configFilePath_, QSettings::IniFormat);
    return settings.value("GoogleOAuth/RefreshToken").toString();
}

QString AppConfig::getTokenType() const
{
    QSettings settings(configFilePath_, QSettings::IniFormat);
    return settings.value("GoogleOAuth/TokenType").toString();
}

QString AppConfig::getOAuthScope() const
{
    QSettings settings(configFilePath_, QSettings::IniFormat);
    return settings.value("GoogleOAuth/Scope").toString();
}

QDateTime AppConfig::getTokenExpiration() const
{
    QSettings settings(configFilePath_, QSettings::IniFormat);

    const QString value =
        settings.value("GoogleOAuth/ExpiresAt").toString();

    return QDateTime::fromString(value, Qt::ISODateWithMs).toUTC();
}

bool AppConfig::isAccessTokenExpired(int bufferSeconds) const
{
    const QDateTime expiration = getTokenExpiration();

    if (!expiration.isValid())
    {
        return true;
    }

    return QDateTime::currentDateTimeUtc().addSecs(bufferSeconds)
           >= expiration;
}

void AppConfig::clearOAuthTokens()
{
    QSettings settings;

    settings.beginGroup("GoogleOAuth");

    settings.remove("AccessToken");
    settings.remove("RefreshToken");
    settings.remove("TokenType");
    settings.remove("ExpiresAt");
    settings.remove("Scope");

    settings.endGroup();

    settings.sync();

    qInfo() << "Stored Google OAuth tokens cleared.";
}