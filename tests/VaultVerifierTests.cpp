#include "VaultVerifier.h"

#include <gtest/gtest.h>

TEST(VaultVerifierTest, CorrectKeyVerifies)
{
    MasterKey::Salt salt{};

    const auto key =
        MasterKey::derive("correct password", salt);

    const auto record =
        VaultVerifier::create(key);

    EXPECT_TRUE(
        VaultVerifier::verify(key, record)
    );
}

TEST(VaultVerifierTest, WrongKeyFailsVerification)
{
    MasterKey::Salt salt{};

    const auto correctKey =
        MasterKey::derive("correct password", salt);

    const auto wrongKey =
        MasterKey::derive("wrong password", salt);

    const auto record =
        VaultVerifier::create(correctKey);

    EXPECT_FALSE(
        VaultVerifier::verify(wrongKey, record)
    );
}

TEST(VaultVerifierTest, DifferentDerivedKeyFailsVerification)
{
    MasterKey::Salt salt1{};
    MasterKey::Salt salt2{};

    salt2[0] = 1;

    const auto key1 =
        MasterKey::derive("correct password", salt1);

    const auto key2 =
        MasterKey::derive("correct password", salt2);

    const auto record =
        VaultVerifier::create(key1);

    EXPECT_FALSE(
        VaultVerifier::verify(key2, record)
    );
}
