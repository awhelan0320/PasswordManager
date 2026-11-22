
#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QString>

#include <sqlite3.h>

/**
 * @brief The GoogleDriveClient class provides functionality to backup and restore a database to/from Google Drive.
 */
class GoogleDriveClient : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructs a GoogleDriveClient with an optional parent QObject.
     * @param parent The parent QObject, default is nullptr.
     *
     * This constructor initializes the GoogleDriveClient, setting up the necessary
     * network access manager for handling HTTP requests to Google Drive.
     */
    explicit GoogleDriveClient(QObject* parent = nullptr);

    // Creates a consistent snapshot and uploads it to Google Drive.
    void backupDatabase(sqlite3* sourceDatabase,
                        const QString& accessToken);

    // Downloads the existing backup to a temporary local file.
    // The caller is responsible for safely installing the restored database.
    void downloadBackup(const QString& accessToken);

signals:
    /**
     * @brief Signal emitted when the backup operation is finished.
     * @param success Indicates whether the backup was successful.
     * @param message Provides additional information about the backup result.
     */
    void backupFinished(bool success, const QString& message);
    /**
     * @brief Signal emitted when the restore operation is finished.
     * @param success Indicates whether the restore was successful.
     * @param message Provides additional information about the restore result.
     */
    void restoreDownloaded(const QString& temporaryFilePath);
    
    /**
     * @brief Signal emitted when the restore operation fails.
     * @param message Provides information about the failure reason.
     */
    void restoreFailed(const QString& message);

private:

    /**
     * @brief Creates a snapshot of the database.
     * @param sourceDatabase The source SQLite database.
     * @param errorMessage Reference to a string to store any error messages.
     * @return The path to the created snapshot file.
     */
    QString createDatabaseSnapshot(sqlite3* sourceDatabase,
                                   QString& errorMessage);

    void findBackupFolder();
    // Creates a backup folder in Google Drive if it doesn't exist.
    void createBackupFolder();
    void findBackupFile();
    // Creates a backup file in Google Drive if it doesn't exist.
    void uploadBackup();

    void downloadBackupFile();

    void cleanupSnapshot();

    QNetworkAccessManager networkManager_;

    QString accessToken_;
    QString folderId_;
    QString backupFileId_;

    QString snapshotPath_;
};
