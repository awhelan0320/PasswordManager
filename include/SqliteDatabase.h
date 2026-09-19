#pragma once

#include <sqlite3.h>

#include <string>

/// Lightweight RAII wrapper around a sqlite3* database handle.
///
/// Opens a SQLite database at construction and closes it on destruction.
/// Copying is disabled to prevent multiple owners of the same handle; move
/// semantics are intentionally not provided here to keep ownership simple.
class SqliteDatabase
{
public:
    /// Construct and open a SQLite database file.
    ///
    /// @param filename Path to the SQLite database file. If the file does not
    /// exist, sqlite3_open will create it according to SQLite's behavior.
    explicit SqliteDatabase(const std::string& filename);

    /// Close the database handle if open.
    ~SqliteDatabase();

    // Non-copyable to enforce single ownership of the sqlite3* handle.
    // SqliteDatabase(const SqliteDatabase&) = delete;
    // SqliteDatabase& operator=(const SqliteDatabase&) = delete;

    /// Access the raw sqlite3* handle. May be nullptr if opening failed.
    ///
    /// The caller does not take ownership; the SqliteDatabase instance owns
    /// and will close the handle.
    sqlite3* get() const;

private:
    sqlite3* db_{nullptr};
};