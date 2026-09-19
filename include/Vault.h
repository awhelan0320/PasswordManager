#pragma once

#include <string>
#include <sodium.h>
#include <vector>
#include "MasterKey.h"

/**
 * VaultMetadata
 *   └── stores/retrieves vault metadata
 *
 *MasterKey
 *   └── derives encryption key
 *
 * VaultVerifier
 *   └── verifies that a key is correct
 *
 * Vault
 *   └── coordinates those pieces
 */

class VaultMetadata;
class SqliteDatabase;

class Vault
{
public:
    explicit Vault(SqliteDatabase& database);

    void create(const std::string& masterPassword);

    bool unlock(const std::string& masterPassword);

    void lock();

    bool isUnlocked() const;

    std::vector<unsigned char> encrypt(
        const std::string& plaintext) const;

    std::string decrypt(
        const std::vector<unsigned char>& ciphertext) const;

private:
    SqliteDatabase& database_;

    bool unlocked_{false};

    MasterKey::Key key_{};
};
