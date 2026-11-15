#pragma once

#include <QMainWindow>

#include "VaultDatabase.h"
#include "Vault.h"
#include "OAuthManager.h"
#include "GoogleTokenClient.h"
#include "GoogleDriveClient.h"

class QTableWidget;
class QAction;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);


private:
    bool initializeVault();
    void setupUi();
    void addEntry();
    void editEntry();
    void deleteEntry();
    void refreshTable();
    void lockVault();
    void ensureValidGoogleToken();
    void updateVaultActions();
    void startGoogleDriveBackup();

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
    // QAction* lockVaultAction_;
    // QAction* addEntryAction_;
    // QAction* updateEntryAction_;
    // QAction* deleteEntryAction_;

    QAction* lockVaultAction_{nullptr};
    QAction* addEntryAction_{nullptr};
    QAction* updateEntryAction_{nullptr};
    QAction* deleteEntryAction_{nullptr};

};