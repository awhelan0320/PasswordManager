/// @file VaultDatabaseTests.cpp
/// @brief Unit tests for VaultDatabase class
///
/// This test suite covers all major functionality of the VaultDatabase class,
/// including:
/// - Adding entries to the database
/// - Retrieving entries by ID and retrieving all entries
/// - Updating existing entries
/// - Removing entries from the database
/// - Handling of null/empty fields and missing IDs

#include "VaultDatabase.h"
#include "SqliteDatabase.h"
#include "Vault.h"
#include <sqlite3.h>
#include <gtest/gtest.h>

#include <cstdio>
#include <iostream>




/// @test Verifies that VaultDatabase correctly adds a new entry and assigns an ID
TEST(VaultDatabaseTest, AddsEntry)
{
    
    const char* databaseName = "add_test.db";

    std::remove(databaseName);

    SqliteDatabase sqliteDb{databaseName};
    Vault vault{sqliteDb};
    vault.create("test-master-password");
    VaultDatabase db{sqliteDb, vault};

    VaultEntry entry{
        0,
        "My Website",
        "andy",
        "secret123",
        "https://example.com",
        "Some notes"
    };

    ASSERT_TRUE(db.add(entry));

    EXPECT_GT(entry.id, 0);

    sqlite3* connection = nullptr;

    ASSERT_EQ(
        sqlite3_open(databaseName, &connection),
        SQLITE_OK
    );

    // const char* sql = R"(
    //     SELECT title, username, password, url, notes
    //     FROM vault_entries;
    // )";

    const char* sql = R"(
        SELECT id, title, username, password, url, notes
        FROM vault_entries;
    )";

    sqlite3_stmt* statement = nullptr;

    ASSERT_EQ(
        sqlite3_prepare_v2(
            connection,
            sql,
            -1,
            &statement,
            nullptr
        ),
        SQLITE_OK
    );

    ASSERT_EQ(
        sqlite3_step(statement),
        SQLITE_ROW
    );

    const int databaseId =
        sqlite3_column_int(statement, 0);

    const char* title =
        reinterpret_cast<const char*>(
            sqlite3_column_text(statement, 1)
        );

    const char* username =
        reinterpret_cast<const char*>(
            sqlite3_column_text(statement, 2)
        );

    const void* password =
        sqlite3_column_blob(statement, 3);

    const int passwordSize =
        sqlite3_column_bytes(statement, 3);

    const char* url =
        reinterpret_cast<const char*>(
            sqlite3_column_text(statement, 4)
        );

    const char* notes =
        reinterpret_cast<const char*>(
            sqlite3_column_text(statement, 5)
        );

    EXPECT_EQ(databaseId, entry.id);
    EXPECT_STREQ(title, entry.title.c_str());
    EXPECT_STREQ(username, entry.username.c_str());
    EXPECT_NE(password, nullptr);
    EXPECT_GT(passwordSize, 0);
    EXPECT_EQ(
        sqlite3_column_type(statement, 3),
        SQLITE_BLOB
    );

    EXPECT_STREQ(url, entry.url.c_str());
    EXPECT_STREQ(notes, entry.notes.c_str());

    sqlite3_finalize(statement);
    sqlite3_close(connection);

    std::remove(databaseName);
}

/// @test Verifies that VaultDatabase correctly retrieves an entry by ID
TEST(VaultDatabaseTest, GetsEntry)
{
    const char* databaseName = "get_test.db";

    std::remove(databaseName);

        SqliteDatabase sqliteDb{databaseName};
    Vault vault{sqliteDb};
    vault.create("test-master-password");
    VaultDatabase db{sqliteDb, vault};

    VaultEntry entry{
        0,
        "My Website",
        "andy",
        "secret123",
        "https://example.com",
        "Some notes"
    };

    ASSERT_TRUE(db.add(entry));
    ASSERT_GT(entry.id, 0);
    std::cout << "Added entry with ID: " << entry.id << std::endl;
    auto result = db.get(entry.id);

    ASSERT_TRUE(result.has_value());

    EXPECT_EQ(result->id, entry.id);
    EXPECT_EQ(result->title, entry.title);
    EXPECT_EQ(result->username, entry.username);
    EXPECT_EQ(result->password, entry.password);
    EXPECT_EQ(result->url, entry.url);
    EXPECT_EQ(result->notes, entry.notes);

    std::remove(databaseName);
}

