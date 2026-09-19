
#include "VaultMetadata.h"
#include "SqliteStatement.h"
#include "SqliteDatabase.h"
#include <sqlite3.h>
#include <stdexcept>
#include <cstring>
#include <memory>




/**
 * @brief Constructs a VaultMetadata instance with the provided SQLite database.
 * @param database The SQLite database to use for metadata storage. 
 */
VaultMetadata::VaultMetadata(SqliteDatabase& database)
    : database_(database)
{
}

/**
 * @brief Initializes the vault metadata table in the database.
 * This method creates the vault_metadata table if it does not already exist.
 * It should be called before any other operations on VaultMetadata.
 */
void VaultMetadata::initialize()
{
    // This table stores vault metadata needed for key derivation, not the
    // user's password entries. The salt is persisted so the same master
    // password can derive the same encryption key whenever the vault is
    // opened. It is not secret, but changing it would make existing data
    // inaccessible because it would produce a different key.
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS vault_metadata (
            id INTEGER PRIMARY KEY,
            salt BLOB NOT NULL,
            verification_nonce BLOB NOT NULL,
            verification_ciphertext BLOB NOT NULL
        );
    )";

    char* errorMessage = nullptr;

    const int result = sqlite3_exec(
        database_.get(),
        sql,
        nullptr,
        nullptr,
        &errorMessage
    );

    if (result != SQLITE_OK)
    {
        std::string message =
            errorMessage ? errorMessage : "Unknown SQLite error";

        sqlite3_free(errorMessage);

        throw std::runtime_error(
            "Failed to create vault_metadata table: " + message
        );
    }
}


void VaultMetadata::create(const std::string& masterPassword)
{

    const auto salt = MasterKey::generateSalt();

    const auto key =
        MasterKey::derive(
            masterPassword,
            salt
        );

    const auto verification =
        VaultVerifier::create(key);

    const char* sql = R"(
        INSERT INTO vault_metadata (
            id,
            salt,
            verification_nonce,
            verification_ciphertext
        )
        VALUES (1, ?, ?, ?);
    )";

    SqliteStatement statement{
        database_.get(),
        sql
    };

    if (sqlite3_bind_blob(
            statement.get(),
            1,
            salt.data(),
            static_cast<int>(salt.size()),
            SQLITE_TRANSIENT) != SQLITE_OK)
    {
        throw std::runtime_error(
            "Failed to bind vault salt."
        );
    }

    if (sqlite3_bind_blob(
            statement.get(),
            2,
            verification.nonce.data(),
            static_cast<int>(verification.nonce.size()),
            SQLITE_TRANSIENT) != SQLITE_OK)
    {
        throw std::runtime_error(
            "Failed to bind verification nonce."
        );
    }

    if (sqlite3_bind_blob(
            statement.get(),
            3,
            verification.ciphertext.data(),
            static_cast<int>(verification.ciphertext.size()),
            SQLITE_TRANSIENT) != SQLITE_OK)
    {
        throw std::runtime_error(
            "Failed to bind verification ciphertext."
        );
    }

    if (sqlite3_step(statement.get()) != SQLITE_DONE)
    {
        throw std::runtime_error(
            "Failed to create vault metadata."
        );
    }
}

/**
 * @brief Retrieves the stored salt from the database.
 * @return The stored salt.
 * This method retrieves the salt from the vault_metadata table. If no salt
 * is found, or if the stored salt is invalid, an exception will be thrown.
 */
MasterKey::Salt VaultMetadata::salt() const
{
    // Retrieve the persisted salt so the vault's encryption key can be
    // recreated from the user's master password.
    const char* sql = R"(
        SELECT salt
        FROM vault_metadata
        WHERE id = 1;
    )";

    SqliteStatement statement{
        database_.get(),
        sql
    };

    const int result = sqlite3_step(statement.get());

    if (result != SQLITE_ROW)
    {
        throw std::runtime_error(
            "Vault metadata does not contain a salt."
        );
    }

    const void* data =
        sqlite3_column_blob(statement.get(), 0);

    const int size =
        sqlite3_column_bytes(statement.get(), 0);

    if (data == nullptr ||
        size != static_cast<int>(MasterKey::Salt{}.size()))
    {
        throw std::runtime_error(
            "Invalid vault salt."
        );
    }

    MasterKey::Salt salt{};

    std::memcpy(
        salt.data(),
        data,
        salt.size()
    );

    return salt;
}

VaultVerifier::Record VaultMetadata::verificationRecord() const
{
    const char* sql = R"(
        SELECT
            verification_nonce,
            verification_ciphertext
        FROM vault_metadata
        WHERE id = 1;
    )";

    SqliteStatement statement{
        database_.get(),
        sql
    };

    if (sqlite3_step(statement.get()) != SQLITE_ROW)
    {
        throw std::runtime_error(
            "Vault verification record not found."
        );
    }

    const void* nonceData =
        sqlite3_column_blob(statement.get(), 0);

    const int nonceSize =
        sqlite3_column_bytes(statement.get(), 0);

    const void* ciphertextData =
        sqlite3_column_blob(statement.get(), 1);

    const int ciphertextSize =
        sqlite3_column_bytes(statement.get(), 1);

    if (nonceData == nullptr ||
        nonceSize != 24 ||
        ciphertextData == nullptr ||
        ciphertextSize <= 0)
    {
        throw std::runtime_error(
            "Invalid vault verification record."
        );
    }

    VaultVerifier::Record record;

    std::memcpy(
        record.nonce.data(),
        nonceData,
        record.nonce.size()
    );

    record.ciphertext.assign(
        static_cast<const unsigned char*>(ciphertextData),
        static_cast<const unsigned char*>(ciphertextData) +
            ciphertextSize
    );

    return record;
}

bool VaultMetadata::exists() const
{
    const char* sql =
        "SELECT 1 "
        "FROM vault_metadata "
        "LIMIT 1;";

    SqliteStatement statement{database_.get(), sql};

    const int result = sqlite3_step(statement.get());

    if (result == SQLITE_ROW)
        return true;

    if (result == SQLITE_DONE)
        return false;

    throw std::runtime_error("Failed to check vault metadata.");
}