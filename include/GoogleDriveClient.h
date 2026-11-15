
#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QString>

#include <sqlite3.h>

class GoogleDriveClient : public QObject
{
    Q_OBJECT

public:
    explicit GoogleDriveClient(QObject* parent = nullptr);

    // Creates a consistent snapshot and uploads it to Google Drive.
    void backupDatabase(sqlite3* sourceDatabase,
                        const QString& accessToken);

    // Downloads the existing backup to a temporary local file.
    // The caller is responsible for safely installing the restored database.
    void downloadBackup(const QString& accessToken);

signals:
    void backupFinished(bool success, const QString& message);

    void restoreDownloaded(const QString& temporaryFilePath);

    void restoreFailed(const QString& message);

private:
    QString createDatabaseSnapshot(sqlite3* sourceDatabase,
                                   QString& errorMessage);

    void findBackupFolder();
    void createBackupFolder();

    void findBackupFile();
    void uploadBackup();

    void downloadBackupFile();

    void cleanupSnapshot();

    QNetworkAccessManager networkManager_;

    QString accessToken_;
    QString folderId_;
    QString backupFileId_;

    QString snapshotPath_;
};