/// @test Verifies that VaultDatabase returns an empty optional for a non-existent ID
TEST(VaultDatabaseTest, GetReturnsEmptyForUnknownId)
{
    const char* databaseName = "get_missing_test.db";

    std::remove(databaseName);

    SqliteDatabase sqliteDb{databaseName};
    Vault vault{sqliteDb};
    VaultDatabase db{sqliteDb, vault};

    auto result = db.get(9999);

    EXPECT_FALSE(result.has_value());

    std::remove(databaseName);
}

/// @test Verifies that VaultDatabase correctly handles entries with empty optional fields
TEST(VaultDatabaseTest, GetsEmptyOptionalFields)
{
    const char* databaseName = "get_null_test.db";

    std::remove(databaseName);

    SqliteDatabase sqliteDb{databaseName};
    Vault vault{sqliteDb};
    vault.create("test-master-password");
    VaultDatabase db{sqliteDb, vault};

    VaultEntry entry{
        0,
        "My Website",
        "andy",
        "secret123",
        "",
        ""
    };

    ASSERT_TRUE(db.add(entry));
    ASSERT_GT(entry.id, 0);

    auto result = db.get(entry.id);

    ASSERT_TRUE(result.has_value());

    EXPECT_EQ(result->title, entry.title);
    EXPECT_EQ(result->username, entry.username);
    EXPECT_EQ(result->password, entry.password);
    EXPECT_EQ(result->url, "");
    EXPECT_EQ(result->notes, "");

    std::remove(databaseName);
}

/// @test Verifies that VaultDatabase correctly handles entries with NULL text columns in the database
TEST(VaultDatabaseTest, GetHandlesNullTextColumns)
{
    const char* databaseName = "get_null_test.db";

    std::remove(databaseName);

    SqliteDatabase sqliteDb{databaseName};
    Vault vault{sqliteDb};

    vault.create("test-master-password");

    VaultDatabase db{sqliteDb, vault};

    // Encrypt a legitimate password using the Vault.
    const std::string plaintextPassword = "secret123";

    const auto encryptedPassword =
        vault.encrypt(plaintextPassword);

    sqlite3* connection = nullptr;

    ASSERT_EQ(
        sqlite3_open(databaseName, &connection),
        SQLITE_OK
    );

    const char* sql = R"(
        INSERT INTO vault_entries
            (title, username, password, url, notes)
        VALUES
            (?, ?, ?, NULL, NULL);
    )";

    sqlite3_stmt* statement = nullptr;

    ASSERT_EQ(
        sqlite3_prepare_v2(
            connection,
            sql,
            -1,
            &statement,
            nullptr
        ),
        SQLITE_OK
    );

    ASSERT_EQ(
        sqlite3_bind_text(
            statement,
            1,
            "My Website",
            -1,
            SQLITE_STATIC
        ),
        SQLITE_OK
    );

    ASSERT_EQ(
        sqlite3_bind_text(
            statement,
            2,
            "andy",
            -1,
            SQLITE_STATIC
        ),
        SQLITE_OK
    );

    ASSERT_EQ(
        sqlite3_bind_blob(
            statement,
            3,
            encryptedPassword.data(),
            static_cast<int>(encryptedPassword.size()),
            SQLITE_TRANSIENT
        ),
        SQLITE_OK
    );

    ASSERT_EQ(
        sqlite3_step(statement),
        SQLITE_DONE
    );

    sqlite3_finalize(statement);
    sqlite3_close(connection);

    auto result = db.get(1);

    ASSERT_TRUE(result.has_value());

    EXPECT_EQ(result->title, "My Website");
    EXPECT_EQ(result->username, "andy");
    EXPECT_EQ(result->password, plaintextPassword);

    EXPECT_TRUE(result->url.empty());
    EXPECT_TRUE(result->notes.empty());

    std::remove(databaseName);
}

