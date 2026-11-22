
#include "AppConfig.h"

#include <QCoreApplication>
#include <QDir>
#include <QSettings>
#include <QStandardPaths>
#include <QFileInfo>
#include <iostream>
#include "GoogleTokenClient.h"

#include <QSettings>

/**
 * @brief Determines the path to the configuration file.
 *
 * This function checks for an environment variable "PASSWORDMANAGER_CONFIG" to determine
 * the configuration file path. If the environment variable is not set, it defaults to
 * a standard location in the user's home directory.
 *
 * @return The path to the configuration file as a QString.
 */

namespace
{
QString determineConfigFilePath()
{
    const QString environmentPath =
        qEnvironmentVariable("PASSWORDMANAGER_CONFIG");

    if (!environmentPath.isEmpty())
        return environmentPath;

    return QDir::homePath()
        + "/.config/PasswordManager/PasswordManager.conf";

    // if (!environmentPath.isEmpty()) {
    //     return QDir::cleanPath(environmentPath);
    // }

    // // Default to the application's standard configuration directory.
    // const QString configDirectory =
    //     QStandardPaths::writableLocation(
    //         QStandardPaths::AppConfigLocation);

    // return configDirectory + "/PasswordManager.conf";

}
}

/**
 * @brief Constructs an AppConfig object with the specified configuration file path.
 *
 * @param configFilePath The path to the configuration file.
 *
 * This constructor initializes the AppConfig object with the provided configuration
 * file path. It is primarily used for testing purposes, allowing the use of a custom
 * configuration file.
 */
AppConfig::AppConfig()
    : configFilePath_{determineConfigFilePath()}
{
    std::cout << "Config file path: " << configFilePath_.toStdString() << std::endl;
}

/**
 * @brief Constructs an AppConfig object with a specified configuration file path.
 *
 * @param configFilePath The path to the configuration file.
 *
 * This constructor allows for the explicit specification of a configuration file path,
 * which is useful for testing or when a non-standard configuration location is desired.
 */
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

/**
 * @brief Retrieves the singleton instance of AppConfig.
 *
 * @return A reference to the singleton AppConfig instance.
 *
 * This method provides access to a single, shared instance of the AppConfig class,
 * ensuring that configuration settings are managed consistently throughout the application.
 */
AppConfig& AppConfig::getInstance()
{
    static AppConfig instance;
    return instance;
}

/**
 * @brief Retrieves the path to the configuration file.
 *
 * @return The path to the configuration file as a QString.
 *
 * This method returns the path that was determined during the construction of the
 * AppConfig object, either from an environment variable or a default location.
 */
QString AppConfig::getConfigFilePath() const
{
    return configFilePath_;
}

/**
 * @brief Retrieves the path to the database file from the configuration.
 *
 * @return The database file path as a QString.
 *
 * This method reads the "Database/path" setting from the configuration file and
 * returns it. If the setting is not found, an empty string is returned.
 */
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

/**
 * @brief Sets the path to the database file in the configuration.
 *
 * @param path The new database file path to be set.
 *
 * This method updates the "Database/path" setting in the configuration file with
 * the provided path. It ensures that the new setting is saved and synchronized
 * with the configuration file.
 */
void AppConfig::setDatabasePath(const QString& path)
{
    QSettings settings{
        configFilePath_,
        QSettings::IniFormat
    };

    settings.setValue("Database/path", path);
    settings.sync();
}

/**
 * @brief Retrieves the Google OAuth Client ID from the configuration.
 *
 * @return The Google OAuth Client ID as a QString.
 *
 * This method reads the "GoogleOAuth/ClientId" setting from the configuration file
 * and returns it. If the setting is not found, an empty string is returned.
 */
QString AppConfig::getGoogleOAuthClientId() const
{
    QSettings settings{
        configFilePath_,
        QSettings::IniFormat
    };
    std::cout << "Google OAuth Client ID from config: " << settings.value("GoogleOAuth/ClientId").toString().toStdString() << std::endl;    
    return settings.value("GoogleOAuth/ClientId").toString();
}

