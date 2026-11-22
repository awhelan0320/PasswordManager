# Password Manager

## Purpose

Password Manager is a desktop password vault built with C++20 and Qt 6.
It stores vault entries in a local SQLite database, encrypts only entry
password values with libsodium, protects the encryption key behind a master
password, and optionally creates a backup of the database in Google Drive.

The application is organized as a small Qt desktop application with separate
classes for presentation, vault state, SQLite access, cryptography, application
configuration, and Google integration.

## Goals

- Keep entry passwords encrypted at rest.
- Derive a vault-specific encryption key from a user-provided master password.
- Verify an attempted master password without storing the password itself.
- Clear the derived encryption key when the vault is locked.
- Provide add, view, update, and delete operations for password entries.
- Persist application and Google OAuth settings in an INI-format configuration
  file.
- Support encrypted-vault database snapshots in Google Drive.
- Keep cryptography and SQLite operations behind focused C++ classes.

## Non-goals

- This is not a browser-extension or cross-device synchronization product.
- The vault does not provide account recovery for a forgotten master password.
- The Google Drive backup is a database snapshot, not a live synchronization
  protocol.
- The current design does not provide database migration/versioning tooling,
  multiple vault profiles, sharing, or an audit log.
- The current UI does not expose a restore workflow, even though
  `GoogleDriveClient` contains a download-and-validate implementation.
- The current design does not treat fields other than `password` as encrypted.
  Titles, usernames, URLs, and notes are stored as regular SQLite text.

## Architecture

The application starts in `PasswordManagerApp.cpp`: libsodium is initialized,
Qt is initialized, application organization metadata is set, and
`MainWindow` is constructed.

```text
Qt widgets and dialogs
  (MainWindow, EntryDialog, CreateVaultDialog, UnlockDialog)
                  |
                  v
Vault coordination
  (Vault, MasterKey, VaultVerifier, VaultCrypto)
                  |
                  v
Vault persistence
  (VaultDatabase, VaultMetadata, SqliteDatabase)
                  |
                  v
SQLite database

Google integration
  (OAuthManager -> GoogleTokenClient -> GoogleDriveClient)
                  |
                  v
Google OAuth and Google Drive APIs
```

The top-level composition is owned by `MainWindow`. It opens the configured
database, owns a `Vault`, and constructs `VaultDatabase` with references to both
the database handle and vault. This composition makes entry operations depend on
the vault being unlocked: database reads and writes call back into `Vault`
before a password can be decrypted or encrypted.

## Components

### MainWindow

`MainWindow` is the application coordinator and primary Qt presentation layer.
It creates menus and actions, constructs the table of vault entries, starts
vault initialization, enables or disables vault actions based on vault state,
and coordinates Google authorization and backup operations.

Its main responsibilities are:

- Prompting for vault creation or unlock on startup.
- Showing the title, username, and URL columns in a `QTableWidget`.
- Routing add, edit, and delete actions to `VaultDatabase`.
- Locking the vault and clearing displayed entries.
- Starting or refreshing Google OAuth credentials when required.
- Triggering a Google Drive database backup.

The password field is never included as a table column. Existing entries are
decrypted by `VaultDatabase` before populating the entry edit dialog, while the
main table only displays title, username, and URL.

### Vault

`Vault` owns the in-memory encryption key and the unlocked state. It coordinates
`VaultMetadata`, `MasterKey`, `VaultVerifier`, and `VaultCrypto`.

Creation:

1. Initialize and create the vault metadata table if necessary.
2. Generate a new random salt.
3. Derive a 32-byte key from the master password and salt.
4. Create an authenticated verification record.
5. Persist the salt and verification record.
6. Keep the derived key in memory and mark the vault unlocked.

Unlock:

1. Read the persisted salt and verification record.
2. Derive a candidate key from the supplied master password.
3. Authenticate/decrypt the verification record with that key.
4. Keep the key only if verification succeeds.
5. Set the unlocked state to true for a successful unlock.

Locking calls `sodium_memzero` on the 32-byte key before clearing the unlocked
flag. Encrypt and decrypt operations throw if called while locked.

### VaultMetadata

`VaultMetadata` manages the `vault_metadata` SQLite table. It stores:

- A random 16-byte key derivation salt.
- A random 24-byte verification nonce.
- An authenticated verification ciphertext.

The salt is public/non-secret metadata, but it must remain unchanged once
entries exist because changing it changes the derived key and prevents existing
passwords from being decrypted. The verification record allows the application
to validate a master password without storing a plaintext password or a
recoverable password hash.

### MasterKey

`MasterKey` generates random salts and derives 32-byte keys with libsodium
Argon2id:

