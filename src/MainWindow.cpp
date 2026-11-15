#include "MainWindow.h"

#include "EntryDialog.h"
#include "CreateVaultDialog.h"
#include "UnlockDialog.h"
#include "VaultMetadata.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QTimer>   
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include "AppConfig.h"
#include <iostream>

// MainWindow::MainWindow(QWidget* parent)
//     : QMainWindow(parent),
//     sqlite_{AppConfig::getInstance().getDatabasePath().toStdString()},
//     vault_{sqlite_},
//     database_{sqlite_, vault_}
// {
//     qDebug() << "MainWindow constructor started";
//     setupUi();

//     auto* vaultMenu = menuBar()->addMenu("Vault");

//     //auto* lockAction = vaultMenu->addAction("Lock Vault");
//     lockVaultAction_ = vaultMenu->addAction("Lock Vault");
    
//     connect(
//         //lockAction,
//         lockVaultAction_,
//         &QAction::triggered,
//         this,
//         &MainWindow::lockVault
//     );
//     qDebug() << "About to initialize vault";
//     if (!initializeVault())
//     {
//         QTimer::singleShot(0, this, &QWidget::close);
//         return;
//     }
//     qDebug() << "Vault initialization completed";
//     updateVaultActions();

//     refreshTable();

//         auto* driveMenu = menuBar()->addMenu("Google Drive");

//         backupDriveAction_ = driveMenu->addAction("Backup to Google Drive");

//         connect(
//             backupDriveAction_,
//             &QAction::triggered,
//             this,
//             &MainWindow::startGoogleDriveBackup
//         );



        

// connect(&oauthManager_,
//         &OAuthManager::authorizationFailed,
//         this,
// [this](const QString& message)
//         {
//             qWarning() << "Google authorization failed:" << message;

//             googleAuthInProgress_ = false;

//             if (backupRequested_) {
//                 backupRequested_ = false;

//                 QMessageBox::critical(
//                     this,
//                     "Google Authorization Failed",
//                     message
//                 );
//             }
//         });

// // connect(&oauthManager_,
// //         &OAuthManager::authorizationCodeReceived,
// //         this,
// //         [](const QString&, const QString&, const QString&)
// //         {
// //             qInfo() << "Google authorization callback received.";
// //         });
                  
// connect(&oauthManager_,
//         &OAuthManager::authorizationCodeReceived,
//         this,
//         [this](const QString& code,
//                const QString& redirectUri,
//                const QString& verifier)
//         {
//             googleAuthInProgress_ = true;
//             tokenClient_.exchangeAuthorizationCode(
//                 AppConfig::getInstance().getGoogleOAuthClientId(),
//                 AppConfig::getInstance().getGoogleOAuthClientSecret(),
//                 code,
//                 redirectUri,
//                 verifier);
//         });

// // connect(&tokenClient_,
// //         &GoogleTokenClient::tokenReceived,
// //         this,
// //         [](const OAuthToken& token)
// //         {
// //             qInfo() << "Google token exchange successful.";
// //             qInfo() << "Access token received:"
// //                     << !token.accessToken.isEmpty();
// //             qInfo() << "Refresh token received:"
// //                     << !token.refreshToken.isEmpty();
// //             qInfo() << "Token expires:"
// //                     << token.expiresAt;
// //         });


//         connect(&tokenClient_,
//         &GoogleTokenClient::tokenReceived,
//         this,
//         [this](const OAuthToken& token)
//         {
//             const bool saved =
//                 AppConfig::getInstance().saveOAuthTokens(token);

//             if (!saved)
//             {
//                 qWarning() << "Failed to save Google OAuth tokens.";
//                 return;
//             }

//             qInfo() << "Google OAuth tokens saved successfully.";
//             qInfo() << "Access token received:"
//                     << !token.accessToken.isEmpty();
//             qInfo() << "Refresh token received:"
//                     << !token.refreshToken.isEmpty();
//             qInfo() << "Token expires:" << token.expiresAt;

