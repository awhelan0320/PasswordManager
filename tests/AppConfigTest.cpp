
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

#include <gtest/gtest.h>

#include <QTemporaryDir>
#include <QDateTime>

#include "AppConfig.h"
#include "GoogleTokenClient.h"

TEST(AppConfigOAuthTests, SavesAndRetrievesOAuthTokens)
{
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());

    const QString configPath =
        tempDir.filePath("PasswordManager.conf");

    AppConfig config(configPath);

    OAuthToken token;

    token.accessToken = "test_access_token";
    token.refreshToken = "test_refresh_token";
    token.tokenType = "Bearer";
    token.scope = "https://www.googleapis.com/auth/drive.file";
    token.expiresAt =
        QDateTime::currentDateTimeUtc().addSecs(3600);

    ASSERT_TRUE(config.saveOAuthTokens(token));

    EXPECT_EQ(config.getAccessToken(), token.accessToken);
    EXPECT_EQ(config.getRefreshToken(), token.refreshToken);
    EXPECT_EQ(config.getTokenType(), token.tokenType);
    EXPECT_EQ(config.getOAuthScope(), token.scope);

    EXPECT_EQ(
        config.getTokenExpiration().toSecsSinceEpoch(),
        token.expiresAt.toUTC().toSecsSinceEpoch());
}

TEST(AppConfigOAuthTests, PreservesExistingRefreshToken)
{
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());

    AppConfig config(
        tempDir.filePath("PasswordManager.conf"));

    OAuthToken firstToken;
    firstToken.accessToken = "first_access";
    firstToken.refreshToken = "original_refresh";
    firstToken.tokenType = "Bearer";
    firstToken.scope = "drive.file";
    firstToken.expiresAt =
        QDateTime::currentDateTimeUtc().addSecs(3600);

    ASSERT_TRUE(config.saveOAuthTokens(firstToken));

    OAuthToken secondToken;
    secondToken.accessToken = "second_access";
    secondToken.refreshToken.clear();
    secondToken.tokenType = "Bearer";
    secondToken.scope = "drive.file";
    secondToken.expiresAt =
        QDateTime::currentDateTimeUtc().addSecs(3600);

    ASSERT_TRUE(config.saveOAuthTokens(secondToken));

    EXPECT_EQ(config.getAccessToken(), "second_access");
    EXPECT_EQ(config.getRefreshToken(), "original_refresh");
}