#pragma once

#include <QMainWindow>

#include "VaultDatabase.h"
#include "Vault.h"
#include "OAuthManager.h"
#include "GoogleTokenClient.h"
#include "GoogleDriveClient.h"

/**
 * @brief The MainWindow class represents the main application window for the Password Manager.
 *
 * This class provides the user interface for managing password entries, including
 * adding, editing, deleting, and backing up entries to Google Drive. It handles
 * user interactions and coordinates with the underlying Vault and database.
 */

class QTableWidget;
class QAction;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    /**
     * @brief Constructs the MainWindow with an optional parent widget.
     * @param parent The parent widget, default is nullptr.
     *
     * This constructor initializes the main application window, setting up the user
     * interface elements and connecting signals and slots for user interactions.
     */
    explicit MainWindow(QWidget* parent = nullptr);


private:
    // Initializes the user interface components and layout of the main window.
    bool initializeVault();
    // Sets up the user interface elements, including the table and action buttons.
    void setupUi();
    // Connects signals and slots for user interactions and events.
    void addEntry();
    // Opens a dialog to edit the selected entry in the table.
    void editEntry();
    // Deletes the selected entry from the vault and updates the table.
    void deleteEntry();
    // Refreshes the table to display the current entries in the vault.
    void refreshTable();
    // Locks the vault, requiring re-authentication to access entries.
    void lockVault();
    // Unlocks the vault, allowing access to entries after successful authentication.
    void ensureValidGoogleToken();
    // Updates the state of actions based on the current vault and authentication status.
    void updateVaultActions();
    // Starts the process of backing up the database to Google Drive.
    void startGoogleDriveBackup();

    // Member variables for managing the vault, database, and user interface.
    GoogleDriveClient driveClient_;
    QAction* backupDriveAction_{nullptr};

    bool backupRequested_{false};
    bool googleAuthInProgress_{false};

    SqliteDatabase sqlite_;
    Vault vault_;
    VaultDatabase database_;
    QTableWidget* table_;
    OAuthManager oauthManager_;
    GoogleTokenClient tokenClient_;

    QAction* lockVaultAction_{nullptr};
    QAction* addEntryAction_{nullptr};
    QAction* updateEntryAction_{nullptr};
    QAction* deleteEntryAction_{nullptr};

};