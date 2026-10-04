#pragma once

#include <QString>

class AppConfig
{
public:
    explicit AppConfig(
        const QString& configFile = "db/PasswordManager.conf");

    static AppConfig& getInstance();

    const QString& getDatabasePath() const;
    void setDatabasePath(const QString& path);

private:
    QString databasePath_;
};