/// @test Verifies that VaultDatabase correctly updates an existing entry
TEST(VaultDatabaseTest, UpdatesEntry)
{
    const char* databaseName = "update_test.db";

    std::remove(databaseName);

    SqliteDatabase sqliteDb{databaseName};
    Vault vault{sqliteDb};
    vault.create("test-master-password");
    VaultDatabase db{sqliteDb, vault};

    VaultEntry entry{
        0,
        "Original Title",
        "andy",
        "original-password",
        "https://example.com",
        "Original notes"
    };

    ASSERT_TRUE(db.add(entry));
    ASSERT_GT(entry.id, 0);

    const int id = entry.id;

    entry.title = "Updated Title";
    entry.username = "new-user";
    entry.password = "new-password";
    entry.url = "https://updated.example.com";
    entry.notes = "Updated notes";

    ASSERT_TRUE(db.update(entry));

    auto result = db.get(id);

    ASSERT_TRUE(result.has_value());

    EXPECT_EQ(result->id, id);
    EXPECT_EQ(result->title, "Updated Title");
    EXPECT_EQ(result->username, "new-user");
    EXPECT_EQ(result->password, "new-password");
    EXPECT_EQ(result->url, "https://updated.example.com");
    EXPECT_EQ(result->notes, "Updated notes");

    std::remove(databaseName);
}

/// @test Verifies that VaultDatabase returns false when attempting to update a non-existent entry
TEST(VaultDatabaseTest, UpdateReturnsFalseForUnknownId)
{
    const char* databaseName = "update_missing_test.db";

    std::remove(databaseName);

    SqliteDatabase sqliteDb{databaseName};
    Vault vault{sqliteDb};
    vault.create("test-master-password");
    VaultDatabase db{sqliteDb, vault};

    VaultEntry entry{
        9999,
        "Title",
        "username",
        "password",
        "https://example.com",
        "notes"
    };

    EXPECT_FALSE(db.update(entry));

    std::remove(databaseName);
}

/// @test Verifies that VaultDatabase correctly removes an entry by ID
TEST(VaultDatabaseTest, RemovesEntry)
{
    const char* databaseName = "remove_test.db";

    std::remove(databaseName);

    SqliteDatabase sqliteDb{databaseName};
    Vault vault{sqliteDb};
    vault.create("test-master-password");
    VaultDatabase db{sqliteDb, vault};

    VaultEntry entry{
        0,
        "My Website",
        "andy",
        "secret123",
        "https://example.com",
        "Some notes"
    };

    ASSERT_TRUE(db.add(entry));
    ASSERT_GT(entry.id, 0);

    const int id = entry.id;

    ASSERT_TRUE(db.get(id).has_value());

    ASSERT_TRUE(db.remove(id));

    EXPECT_FALSE(db.get(id).has_value());

    std::remove(databaseName);
}

/// @test Verifies that VaultDatabase returns false when attempting to remove a non-existent entry
TEST(VaultDatabaseTest, RemoveReturnsFalseForUnknownId)
{
    const char* databaseName = "remove_missing_test.db";

    std::remove(databaseName);

    SqliteDatabase sqliteDb{databaseName};
    Vault vault{sqliteDb};
    VaultDatabase db{sqliteDb, vault};

    EXPECT_FALSE(db.remove(9999));

    std::remove(databaseName);
}

/// @test Verifies that VaultDatabase returns an empty vector when retrieving all entries from an empty database
TEST(VaultDatabaseTest, GetAllReturnsEmptyDatabase)
{
    const char* databaseName = "get_all_empty_test.db";

    std::remove(databaseName);

    SqliteDatabase sqliteDb{databaseName};
    Vault vault{sqliteDb};
    vault.create("test-master-password");
    VaultDatabase db{sqliteDb, vault};

    auto entries = db.getAll();

    EXPECT_TRUE(entries.empty());

    std::remove(databaseName);
}

/// @test Verifies that VaultDatabase correctly retrieves all entries in the order they were added
TEST(VaultDatabaseTest, GetAllReturnsAllEntries)
{
    const char* databaseName = "get_all_test.db";

    std::remove(databaseName);

    SqliteDatabase sqliteDb{databaseName};
    Vault vault{sqliteDb};
    vault.create("test-master-password");
    VaultDatabase db{sqliteDb, vault};

    VaultEntry first{
        0,
        "First",
        "user1",
        "password1",
        "https://first.example.com",
        "First notes"
    };

    VaultEntry second{
        0,
        "Second",
        "user2",
        "password2",
        "https://second.example.com",
        "Second notes"
    };

    VaultEntry third{
        0,
        "Third",
        "user3",
        "password3",
        "https://third.example.com",
        "Third notes"
    };

    ASSERT_TRUE(db.add(first));
    ASSERT_TRUE(db.add(second));
    ASSERT_TRUE(db.add(third));

    auto entries = db.getAll();

    ASSERT_EQ(entries.size(), 3);

    EXPECT_EQ(entries[0].id, first.id);
    EXPECT_EQ(entries[0].title, first.title);

    EXPECT_EQ(entries[1].id, second.id);
    EXPECT_EQ(entries[1].title, second.title);

    EXPECT_EQ(entries[2].id, third.id);
    EXPECT_EQ(entries[2].title, third.title);

    std::remove(databaseName);
}

