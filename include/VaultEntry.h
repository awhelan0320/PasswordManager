#pragma once

#include <string>

/// @struct VaultEntry
/// @brief Represents a single password vault entry with metadata
struct VaultEntry
{
    int id{};                           ///< Unique identifier for the vault entry
    std::string title;                  ///< Title or name of the entry
    std::string username;               ///< Username associated with the entry
    std::string password;               ///< Password associated with the entry
    std::string url;                    ///< URL or website associated with the entry
    std::string notes;                  ///< Additional notes or comments for the entry
};