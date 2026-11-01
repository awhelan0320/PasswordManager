
#include "AppConfig.h"

#include <gtest/gtest.h>

#include <QSettings>

class AppConfigTest : public ::testing::Test
{
protected:
    AppConfig& config = AppConfig::getInstance();

    QString originalDatabasePath;
    QString originalClientId;

    void SetUp() override
    {
        originalDatabasePath = config.getDatabasePath();
        originalClientId = config.getGoogleOAuthClientId();
    }

    void TearDown() override
    {
        config.setDatabasePath(originalDatabasePath);
        config.setGoogleOAuthClientId(originalClientId);
    }
};

TEST_F(AppConfigTest, GetInstanceReturnsSameInstance)
{
    AppConfig& first = AppConfig::getInstance();
    AppConfig& second = AppConfig::getInstance();

    EXPECT_EQ(&first, &second);
}

TEST_F(AppConfigTest, ReadsDatabasePath)
{
    qDebug() << "Database path: " << config.getDatabasePath();
    EXPECT_FALSE(config.getDatabasePath().isEmpty());
}

TEST_F(AppConfigTest, ReadsGoogleOAuthClientId)
{
    std::cout << "Google OAuth Client ID: " << config.getGoogleOAuthClientId().toStdString() << std::endl;
    EXPECT_FALSE(config.getGoogleOAuthClientId().isEmpty());
}

TEST_F(AppConfigTest, SavesDatabasePath)
{
    const QString testPath = "/tmp/test-passwords.db";

    config.setDatabasePath(testPath);

    EXPECT_EQ(config.getDatabasePath(), testPath);

    QSettings settings{
        config.getConfigFilePath(),
        QSettings::IniFormat
    };

    settings.sync();

    EXPECT_EQ(
        settings.value("Database/path").toString(),
        testPath
    );
}

TEST_F(AppConfigTest, SavesGoogleOAuthClientId)
{
    const QString testClientId = "test-client-id";

    config.setGoogleOAuthClientId(testClientId);

    EXPECT_EQ(
        config.getGoogleOAuthClientId(),
        testClientId
    );

    QSettings settings{
        config.getConfigFilePath(),
        QSettings::IniFormat
    };

    settings.sync();

    EXPECT_EQ(
        settings.value("GoogleOAuth/ClientId").toString(),
        testClientId
    );
}