- Algorithm: `crypto_pwhash_ALG_ARGON2ID13`
- Salt size: 16 bytes
- Derived key size: 32 bytes
- Limits: libsodium moderate operation and memory limits

The same password and stored salt deterministically produce the same key.

### VaultVerifier

`VaultVerifier` creates and checks an encrypted verification marker containing
the text `PasswordManager vault verification`. The marker is encrypted with
XChaCha20-Poly1305 using the derived key.

Verification is a constant-time plaintext comparison via `sodium_memcmp`.
A decryption/authentication failure or mismatched plaintext is reported as an
incorrect master password.

### VaultCrypto

`VaultCrypto` encrypts and decrypts individual password strings with
libsodium's XChaCha20-Poly1305-IETF authenticated encryption:

- A fresh random nonce is generated for every encryption operation.
- The nonce is stored as a prefix of the ciphertext blob.
- The authentication tag is included in the encrypted blob.
- No associated data is used.
- Decryption returns an error for blobs that are too short or fail
  authentication.

The nonce prefix and authentication tag make reused values produce different
stored blobs and allow tampering to be detected before plaintext is returned.

## Database

### Database location

The database path is read from `AppConfig`. By default, configuration is stored
at `~/.config/PasswordManager/PasswordManager.conf`; this can be replaced by
setting `PASSWORDMANAGER_CONFIG` to an alternate configuration file path.

The checked-in configuration currently identifies a database path but should be
treated as environment-specific. OAuth credentials and tokens must not be
committed to source control.

### SQLite wrapper

`SqliteDatabase` is an RAII wrapper around `sqlite3*`. Construction opens or
creates the configured file and the destructor closes the handle. Copying is
disabled and an open failure is reported as `std::runtime_error`.

`SqliteStatement` provides RAII finalization for prepared statements so callers
do not need to manage statement cleanup directly.

### Tables

`vault_metadata`:

```sql
CREATE TABLE IF NOT EXISTS vault_metadata (
    id INTEGER PRIMARY KEY,
    salt BLOB NOT NULL,
    verification_nonce BLOB NOT NULL,
    verification_ciphertext BLOB NOT NULL
);
```

The metadata record uses ID `1`.

`vault_entries`:

```sql
CREATE TABLE IF NOT EXISTS vault_entries (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    title TEXT NOT NULL,
    username TEXT NOT NULL,
    password BLOB NOT NULL,
    url TEXT,
    notes TEXT
);
```

Only `password` receives application-level encryption. The entry ID is assigned
by SQLite. URL and notes columns may be nullable; title and username are required
by the schema.

### VaultDatabase

`VaultDatabase` is the entry repository.

- `add` encrypts the entry password and inserts title, username, ciphertext,
  URL, and notes.
- `get` selects one entry and decrypts its password.
- `getAll` retrieves entries ordered by ID and decrypts each password.
- `update` encrypts the complete replacement password and updates one matching
  row.
- `remove` deletes one matching row and reports whether one row changed.

The repository throws `std::runtime_error` for binding or statement errors.
Encrypted password blobs are rejected if they are missing or too small.

## User flows

### Startup

1. `MainWindow` creates a `SqliteDatabase` using the configured database path.
2. It creates `Vault` and `VaultDatabase` around that handle.
3. `VaultMetadata::initialize` ensures the metadata table exists.
4. If metadata does not exist, `CreateVaultDialog` asks for a master password
   twice and calls `Vault::create`.
5. If metadata exists, `UnlockDialog` asks for the master password and calls
   `Vault::unlock`.
6. Canceling startup closes the window.
7. Once unlocked, entry actions and Google Drive backup are enabled and entries
   are loaded into the table.

### Add, edit, and delete

`EntryDialog` collects title, username, password, URL, and notes. Adding passes
a new `VaultEntry` to `VaultDatabase::add`. Editing first retrieves and
decrypts the selected entry, allows the user to modify it, and then updates the
row. Deleting requires confirmation in `MainWindow`.

Database exceptions are surfaced in Qt message boxes. Successful mutations
reload the table.

### Lock and unlock

Locking zeroes the in-memory derived key, clears the table, disables entry and
backup actions, and opens `UnlockDialog`. Canceling the unlock dialog closes the
application. A successful unlock restores actions and reloads entries.

## Configuration

`AppConfig` uses `QSettings` and maintains an INI-format file. Configuration
keys include:

