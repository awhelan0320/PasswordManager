#include "VaultCrypto.h"

#include <gtest/gtest.h>

TEST(VaultCryptoTest, EncryptDecryptRoundTrip)
{
    MasterKey::Salt salt{};

    const auto key =
        MasterKey::derive(
            "correct password",
            salt
        );

    const std::string original =
        "MySecretPassword123!";

    const auto encrypted =
        VaultCrypto::encrypt(
            original,
            key
        );

    const auto decrypted =
        VaultCrypto::decrypt(
            encrypted,
            key
        );

    EXPECT_EQ(decrypted, original);
}

TEST(VaultCryptoTest, WrongKeyCannotDecrypt)
{
    MasterKey::Salt salt1{};
    MasterKey::Salt salt2{};

    salt2[0] = 1;

    const auto correctKey =
        MasterKey::derive(
            "correct password",
            salt1
        );

    const auto wrongKey =
        MasterKey::derive(
            "wrong password",
            salt2
        );

    const auto encrypted =
        VaultCrypto::encrypt(
            "MySecretPassword123!",
            correctKey
        );

    EXPECT_THROW(
        VaultCrypto::decrypt(
            encrypted,
            wrongKey
        ),
        std::runtime_error
    );
}

TEST(VaultCryptoTest, EncryptingSamePlaintextProducesDifferentCiphertext)
{
    MasterKey::Salt salt{};

    const auto key =
        MasterKey::derive(
            "correct password",
            salt
        );

    const auto encrypted1 =
        VaultCrypto::encrypt(
            "MySecretPassword123!",
            key
        );

    const auto encrypted2 =
        VaultCrypto::encrypt(
            "MySecretPassword123!",
            key
        );

    EXPECT_NE(encrypted1, encrypted2);
}

TEST(VaultCryptoTest, ModifiedCiphertextCannotBeDecrypted)
{
    MasterKey::Salt salt{};

    const auto key =
        MasterKey::derive(
            "correct password",
            salt
        );

    auto encrypted =
        VaultCrypto::encrypt(
            "MySecretPassword123!",
            key
        );

    encrypted.back() ^= 0x01;

    EXPECT_THROW(
        VaultCrypto::decrypt(
            encrypted,
            key
        ),
        std::runtime_error
    );
}

