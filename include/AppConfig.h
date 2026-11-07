
#pragma once

#include <QString>
#include <QDateTime>

// OAuthToken is defined in GoogleTokenClient.h
struct OAuthToken;

class AppConfig
{
public:

    // Explicit config path, primarily useful for testing.
    explicit AppConfig(const QString& configFilePath);

    static AppConfig& getInstance();

    QString getDatabasePath() const;
    void setDatabasePath(const QString& path);

    QString getGoogleOAuthClientId() const;
    QString getConfigFilePath() const;
    QString getGoogleOAuthClientSecret() const;

    void setGoogleOAuthClientId(const QString& clientId);    
    bool saveOAuthTokens(const OAuthToken& token);
    QString getAccessToken() const;
    QString getRefreshToken() const;
    QString getTokenType() const;
    QString getOAuthScope() const;
    QDateTime getTokenExpiration() const;
    
    bool isAccessTokenExpired(int bufferSeconds = 60) const;
    

private:
    AppConfig();
    QString configFilePath_;
};
