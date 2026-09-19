#pragma once

#include <sqlite3.h>

/// @class SqliteStatement
/// @brief RAII wrapper for SQLite prepared statements
/// 
/// Manages the lifecycle of sqlite3_stmt objects, automatically finalizing
/// statements when the wrapper is destroyed. Move-only type with deleted copy
/// constructor and assignment operator.
class SqliteStatement
{
public:
    /// @brief Constructs a prepared statement from SQL text
    /// @param db Pointer to an open sqlite3 database connection
    /// @param sql SQL statement text to prepare
    SqliteStatement(sqlite3* db, const char* sql);

    /// @brief Destroys the statement and finalizes the SQLite resource
    ~SqliteStatement();

    /// @brief Copy constructor is deleted (non-copyable)
    SqliteStatement(const SqliteStatement&) = delete;
    
    /// @brief Assignment operator is deleted (non-assignable)
    SqliteStatement& operator=(const SqliteStatement&) = delete;

    /// @brief Returns the underlying sqlite3_stmt pointer
    /// @return Pointer to the prepared statement, or nullptr if invalid
    sqlite3_stmt* get() const;

private:
    sqlite3_stmt* statement_{nullptr}; ///< The underlying SQLite prepared statement
};