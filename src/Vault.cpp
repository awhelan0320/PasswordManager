#include "Vault.h"

#include "VaultMetadata.h"
#include "SqliteDatabase.h"

#include "VaultCrypto.h"
#include <stdexcept>
#include <vector>

Vault::Vault(SqliteDatabase& database)
    : database_(database)
{
}

void Vault::create(const std::string& masterPassword)
{
    VaultMetadata metadata{database_};

    metadata.initialize();

    metadata.create(masterPassword);

    key_ =
        MasterKey::derive(
            masterPassword,
            metadata.salt()
        );

    unlocked_ = true;
}

bool Vault::unlock(const std::string& masterPassword)
{
    VaultMetadata metadata{database_};

    const auto salt = metadata.salt();

    const auto candidateKey =
        MasterKey::derive(
            masterPassword,
            salt
        );

    const auto record =
        metadata.verificationRecord();

    if (!VaultVerifier::verify(candidateKey, record))
    {
        return false;
    }

    key_ = candidateKey;

    unlocked_ = true;

    return true;
}

void Vault::lock()
{
    sodium_memzero(
        key_.data(),
        key_.size()
    );

    unlocked_ = false;
}

bool Vault::isUnlocked() const
{
    return unlocked_;
}

std::vector<unsigned char> Vault::encrypt(
    const std::string& plaintext) const
{
    if (!unlocked_)
    {
        throw std::runtime_error("Vault is locked.");
    }

    return VaultCrypto::encrypt(
        plaintext,
        key_
    );
}

std::string Vault::decrypt(
    const std::vector<unsigned char>& ciphertext) const
{
    if (!unlocked_)
    {
        throw std::runtime_error("Vault is locked.");
    }

    return VaultCrypto::decrypt(
        ciphertext,
        key_
    );
}