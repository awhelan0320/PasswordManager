#pragma once

#include <string>
#include <vector>
#include "VaultEntry.h"
#include <optional>
#include "SqliteDatabase.h"
#include "Vault.h"

/// @class VaultDatabase
/// @brief Manages persistent storage and retrieval of vault entries using SQLite.
class VaultDatabase
{
public:
    /// @brief Constructs a VaultDatabase and opens/creates the specified database file.
    /// @param database Reference to the SQLite database instance.
    explicit VaultDatabase(
        SqliteDatabase& database,
        Vault& vault);
  

    // VaultDatabase(const VaultDatabase&) = delete;
    // VaultDatabase& operator=(const VaultDatabase&) = delete;
    
    /// @brief Adds a new vault entry to the database.
    /// @param entry The vault entry to add.
    /// @return true if the entry was added successfully, false otherwise.
    bool add(VaultEntry& entry);
    
    /// @brief Retrieves a vault entry by its ID.
    /// @param id The ID of the entry to retrieve.
    /// @return An optional containing the vault entry if found, or empty if not found.
    std::optional<VaultEntry> get(int id);
    
    /// @brief Retrieves all vault entries from the database.
    /// @return A vector containing all vault entries.
    std::vector<VaultEntry> getAll();
    
    /// @brief Updates an existing vault entry in the database.
    /// @param entry The vault entry with updated values.
    /// @return true if the entry was updated successfully, false otherwise.
    bool update(const VaultEntry& entry);
    
    /// @brief Removes a vault entry from the database by its ID.
    /// @param id The ID of the entry to remove.
    /// @return true if the entry was removed successfully, false otherwise.
    bool remove(int id);
    
private:
    void createTable();
    VaultEntry readVaultEntry(sqlite3_stmt* statement);    
    SqliteDatabase db_;
    Vault& vault_;
};