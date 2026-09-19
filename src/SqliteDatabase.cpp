#include "SqliteDatabase.h"

#include <stdexcept>

/// \brief Constructs a SqliteDatabase object and opens a connection to the database file.
/// 
/// \param filename The path to the SQLite database file to open or create.
/// \throws std::runtime_error if the database fails to open.
SqliteDatabase::SqliteDatabase(const std::string& filename)
{
    const int result =
        sqlite3_open(filename.c_str(), &db_);

    if (result != SQLITE_OK) {
        std::string error =
            "Failed to open database: " +
            std::string(sqlite3_errmsg(db_));

        sqlite3_close(db_);
        db_ = nullptr;

        throw std::runtime_error(error);
    }
}

/// \brief Returns the underlying sqlite3* database handle.
///
/// \return The sqlite3* handle, or nullptr if the database failed to open. 
sqlite3* SqliteDatabase::get() const
{
    return db_;
}

/// \brief Destructor that closes the database connection if it is open.    
SqliteDatabase::~SqliteDatabase()
{
    if (db_ != nullptr) {
        sqlite3_close(db_);
    }
}