//             googleAuthInProgress_ = false;

//             if (backupRequested_) {
//                 backupRequested_ = false;
//                 startGoogleDriveBackup();
//             }            
//         });

//         connect(
//             &driveClient_,
//             &GoogleDriveClient::backupFinished,
//             this,
//             [this](bool success, const QString& message)
//             {
//                 if (success) {
//                     QMessageBox::information(
//                         this,
//                         "Google Drive Backup",
//                         message
//                     );
//                 } else {
//                     QMessageBox::critical(
//                         this,
//                         "Google Drive Backup Failed",
//                         message
//                     );
//                 }
//             }
//         );

//         connect(&tokenClient_,
//         &GoogleTokenClient::tokenExchangeFailed,
//         this,
//         [this](const QString& message)
//         {
//             qWarning() << "Google token exchange failed:"
//                        << message;

//             googleAuthInProgress_ = false;

//             if (backupRequested_) {
//                 backupRequested_ = false;

//                 QMessageBox::critical(
//                     this,
//                     "Google Authorization Failed",
//                     message
//                 );
//             }
//         });

//         //oauthManager_.startAuthorization(AppConfig::getInstance().getGoogleOAuthClientId());        
//         ensureValidGoogleToken();
// }


MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      sqlite_{AppConfig::getInstance().getDatabasePath().toStdString()},
      vault_{sqlite_},
      database_{sqlite_, vault_}
{
    qDebug() << "MainWindow constructor started";

    // -------------------------------------------------
    // 1. Build the main window UI
    // -------------------------------------------------
    setupUi();

    // -------------------------------------------------
    // 2. Create menus and actions
    // -------------------------------------------------
    auto* vaultMenu = menuBar()->addMenu("Vault");

    lockVaultAction_ = vaultMenu->addAction("Lock Vault");

    connect(
        lockVaultAction_,
        &QAction::triggered,
        this,
        &MainWindow::lockVault
    );

    auto* driveMenu = menuBar()->addMenu("Google Drive");

    backupDriveAction_ =
        driveMenu->addAction("Backup to Google Drive");

    connect(
        backupDriveAction_,
        &QAction::triggered,
        this,
        &MainWindow::startGoogleDriveBackup
    );

    // -------------------------------------------------
    // 3. Connect OAuth signals
    // -------------------------------------------------
    connect(
        &oauthManager_,
        &OAuthManager::authorizationFailed,
        this,
        [this](const QString& message)
        {
            qWarning() << "Google authorization failed:" << message;

            googleAuthInProgress_ = false;

            if (backupRequested_)
            {
                backupRequested_ = false;

                QMessageBox::critical(
                    this,
                    "Google Authorization Failed",
                    message
                );
            }
        }
    );

    connect(
        &oauthManager_,
        &OAuthManager::authorizationCodeReceived,
        this,
        [this](
            const QString& code,
            const QString& redirectUri,
            const QString& verifier)
        {
            googleAuthInProgress_ = true;

            tokenClient_.exchangeAuthorizationCode(
                AppConfig::getInstance().getGoogleOAuthClientId(),
                AppConfig::getInstance().getGoogleOAuthClientSecret(),
                code,
                redirectUri,
                verifier
            );
        }
    );

    connect(
        &tokenClient_,
        &GoogleTokenClient::tokenReceived,
        this,
        [this](const OAuthToken& token)
        {
            const bool saved =
                AppConfig::getInstance().saveOAuthTokens(token);

            if (!saved)
            {
                googleAuthInProgress_ = false;
                backupRequested_ = false;

                qWarning() << "Failed to save Google OAuth tokens.";

                QMessageBox::critical(
                    this,
                    "Google Authorization Failed",
                    "Unable to save Google OAuth tokens."
                );

                return;
            }

            qInfo() << "Google OAuth tokens saved successfully.";
            qInfo() << "Access token received:"
                    << !token.accessToken.isEmpty();
            qInfo() << "Refresh token received:"
                    << !token.refreshToken.isEmpty();
            qInfo() << "Token expires:" << token.expiresAt;

            googleAuthInProgress_ = false;

            if (backupRequested_)
            {
                backupRequested_ = false;
                startGoogleDriveBackup();
            }
        }
    );
    // 11/15/2026
    // connect(
    //     &tokenClient_,
    //     &GoogleTokenClient::tokenExchangeFailed,
    //     this,
    //     [this](const QString& message)
    //     {
    //         qWarning() << "Google token exchange failed:" << message;

    //         googleAuthInProgress_ = false;

    //         if (backupRequested_)
    //         {
    //             backupRequested_ = false;

    //             QMessageBox::critical(
    //                 this,
    //                 "Google Authorization Failed",
    //                 message
    //             );
    //         }
    //     }
    // );

connect(
    &tokenClient_,
    &GoogleTokenClient::tokenExchangeFailed,
    this,
    [this](const QString& message)
    {
        qWarning() << "Google authentication failed:" << message;

        googleAuthInProgress_ = false;

        if (!backupRequested_)
            return;

        // A rejected refresh token requires fresh authorization.
        if (message.contains("Bad Request", Qt::CaseInsensitive))
        {
            qWarning() << "Stored Google credentials may be invalid.";

            AppConfig::getInstance().clearOAuthTokens();

            backupRequested_ = true;
            googleAuthInProgress_ = true;

            oauthManager_.startAuthorization(
                AppConfig::getInstance().getGoogleOAuthClientId());

            return;
        }

        backupRequested_ = false;

        QMessageBox::critical(
            this,
            "Google Authorization Failed",
            message
        );
    }
);


    // -------------------------------------------------
    // 4. Connect Google Drive signals
    // -------------------------------------------------
    connect(
        &driveClient_,
        &GoogleDriveClient::backupFinished,
        this,
        [this](bool success, const QString& message)
        {
            if (success)
            {
                QMessageBox::information(
                    this,
                    "Google Drive Backup",
                    message
                );
            }
            else
            {
                QMessageBox::critical(
                    this,
                    "Google Drive Backup Failed",
                    message
                );
            }
        }
    );

    // -------------------------------------------------
    // 5. Initialize the vault
    // -------------------------------------------------
    qDebug() << "About to initialize vault";

    if (!initializeVault())
    {
        qDebug() << "Vault initialization cancelled.";

        QTimer::singleShot(0, this, &QWidget::close);
        return;
    }

    qDebug() << "Vault initialization completed.";

    // -------------------------------------------------
    // 6. Update UI and display vault entries
    // -------------------------------------------------
    qDebug() << "Updating vault actions.";

    updateVaultActions();

    qDebug() << "Refreshing vault table.";

    refreshTable();

    // -------------------------------------------------
    // 7. Handle Google authentication
    // -------------------------------------------------
    qDebug() << "Checking Google authentication.";

    ensureValidGoogleToken();

    qDebug() << "MainWindow constructor completed.";
}



