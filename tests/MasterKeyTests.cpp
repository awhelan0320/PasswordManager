#include "MasterKey.h"

#include <gtest/gtest.h>

/**
 * Test cases for the MasterKey class.
 */

 // Test that the same password and salt produce the same key.
TEST(MasterKeyTest, SamePasswordAndSaltProduceSameKey)
{
    const MasterKey::Salt salt{};

    const auto key1 =
        MasterKey::derive("my password", salt);

    const auto key2 =
        MasterKey::derive("my password", salt);

    EXPECT_EQ(key1, key2);
}

// Test that different passwords produce different keys.
TEST(MasterKeyTest, DifferentPasswordsProduceDifferentKeys)
{
    const MasterKey::Salt salt{};

    const auto key1 =
        MasterKey::derive("password one", salt);

    const auto key2 =
        MasterKey::derive("password two", salt);

    EXPECT_NE(key1, key2);
}

// Test that different salts produce different keys.
TEST(MasterKeyTest, DifferentSaltsProduceDifferentKeys)
{
    const MasterKey::Salt salt1{};
    MasterKey::Salt salt2{};

    salt2[0] = 1;

    const auto key1 =
        MasterKey::derive("my password", salt1);

    const auto key2 =
        MasterKey::derive("my password", salt2);

    EXPECT_NE(key1, key2);
}

// Test that generated salts are different.
TEST(MasterKeyTest, GeneratedSaltsAreDifferent)
{
    const auto salt1 = MasterKey::generateSalt();
    const auto salt2 = MasterKey::generateSalt();

    EXPECT_NE(salt1, salt2);
}

