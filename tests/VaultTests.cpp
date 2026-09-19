#include "Vault.h"

#include "SqliteDatabase.h"

#include <gtest/gtest.h>

TEST(VaultTest, CreateStartsUnlocked)
{
    SqliteDatabase database{":memory:"};

    Vault vault{database};

    vault.create("correct password");

    EXPECT_TRUE(vault.isUnlocked());
}

TEST(VaultTest, CorrectPasswordUnlocksVault)
{
    SqliteDatabase database{":memory:"};

    Vault vault{database};

    vault.create("correct password");

    vault.lock();

    EXPECT_FALSE(vault.isUnlocked());

    EXPECT_TRUE(
        vault.unlock("correct password")
    );

    EXPECT_TRUE(vault.isUnlocked());
}

TEST(VaultTest, WrongPasswordDoesNotUnlockVault)
{
    SqliteDatabase database{":memory:"};

    Vault vault{database};

    vault.create("correct password");

    vault.lock();

    EXPECT_FALSE(
        vault.unlock("wrong password")
    );

    EXPECT_FALSE(vault.isUnlocked());
}

TEST(VaultTest, LockLocksVault)
{
    SqliteDatabase database{":memory:"};

    Vault vault{database};

    vault.create("correct password");

    EXPECT_TRUE(vault.isUnlocked());

    vault.lock();

    EXPECT_FALSE(vault.isUnlocked());
}