void MainWindow::setupUi()
{
    setWindowTitle("Password Manager");
    resize(800, 500);

    table_ = new QTableWidget(this);

    table_->setColumnCount(3);

    table_->setHorizontalHeaderLabels({
        "Title",
        "Username",
        "URL"
    });

    table_->setColumnWidth(0, 250);
    table_->setColumnWidth(1, 200);
    table_->horizontalHeader()->setStretchLastSection(true);

    table_->verticalHeader()->setVisible(false);

    table_->setSelectionBehavior(
        QAbstractItemView::SelectRows
    );

    table_->setSelectionMode(
        QAbstractItemView::SingleSelection
    );

    table_->setEditTriggers(
        QAbstractItemView::NoEditTriggers
    );

    //auto* addButton = new QPushButton("Add", this);
    addEntryAction_ = menuBar()->addAction("Add Entry");
    updateEntryAction_ = menuBar()->addAction("Update Entry");
    deleteEntryAction_ = menuBar()->addAction("Delete Entry");

    connect(
        addEntryAction_,
        &QAction::triggered,
        this,
        &MainWindow::addEntry
    );

    connect(
        updateEntryAction_,
        &QAction::triggered,
        this,
        &MainWindow::editEntry
    );

    connect(
        deleteEntryAction_,
        &QAction::triggered,
        this,
        &MainWindow::deleteEntry
    );


    auto* buttonLayout = new QHBoxLayout;

    //buttonLayout->addWidget(addButton);
    //buttonLayout->addWidget(editButton);
    //buttonLayout->addWidget(deleteButton);
    auto* layout = new QVBoxLayout;

    layout->addWidget(table_);
    layout->addLayout(buttonLayout);

    auto* centralWidget = new QWidget(this);

    centralWidget->setLayout(layout);

    setCentralWidget(centralWidget);
}