- `Database/path`: SQLite database filename.
- `GoogleOAuth/ClientId`: Google OAuth client identifier.
- `GoogleOAuth/ClientSecret`: Google OAuth client secret.
- `GoogleOAuth/AccessToken`: current access token.
- `GoogleOAuth/RefreshToken`: refresh token.
- `GoogleOAuth/TokenType`: token type, normally `Bearer`.
- `GoogleOAuth/ExpiresAt`: UTC access-token expiration.
- `GoogleOAuth/Scope`: OAuth scope.

The default path is
`~/.config/PasswordManager/PasswordManager.conf`.
`PASSWORDMANAGER_CONFIG` overrides that path.

OAuth values and the client secret are credentials. The configuration file
should have restrictive user-only permissions, and a release build should not
ship real credentials. The refresh-token preservation behavior in
`AppConfig::saveOAuthTokens` retains an existing refresh token when Google
omits one in a later response.

## Google integration

### OAuth authorization

`OAuthManager` uses an authorization-code flow with PKCE.

1. It listens on a random loopback port on `127.0.0.1`.
2. It generates a random OAuth state value and a PKCE verifier/challenge.
3. It opens Google's authorization URL in the system browser.
4. It validates the HTTP callback path and returned OAuth state.
5. It emits the authorization code, loopback redirect URI, and verifier.

The requested scope is `https://www.googleapis.com/auth/drive.file`.
`GoogleTokenClient` exchanges the authorization code, including the client secret
and PKCE verifier, at Google's token endpoint. It also refreshes access tokens
when a refresh token is available.

`MainWindow`:

- Starts authorization if no refresh token exists.
- Refreshes the access token if the stored token is missing or expired.
- Restarts authorization if Google rejects a stored refresh token.
- Saves successful token responses through `AppConfig`.
- Reattempts a pending backup after authorization or refresh succeeds.

### Google Drive backup

`GoogleDriveClient` creates a consistent SQLite snapshot using
`sqlite3_backup`. It writes the snapshot to a temporary file, then searches
Google Drive for a top-level folder named `PasswordManager`.

Backup behavior:

1. Create a SQLite snapshot of the open source database.
2. Find or create the `PasswordManager` folder.
3. Find `PasswordManagerBackup.db` in that folder.
4. Create the file or replace existing file contents.
5. Delete the temporary snapshot.
6. Emit success or a human-readable failure result.

The application requires the vault to be unlocked before it will start a
backup. Backup data is not a second encryption layer; the snapshot contains the
same encrypted vault database and credentials used locally.

## Security design

- Master passwords are not persisted.
- A random 16-byte salt prevents identical passwords from deriving identical
  keys across vaults.
- Argon2id derives the master key.
- The derived key is retained only while the vault is unlocked.
- Locking zeroes the derived key.
- Password values use XChaCha20-Poly1305-IETF with a random nonce.
- Authentication failures are treated as invalid ciphertext or invalid master
  passwords.
- OAuth state, PKCE, and loopback redirect validation protect the browser
  authorization flow.

The design currently depends on SQLite file permissions and host security.
Metadata and non-password entry fields are not encrypted, and the desktop process
must trust the operating system and user account protections. Clipboard,
password-generator, and automatic-lock policies are not part of the current
implementation.

## Testing

CMake registers GoogleTest targets for the core components:

- `AppConfigTest`
- `GoogleTests`
- `PkceHelperTests`
- `MasterKeyTests`
- `VaultMetadataTest`
- `VaultVerifierTest`
- `VaultCryptoTest`
- `VaultTest`
- `VaultDatabaseTests`

The tests use temporary or explicit test database/configuration paths where
needed and cover key derivation, vault creation/unlock behavior, cryptographic
operations, metadata, SQLite entry operations, OAuth token exchange behavior,
configuration, and PKCE helpers.

Tests can be built and discovered through CMake/CTest. The smallest relevant
CTest suite should be run after changes to a subsystem, with the full suite run
before broader release validation.

## Error handling

Database and crypto classes use exceptions, primarily `std::runtime_error`, for
invalid state or I/O failures. `MainWindow` catches database exceptions and
presents Qt message boxes. Network and OAuth components use Qt signals and
callbacks to report failures to the UI.

Cryptographic invalidity is deliberately reported as a boolean verification
failure at unlock time, rather than exposing an exception as an incorrect
master-password result.

## Future features

- Restore/import UI for a downloaded Google Drive database backup.
- Automatic relock and configurable inactivity timeout.
- Password generator and strength feedback.
- Support for vault export/import and explicit database migration/versioning.
- Encrypted storage of titles and notes, not just password values.
- Stronger configuration protections for OAuth secrets and tokens.
- Multiple vault support and per-vault database paths.
- Backup conflict handling and restore integrity checks.
- Automated UI tests and broader failure-injection testing.