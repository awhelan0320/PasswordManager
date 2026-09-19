#include "VaultDatabase.h"
#include "SqliteStatement.h"
#include <sqlite3.h>
#include <string>
#include <stdexcept>

// The anonymous namespace is used to define helper functions that are only visible within this translation unit (VaultDatabase.cpp). This prevents name clashes with other translation units.
namespace
{
    /**
     * @brief Helper function to safely retrieve text from a SQLite statement column.
     * @param statement Pointer to the SQLite statement.
     * @param column The index of the column to retrieve text from.
     * @return The text from the specified column as a std::string. Returns an empty
     *         string if the column is NULL.
     */
    std::string columnText(sqlite3_stmt* statement, int column)
    {
        const unsigned char* text =
            sqlite3_column_text(statement, column);

        if (text == nullptr) {
            return {};
        }

        return reinterpret_cast<const char*>(text);
    }
}

/**
 * @brief Constructs a VaultDatabase and opens/creates the specified database file.
 * @param database Reference to the SQLite database instance.
 * @throws std::runtime_error if the database fails to open or the table creation fails.
 */
VaultDatabase::VaultDatabase(
    SqliteDatabase& database,
    Vault& vault)
    : db_(database),
      vault_(vault)
{
    createTable();
}

/**
 * @brief Creates the vault_entries table if it does not already exist.
 * @throws std::runtime_error if the table creation fails.
 */
void VaultDatabase::createTable()
{
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS vault_entries (
            id       INTEGER PRIMARY KEY AUTOINCREMENT,
            title    TEXT NOT NULL,
            username TEXT NOT NULL,
            password BLOB NOT NULL,
            url      TEXT,
            notes    TEXT
        );
    )";

    char* errorMessage = nullptr;

// Execute the SQL statement to create the table
// db_             → which database?
// sql             → what SQL?
// nullptr         → callback function
// nullptr         → callback data
// &errorMessage   → where should SQLite put an error message?

// In SQL: CREATE TABLE IF NOT EXISTS vault_entries (id INTEGER PRIMARY KEY AUTOINCREMENT, title TEXT NOT NULL, username TEXT NOT NULL, password BLOB NOT NULL, url TEXT, notes TEXT);


    const int result = sqlite3_exec(
        db_.get(),
        sql,
        nullptr,
        nullptr,
        &errorMessage
    );

    if (result != SQLITE_OK) {
        std::string message = errorMessage
            ? errorMessage
            : "Unknown SQLite error";

        sqlite3_free(errorMessage);

        throw std::runtime_error(
            "Failed to create vault_entries table: " + message
        );
    }
}

/**
 * @brief Retrieves a vault entry by its ID.
 * @param id The ID of the entry to retrieve.
 * @return An optional containing the vault entry if found, or empty if not found.
 */
std::optional<VaultEntry> VaultDatabase::get(int id)
{
    const char* sql = R"(
        SELECT id, title, username, password, url, notes
        FROM vault_entries
        WHERE id = ?;
    )";

    SqliteStatement statement{db_.get(), sql};

    if (sqlite3_bind_int(statement.get(), 1, id) != SQLITE_OK)
    {
        throw std::runtime_error(
            "Failed to bind ID for SELECT: " +
            std::string(sqlite3_errmsg(db_.get()))
        );
    }

    const int stepResult = sqlite3_step(statement.get());

    if (stepResult == SQLITE_DONE)
    {
        return std::nullopt;
    }

    if (stepResult != SQLITE_ROW)
    {
        throw std::runtime_error(
            "Failed to execute SELECT: " +
            std::string(sqlite3_errmsg(db_.get()))
        );
    }

    return readVaultEntry(statement.get());
}


/**
 * @brief Adds a new vault entry.
 * @param entry The vault entry to add.
 * @return true if the entry was added successfully, false otherwise.
 */