void MainWindow::addEntry()
{
    EntryDialog dialog{this};

    if (dialog.exec() != QDialog::Accepted)
    {
        return;
    }

    VaultEntry entry = dialog.entry();

    try
    {
        database_.add(entry);
    }
    catch (const std::exception& e)
    {
        QMessageBox::critical(
            this,
            "Database Error",
            e.what()
        );

        return;
    }
    // std::cout << "Google OAuth Client ID: " << AppConfig::getInstance().getGoogleOAuthClientId().toStdString() << std::endl;
    refreshTable();
}

void MainWindow::editEntry()
{
    const int row = table_->currentRow();

    if (row < 0)
    {
        return;
    }

    const int id =
        table_->item(row, 0)->data(Qt::UserRole).toInt();

    try
    {
        const auto existingEntry = database_.get(id);

        if (!existingEntry)
        {
            return;
        }

        EntryDialog dialog{*existingEntry, this};

        if (dialog.exec() != QDialog::Accepted)
        {
            return;
        }

        VaultEntry entry = dialog.entry();

        entry.id = existingEntry->id;

        database_.update(entry);
    }
    catch (const std::exception& e)
    {
        QMessageBox::critical(
            this,
            "Database Error",
            e.what()
        );

        return;
    }

    refreshTable();
}

void MainWindow::deleteEntry()
{
    const int row = table_->currentRow();

    if (row < 0)
    {
        return;
    }

    const int id =
        table_->item(row, 0)->data(Qt::UserRole).toInt();

    const QString title =
        table_->item(row, 0)->text();

    const auto result = QMessageBox::question(
        this,
        "Delete Entry",
        "Are you sure you want to delete \"" +
            title +
            "\"?"
    );

    if (result != QMessageBox::Yes)
    {
        return;
    }

    try
    {
        database_.remove(id);
    }
    catch (const std::exception& e)
    {
        QMessageBox::critical(
            this,
            "Database Error",
            e.what()
        );

        return;
    }

    refreshTable();
}

void MainWindow::refreshTable()
{
    const auto entries = database_.getAll();

    table_->setRowCount(
        static_cast<int>(entries.size())
    );

    for (int row = 0;
         row < static_cast<int>(entries.size());
         ++row)
    {
        const auto& entry = entries[row];

        auto* titleItem = new QTableWidgetItem(
            QString::fromStdString(entry.title)
        );

        titleItem->setData(
            Qt::UserRole,
            entry.id
        );

        table_->setItem(
            row,
            0,
            titleItem
        );

        table_->setItem(
            row,
            1,
            new QTableWidgetItem(
                QString::fromStdString(entry.username)
            )
        );

        table_->setItem(
            row,
            2,
            new QTableWidgetItem(
                QString::fromStdString(entry.url)
            )
        );
    }
}

