#include "SqliteStatement.h"

#include <stdexcept>
#include <string>

/// @brief Constructs a prepared statement from SQL text
/// @param db Pointer to an open sqlite3 database connection
/// @param sql SQL statement text to prepare

SqliteStatement::SqliteStatement(
    sqlite3* db,
    const char* sql)
{
    const int result = sqlite3_prepare_v2(
        db,
        sql,
        -1,
        &statement_,
        nullptr
    );

    if (result != SQLITE_OK) {
        throw std::runtime_error(
            "Failed to prepare statement: " +
            std::string(sqlite3_errmsg(db))
        );
    }
}

/// @brief Destroys the statement and finalizes the SQLite resource
SqliteStatement::~SqliteStatement()
{
    if (statement_ != nullptr) {
        sqlite3_finalize(statement_);
    }
}

/// @brief Returns the underlying sqlite3_stmt pointer
/// @return Pointer to the prepared statement, or nullptr if invalid
sqlite3_stmt* SqliteStatement::get() const
{
    return statement_;
}
