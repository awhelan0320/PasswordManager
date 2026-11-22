
#pragma once

#include <QString>
#include <QDateTime>



/**
 * @brief The AppConfig class manages the application's configuration settings.
 *
 * This class provides methods to read and write configuration settings, including
 * database paths and Google OAuth credentials. It uses QSettings to store the
 * configuration in an INI file format.
 */

// OAuthToken is defined in GoogleTokenClient.h
struct OAuthToken;

class AppConfig
{
public:

    // Explicit config path, primarily useful for testing.
    explicit AppConfig(const QString& configFilePath);
    // Default constructor uses the standard config file path.
    static AppConfig& getInstance();
    // Returns the path to the configuration file.
    QString getConfigFilePath() const;
    // Returns the path to the database file.
    QString getDatabasePath() const;
    // Sets the path to the database file in the configuration.
    void setDatabasePath(const QString& path);
    // Retrieves the Google OAuth Client ID from the configuration.
    QString getGoogleOAuthClientId() const;
    // Retrieves the Google OAuth Client Secret from the configuration.
    QString getGoogleOAuthClientSecret() const;
    
    // Sets the Google OAuth Client ID in the configuration.
    void setGoogleOAuthClientId(const QString& clientId);   
    // Saves the provided OAuth token information to the configuration. 
    bool saveOAuthTokens(const OAuthToken& token);
    // Clears the stored OAuth tokens from the configuration.
    void clearOAuthTokens();
    // Retrieves the stored access token from the configuration.
    QString getAccessToken() const;
    // Retrieves the stored refresh token from the configuration.
    QString getRefreshToken() const;
    // Retrieves the stored token type from the configuration.
    QString getTokenType() const;
    // Retrieves the stored OAuth scope from the configuration.
    QString getOAuthScope() const;
    // Retrieves the expiration time of the stored access token from the configuration.
    QDateTime getTokenExpiration() const;
    // Checks if the stored access token is expired, with an optional buffer time in seconds.
    bool isAccessTokenExpired(int bufferSeconds = 60) const;
    

private:
    // Private constructor for singleton pattern.
    AppConfig();
    // Path to the configuration file.
    QString configFilePath_;
};