/**
 * @brief Sets the Google OAuth Client ID in the configuration.
 *
 * @param clientId The new Google OAuth Client ID to be set.
 *
 * This method updates the "GoogleOAuth/ClientId" setting in the configuration file
 * with the provided client ID. It ensures that the new setting is saved and synchronized
 * with the configuration file.
 */
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

/**
 * @brief Retrieves the Google OAuth Client Secret from the configuration.
 *
 * @return The Google OAuth Client Secret as a QString.
 *
 * This method reads the "GoogleOAuth/ClientSecret" setting from the configuration file
 * and returns it. If the setting is not found, an empty string is returned.
 */
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

/**
 * @brief Saves the provided OAuth token information to the configuration.
 *
 * @param token The OAuthToken structure containing access and refresh tokens, token type,
 *              scope, and expiration time.
 * @return True if the tokens were successfully saved; otherwise, false.
 *
 * This method updates the configuration file with the provided OAuth token information,
 * ensuring that the access token, refresh token, token type, scope, and expiration time
 * are stored for future use.
 */
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


/**
 * @brief Retrieves the stored access token from the configuration.
 *
 * @return The stored access token as a QString.
 *
 * This method reads the "GoogleOAuth/AccessToken" setting from the configuration file
 * and returns it. If the setting is not found, an empty string is returned.
 */
QString AppConfig::getAccessToken() const
{
    QSettings settings(configFilePath_, QSettings::IniFormat);
    return settings.value("GoogleOAuth/AccessToken").toString();
}

/**
 * @brief Retrieves the stored refresh token from the configuration.
 *
 * @return The stored refresh token as a QString.
 *
 * This method reads the "GoogleOAuth/RefreshToken" setting from the configuration file
 * and returns it. If the setting is not found, an empty string is returned.
 */
QString AppConfig::getRefreshToken() const
{
    QSettings settings(configFilePath_, QSettings::IniFormat);
    return settings.value("GoogleOAuth/RefreshToken").toString();
}

/**
 * @brief Retrieves the stored token type from the configuration.
 *
 * @return The stored token type as a QString.
 *
 * This method reads the "GoogleOAuth/TokenType" setting from the configuration file
 * and returns it. If the setting is not found, an empty string is returned.
 */
QString AppConfig::getTokenType() const
{
    QSettings settings(configFilePath_, QSettings::IniFormat);
    return settings.value("GoogleOAuth/TokenType").toString();
}

/**
 * @brief Retrieves the stored OAuth scope from the configuration.
 *
 * @return The stored OAuth scope as a QString.
 *
 * This method reads the "GoogleOAuth/Scope" setting from the configuration file
 * and returns it. If the setting is not found, an empty string is returned.
 */
QString AppConfig::getOAuthScope() const
{
    QSettings settings(configFilePath_, QSettings::IniFormat);
    return settings.value("GoogleOAuth/Scope").toString();
}

/**
 * @brief Retrieves the expiration time of the stored access token from the configuration.
 *
 * @return The expiration time as a QDateTime object.
 *
 * This method reads the "GoogleOAuth/ExpiresAt" setting from the configuration file,
 * converts it to a QDateTime object, and returns it. If the setting is not found or
 * is invalid, an invalid QDateTime object is returned.
 */
QDateTime AppConfig::getTokenExpiration() const
{
    QSettings settings(configFilePath_, QSettings::IniFormat);

    const QString value =
        settings.value("GoogleOAuth/ExpiresAt").toString();

    return QDateTime::fromString(value, Qt::ISODateWithMs).toUTC();
}

/**
 * @brief Checks if the stored access token is expired, with an optional buffer time in seconds.
 *
 * @param bufferSeconds The number of seconds to subtract from the expiration time for early expiration check.
 * @return True if the access token is expired or invalid; otherwise, false.
 *
 * This method compares the current UTC time with the stored expiration time of the access token,
 * adjusted by the specified buffer time. If the token is expired or invalid, it returns true.
 */
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

/**
 * @brief Clears the stored OAuth tokens from the configuration.
 *
 * This method removes the access token, refresh token, token type, scope, and expiration time
 * from the configuration file, effectively logging out the user from Google OAuth.
 */
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