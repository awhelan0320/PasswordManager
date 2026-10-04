#include "AppConfig.h"

#include <gtest/gtest.h>

#include <QFile>
#include <QSettings>

class AppConfigTest : public ::testing::Test
{
protected:
    const QString configFile{"test-config.conf"};

    void SetUp() override
    {
        QSettings settings{
            configFile,
            QSettings::IniFormat
        };

        settings.setValue(
            "Database/path",
            "/tmp/test-passwords.db"
        );

        settings.sync();
    }

    void TearDown() override
    {
        QFile::remove(configFile);
    }
};

TEST_F(AppConfigTest, LoadsDatabasePathFromConfigFile)
{
    AppConfig config{configFile};

    EXPECT_EQ(
        config.getDatabasePath(),
        "/tmp/test-passwords.db"
    );
}