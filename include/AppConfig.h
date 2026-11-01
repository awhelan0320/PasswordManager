
#pragma once

#include <QString>

class AppConfig
{
public:
    static AppConfig& getInstance();

    QString getDatabasePath() const;
    void setDatabasePath(const QString& path);

    QString getGoogleOAuthClientId() const;
    void setGoogleOAuthClientId(const QString& clientId);

    QString getConfigFilePath() const;
    QString getGoogleOAuthClientSecret() const;

private:
    AppConfig();

    QString configFilePath_;
};