TEST(VaultDatabaseTest, PasswordRoundTripsEncrypted)
{
    SqliteDatabase database{":memory:"};

    Vault vault{database};

    vault.create("master password");

    VaultDatabase vaultDatabase{
        database,
        vault
    };

    VaultEntry entry;

    entry.title = "Test Entry";
    entry.username = "andy";
    entry.password = "SuperSecret123!";
    entry.url = "https://example.com";
    entry.notes = "Test notes";

    ASSERT_TRUE(
        vaultDatabase.add(entry)
    );

    const auto result =
        vaultDatabase.get(entry.id);

    ASSERT_TRUE(result.has_value());

    EXPECT_EQ(
        result->password,
        "SuperSecret123!"
    );
}

TEST(VaultDatabaseTest, AddThrowsWhenVaultIsLocked)
{
    const char* databaseName = "add_locked_test.db";

    std::remove(databaseName);

    SqliteDatabase sqliteDb{databaseName};
    Vault vault{sqliteDb};

    // Deliberately DO NOT call vault.create() or vault.unlock().
    VaultDatabase db{sqliteDb, vault};

    VaultEntry entry{
        0,
        "My Website",
        "andy",
        "secret123",
        "https://example.com",
        "Some notes"
    };

    EXPECT_THROW(
        db.add(entry),
        std::runtime_error
    );

    std::remove(databaseName);
}

TEST(VaultDatabaseTest, UpdateThrowsWhenVaultIsLocked)
{
    const char* databaseName = "update_locked_test.db";

    std::remove(databaseName);

    SqliteDatabase sqliteDb{databaseName};
    Vault vault{sqliteDb};

    vault.create("test-master-password");

    VaultDatabase db{sqliteDb, vault};

    VaultEntry entry{
        0,
        "Original Title",
        "andy",
        "original-password",
        "https://example.com",
        "Original notes"
    };

    ASSERT_TRUE(db.add(entry));
    ASSERT_GT(entry.id, 0);

    // Now lock the vault.
    vault.lock();

    entry.password = "new-password";

    EXPECT_THROW(
        db.update(entry),
        std::runtime_error
    );

    std::remove(databaseName);
}

TEST(VaultDatabaseTest, GetThrowsWhenVaultIsLocked)
{
    const char* databaseName = "get_locked_test.db";
    std::remove(databaseName);

    SqliteDatabase sqliteDb{databaseName};
    Vault vault{sqliteDb};

    vault.create("test-master-password");

    VaultDatabase db{sqliteDb, vault};

    VaultEntry entry{
        0,
        "My Website",
        "andy",
        "secret123",
        "https://example.com",
        "Some notes"
    };

    ASSERT_TRUE(db.add(entry));
    ASSERT_GT(entry.id, 0);

    // Lock the vault after the entry has been stored.
    vault.lock();

    // Reading requires decrypting the password, so it must fail.
    EXPECT_THROW(
        db.get(entry.id),
        std::runtime_error
    );

    std::remove(databaseName);
}

TEST(VaultDatabaseTest, GetAllThrowsWhenVaultIsLocked)
{
    const char* databaseName = "get_all_locked_test.db";
    std::remove(databaseName);

    SqliteDatabase sqliteDb{databaseName};
    Vault vault{sqliteDb};

    vault.create("test-master-password");

    VaultDatabase db{sqliteDb, vault};

    VaultEntry entry1{
        0,
        "Website One",
        "andy",
        "password-one",
        "https://one.example.com",
        "Notes one"
    };

    VaultEntry entry2{
        0,
        "Website Two",
        "andy",
        "password-two",
        "https://two.example.com",
        "Notes two"
    };

    ASSERT_TRUE(db.add(entry1));
    ASSERT_TRUE(db.add(entry2));

    // The entries exist, but the vault is now locked.
    vault.lock();

    // getAll() must decrypt each password, so it must fail.
    EXPECT_THROW(
        db.getAll(),
        std::runtime_error
    );

    std::remove(databaseName);
}