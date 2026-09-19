#include "gtest/gtest.h"
#include "VaultMetadata.h"
#include "SqliteDatabase.h"
#include "VaultVerifier.h"
#include "MasterKey.h"
#include "Vault.h"

// Test cases for the VaultMetadata class.

// Test that the initialize() method creates the vault_metadata table 
// without throwing an exception.
TEST(VaultMetadataTest, InitializeCreatesMetadataTable)
{
    SqliteDatabase database{":memory:"};

    VaultMetadata metadata{database};

    EXPECT_NO_THROW(
        metadata.initialize()
    );
}

/**
 * Test that create() generates a salt and that it can be retrieved using salt().
 * This test ensures that the salt is stored in the database and can be retrieved correctly.
 */

TEST(VaultMetadataTest, CreatedSaltCanBeRetrieved)
{
    SqliteDatabase database{":memory:"};

    VaultMetadata metadata{database};

    metadata.initialize();

    metadata.create("correct password");

    const auto createdSalt =
        metadata.salt();

    const auto retrievedSalt =
        metadata.salt();

    EXPECT_EQ(createdSalt, retrievedSalt);
}

/**
 * Test that the salt remains stable across multiple calls to salt() after it has been created.
 */

 

TEST(VaultMetadataTest, SaltIsStable)
{
    SqliteDatabase database{":memory:"};

    VaultMetadata metadata{database};

    metadata.initialize();



    metadata.create("correct password");

    const auto salt1 =
        metadata.salt();

    const auto salt2 =
        metadata.salt();

    EXPECT_EQ(salt1, salt2);
}

TEST(VaultMetadataTest, CorrectPasswordVerifies)
{
    SqliteDatabase database{":memory:"};

    VaultMetadata metadata{database};

    metadata.initialize();

    metadata.create("correct password");

    const auto salt = metadata.salt();

    const auto key =
        MasterKey::derive(
            "correct password",
            salt
        );

    const auto record =
        metadata.verificationRecord();

    EXPECT_TRUE(
        VaultVerifier::verify(key, record)
    );
}

TEST(VaultMetadataTest, ExistsReturnsFalseForNewDatabase)
{
    SqliteDatabase database{":memory:"};

    VaultMetadata metadata{database};
    metadata.initialize();

    EXPECT_FALSE(metadata.exists());
}

TEST(VaultMetadataTest, ExistsReturnsTrueAfterVaultCreation)
{
    SqliteDatabase database{":memory:"};

    Vault vault{database};
    vault.create("test-master-password");

    VaultMetadata metadata{database};

    EXPECT_TRUE(metadata.exists());
}