bool MainWindow::initializeVault()
{
    qDebug() << "initializeVault: entered";
    VaultMetadata metadata{sqlite_};
    metadata.initialize();
    qDebug() << "initializeVault: after metadata.initialize()";
    if (!metadata.exists())
    {
        qDebug() << "No metadata found. Prompting user to create a new vault.";   
        CreateVaultDialog dialog{vault_, this};
        qDebug() << "initializeVault: created CreateVaultDialog";  
        if (dialog.exec() != QDialog::Accepted)
            return false;

        return vault_.isUnlocked();
    }
    qDebug() << "initializeVault: vault exists";

    UnlockDialog dialog{vault_, this};
    qDebug() << "initializeVault: created UnlockDialog";

    const int result = dialog.exec();
    
    qDebug() << "Unlock dialog result:" << result;
    qDebug() << "Vault unlocked:" << vault_.isUnlocked();

    if (result != QDialog::Accepted)
        return false;

    qDebug() << "initializeVault: after dialog.exec()";
    return vault_.isUnlocked();
}

void MainWindow::lockVault()
{
    vault_.lock();

    table_->setRowCount(0);

    updateVaultActions();

    UnlockDialog dialog{vault_, this};

    if (dialog.exec() == QDialog::Accepted)
    {
        updateVaultActions();
        refreshTable();
    }
    else
    {
        close();
    }
}

void MainWindow::updateVaultActions()
{
    const bool unlocked = vault_.isUnlocked();

    lockVaultAction_->setEnabled(unlocked);
    addEntryAction_->setEnabled(unlocked);
    updateEntryAction_->setEnabled(unlocked);
    deleteEntryAction_->setEnabled(unlocked);
    backupDriveAction_->setEnabled(unlocked);
}

void MainWindow::ensureValidGoogleToken()
{
    auto& config = AppConfig::getInstance();

    const QString refreshToken = config.getRefreshToken();

    // No refresh token: obtain authorization from Google.
    if (refreshToken.isEmpty())
    {
        qInfo() << "Google authorization required.";

        googleAuthInProgress_ = true;
        
        oauthManager_.startAuthorization(
            config.getGoogleOAuthClientId());

        return;
    }

    // A valid access token can be used as-is.
    if (!config.isAccessTokenExpired())
    {
        qInfo() << "Google access token is valid.";
        return;
    }

    // Access token expired or missing: refresh it.
    qInfo() << "Refreshing Google access token.";

    googleAuthInProgress_ = true;

    tokenClient_.refreshAccessToken(
        config.getGoogleOAuthClientId(),
        config.getGoogleOAuthClientSecret(),
        refreshToken);
}

void MainWindow::startGoogleDriveBackup()
{
    qDebug() << "startGoogleDriveBackup called";
    if (!vault_.isUnlocked()) {
        QMessageBox::warning(
            this,
            "Vault Locked",
            "Unlock the vault before backing it up."
        );
        return;
    }

    auto& config = AppConfig::getInstance();

    // Use the existing token if it is still valid.
    if (!config.isAccessTokenExpired()) {
        driveClient_.backupDatabase(
            sqlite_.get(),
            config.getAccessToken()
        );
        return;
    }

    // Token is expired or missing. Wait for authorization/refresh.
    backupRequested_ = true;

    if (googleAuthInProgress_)
        return;

    googleAuthInProgress_ = true;

    const QString refreshToken = config.getRefreshToken();

    // if (!refreshToken.isEmpty()) {
    //     tokenClient_.refreshAccessToken(
    //         config.getGoogleOAuthClientId(),
    //         config.getGoogleOAuthClientSecret(),
    //         refreshToken
    //     );
    // } else {
    //     oauthManager_.startAuthorization(
    //         config.getGoogleOAuthClientId()
    //     );
    // }

    if (!refreshToken.isEmpty())
    {
        qInfo() << "Attempting to refresh Google access token.";

        tokenClient_.refreshAccessToken(
            config.getGoogleOAuthClientId(),
            config.getGoogleOAuthClientSecret(),
            refreshToken);
    }
    else
    {
        qInfo() << "No refresh token. Starting Google authorization.";

        oauthManager_.startAuthorization(
            config.getGoogleOAuthClientId());
    }
}