bool VaultDatabase::add(VaultEntry& entry)
{

    const auto encryptedPassword =
        vault_.encrypt(entry.password);    
    const char* sql = R"(
        INSERT INTO vault_entries
            (title, username, password, url, notes)
        VALUES
            (?, ?, ?, ?, ?);
    )";

    SqliteStatement statement{db_.get(), sql};

    if (sqlite3_bind_text(
            statement.get(),
            1,
            entry.title.c_str(),
            -1,
            SQLITE_TRANSIENT) != SQLITE_OK)
    {
            throw std::runtime_error(
                "Failed to bind title for INSERT: " +
                std::string(sqlite3_errmsg(db_.get()))
            );
    }

    if (sqlite3_bind_text(
            statement.get(),
            2,
            entry.username.c_str(),
            -1,
            SQLITE_TRANSIENT) != SQLITE_OK)
    {
        throw std::runtime_error(
            "Failed to bind username for INSERT: " +
            std::string(sqlite3_errmsg(db_.get()))
        );
    }

    if (sqlite3_bind_blob(
            statement.get(),
            3,
            encryptedPassword.data(),
            static_cast<int>(encryptedPassword.size()),
            SQLITE_TRANSIENT) != SQLITE_OK)
    {
        throw std::runtime_error(
            "Failed to bind password for INSERT: " +
            std::string(sqlite3_errmsg(db_.get()))
        );
    }

    if (sqlite3_bind_text(
            statement.get(),
            4,
            entry.url.c_str(),
            -1,
            SQLITE_TRANSIENT) != SQLITE_OK)
    {
        throw std::runtime_error(
            "Failed to bind url for INSERT: " +
            std::string(sqlite3_errmsg(db_.get()))
        );
    }

    if (sqlite3_bind_text(
            statement.get(),
            5,
            entry.notes.c_str(),
            -1,
            SQLITE_TRANSIENT) != SQLITE_OK)
    {
        throw std::runtime_error(
            "Failed to bind notes for INSERT: " +
            std::string(sqlite3_errmsg(db_.get()))
        );
    }

    if (sqlite3_step(statement.get()) != SQLITE_DONE) {
        throw std::runtime_error(
            "Failed to execute INSERT: " +
            std::string(sqlite3_errmsg(db_.get()))
        );
    }

    entry.id = static_cast<int>(
        sqlite3_last_insert_rowid(db_.get())
    );

    return true;
}

/**
 * @brief Updates an existing vault entry.
 * @param entry The vault entry to update.
 * @return true if the entry was updated successfully, false otherwise.
 */
bool VaultDatabase::update(const VaultEntry& entry)
{
    const char* sql = R"(
        UPDATE vault_entries
        SET
            title = ?,
            username = ?,
            password = ?,
            url = ?,
            notes = ?
        WHERE id = ?;
    )";

    SqliteStatement statement{db_.get(), sql};
    
    if (sqlite3_bind_text(
            statement.get(), 1, entry.title.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
        throw std::runtime_error(
            "Failed to bind title for UPDATE: " +
            std::string(sqlite3_errmsg(db_.get()))
        );
    }

    if (sqlite3_bind_text(
            statement.get(), 2, entry.username.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
        throw std::runtime_error(
            "Failed to bind username for UPDATE: " +
            std::string(sqlite3_errmsg(db_.get()))
        );
    }

    const auto encryptedPassword =
        vault_.encrypt(entry.password);

    if (sqlite3_bind_blob(
              statement.get(), 3, encryptedPassword.data(), static_cast<int>(encryptedPassword.size()), SQLITE_TRANSIENT) != SQLITE_OK)
    {
        throw std::runtime_error(
            "Failed to bind password for UPDATE: " +
            std::string(sqlite3_errmsg(db_.get()))
        );
    }

    if (sqlite3_bind_text(
            statement.get(), 4, entry.url.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {

        throw std::runtime_error(
            "Failed to bind url for UPDATE: " +
            std::string(sqlite3_errmsg(db_.get()))
        );
    }

    if (sqlite3_bind_text(
            statement.get(), 5, entry.notes.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
        throw std::runtime_error(
            "Failed to bind notes for UPDATE: " +
            std::string(sqlite3_errmsg(db_.get()))
        );
    }

    if (sqlite3_bind_int(statement.get(), 6, entry.id) != SQLITE_OK) {
        throw std::runtime_error(
            "Failed to bind ID for UPDATE: " +
            std::string(sqlite3_errmsg(db_.get()))
        );
    }

    const int stepResult = sqlite3_step(statement.get());

    if (stepResult != SQLITE_DONE) {
        throw std::runtime_error(
            "Failed to execute UPDATE: " +
            std::string(sqlite3_errmsg(db_.get()))
        );
    }

    const int rowsChanged = sqlite3_changes(db_.get());

    return rowsChanged == 1;
}


/**
 * @brief Removes a vault entry by its ID.
 * @param id The ID of the entry to remove.
 * @return true if the entry was removed successfully, false otherwise.
 */
bool VaultDatabase::remove(int id)
{
    const char* sql = R"(
        DELETE FROM vault_entries
        WHERE id = ?;
    )";

    SqliteStatement statement{db_.get(), sql};

    if (sqlite3_bind_int(statement.get(), 1, id) != SQLITE_OK) {
        throw std::runtime_error(
            "Failed to bind ID for DELETE: " +
            std::string(sqlite3_errmsg(db_.get()))
        );
    }

    const int stepResult = sqlite3_step(statement.get());

    if (stepResult != SQLITE_DONE) {
        throw std::runtime_error(
            "Failed to execute DELETE: " +
            std::string(sqlite3_errmsg(db_.get()))
        );
    }

    const int rowsChanged = sqlite3_changes(db_.get());

    return rowsChanged == 1;
}

/**
 * @brief Retrieves all vault entries.
 * @return A vector containing all vault entries.
 */
std::vector<VaultEntry> VaultDatabase::getAll()
{
    const char* sql = R"(
        SELECT id, title, username, password, url, notes
        FROM vault_entries
        ORDER BY id;
    )";

    SqliteStatement statement{db_.get(), sql};

    std::vector<VaultEntry> entries;

    while (true)
    {
        const int stepResult = sqlite3_step(statement.get());

        if (stepResult == SQLITE_ROW)
        {
            entries.push_back(readVaultEntry(statement.get()));
        }
        else if (stepResult == SQLITE_DONE)
        {
            break;
        }
        else
        {
            throw std::runtime_error(
                "Failed to execute SELECT: " +
                std::string(sqlite3_errmsg(db_.get()))
            );
        }
    }

    return entries;
}

/**
 * @brief Reads a VaultEntry from the current row of a SQLite statement.
 * @param statement Pointer to the SQLite statement.
 * @return A VaultEntry populated with data from the current row.
 */
VaultEntry VaultDatabase::readVaultEntry(sqlite3_stmt* statement)
{
    VaultEntry entry;

    entry.id = sqlite3_column_int(statement, 0);

    entry.title = columnText(statement, 1);
    entry.username = columnText(statement, 2);

    const void* passwordData =
        sqlite3_column_blob(statement, 3);

    const int passwordSize =
        sqlite3_column_bytes(statement, 3);

    if (passwordData == nullptr || passwordSize <= 0)
    {
        throw std::runtime_error(
            "Invalid encrypted password in database."
        );
    }

    std::vector<unsigned char> encryptedPassword(
        static_cast<const unsigned char*>(passwordData),
        static_cast<const unsigned char*>(passwordData) + passwordSize
    );

    entry.password = vault_.decrypt(encryptedPassword);
    entry.url = columnText(statement, 4);
    entry.notes = columnText(statement, 5);

    return entry;
}