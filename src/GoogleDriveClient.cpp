
#include "GoogleDriveClient.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTemporaryFile>
#include <QUrl>
#include <QUrlQuery>
#include <QUuid>
#include <QDir>

namespace
{
    constexpr auto DRIVE_API =
        "https://www.googleapis.com/drive/v3";

    constexpr auto DRIVE_UPLOAD_API =
        "https://www.googleapis.com/upload/drive/v3";

    constexpr auto FOLDER_NAME = "PasswordManager";
    constexpr auto BACKUP_NAME = "PasswordManagerBackup.db";

    constexpr auto FOLDER_MIME_TYPE =
        "application/vnd.google-apps.folder";

    QNetworkRequest authorizedRequest(
        const QUrl& url,
        const QString& accessToken)
    {
        QNetworkRequest request(url);

        request.setRawHeader(
            "Authorization",
            QByteArray("Bearer ") + accessToken.toUtf8());

        return request;
    }

    QString replyError(QNetworkReply* reply)
    {
        const QByteArray body = reply->readAll();

        if (!body.isEmpty()) {
            const QJsonDocument document =
                QJsonDocument::fromJson(body);

            if (document.isObject()) {
                const QJsonObject error =
                    document.object().value("error").toObject();

                const QString message =
                    error.value("message").toString();

                if (!message.isEmpty())
                    return message;
            }

            return QString::fromUtf8(body);
        }

        return reply->errorString();
    }

    QUrl filesUrl(const QString& searchQuery)
    {
        QUrl url(QString(DRIVE_API) + "/files");

        QUrlQuery query;
        query.addQueryItem("q", searchQuery);
        query.addQueryItem("fields", "files(id,name,mimeType)");
        query.addQueryItem("pageSize", "100");

        url.setQuery(query);

        return url;
    }

    QString escapeDriveQuery(const QString& value)
    {
        QString result = value;
        result.replace("\\", "\\\\");
        result.replace("'", "\\'");
        return result;
    }

    bool isValidSqliteDatabase(const QString& path)
    {
        sqlite3* database = nullptr;

        const QByteArray filename = path.toUtf8();

        if (sqlite3_open_v2(
                filename.constData(),
                &database,
                SQLITE_OPEN_READONLY,
                nullptr) != SQLITE_OK) {
            if (database)
                sqlite3_close(database);

            return false;
        }

        sqlite3_stmt* statement = nullptr;

        const int prepareResult = sqlite3_prepare_v2(
            database,
            "PRAGMA quick_check",
            -1,
            &statement,
            nullptr);

        bool valid = false;

        if (prepareResult == SQLITE_OK &&
            sqlite3_step(statement) == SQLITE_ROW) {

            const auto* result =
                reinterpret_cast<const char*>(
                    sqlite3_column_text(statement, 0));

            valid = result &&
                    QString::fromUtf8(result) == "ok";
        }

        if (statement)
            sqlite3_finalize(statement);

        sqlite3_close(database);

        return valid;
    }
}

GoogleDriveClient::GoogleDriveClient(QObject* parent)
    : QObject(parent)
{
}

QString GoogleDriveClient::createDatabaseSnapshot(
    sqlite3* sourceDatabase,
    QString& errorMessage)
{
    if (!sourceDatabase) {
        errorMessage = "The source SQLite database is not open.";
        return {};
    }

    QTemporaryFile temporaryFile(
        QDir::tempPath() + "/PasswordManagerBackup_XXXXXX.db");

    temporaryFile.setAutoRemove(false);

    if (!temporaryFile.open()) {
        errorMessage = "Could not create temporary backup file.";
        return {};
    }

    const QString path = temporaryFile.fileName();
    temporaryFile.close();

    sqlite3* destinationDatabase = nullptr;

    const QByteArray destinationPath = path.toUtf8();

    if (sqlite3_open(
            destinationPath.constData(),
            &destinationDatabase) != SQLITE_OK) {

        errorMessage = destinationDatabase
            ? QString::fromUtf8(
                  sqlite3_errmsg(destinationDatabase))
            : "Could not open temporary SQLite database.";

        if (destinationDatabase)
            sqlite3_close(destinationDatabase);

        QFile::remove(path);
        return {};
    }

    sqlite3_backup* backup = sqlite3_backup_init(
        destinationDatabase,
        "main",
        sourceDatabase,
        "main");

    if (!backup) {
        errorMessage =
            QString::fromUtf8(
                sqlite3_errmsg(destinationDatabase));

        sqlite3_close(destinationDatabase);
        QFile::remove(path);
        return {};
    }

    int result = SQLITE_OK;

    do {
        result = sqlite3_backup_step(backup, 100);

        if (result == SQLITE_BUSY ||
            result == SQLITE_LOCKED) {
            sqlite3_sleep(10);
        }

    } while (result == SQLITE_OK ||
             result == SQLITE_BUSY ||
             result == SQLITE_LOCKED);

    const int finishResult = sqlite3_backup_finish(backup);

    const int closeResult =
        sqlite3_close(destinationDatabase);

    if (result != SQLITE_DONE ||
        finishResult != SQLITE_OK ||
        closeResult != SQLITE_OK) {

        errorMessage =
            "SQLite could not complete the database snapshot.";

        QFile::remove(path);
        return {};
    }

    return path;
}

void GoogleDriveClient::backupDatabase(
    sqlite3* sourceDatabase,
    const QString& accessToken)
{
    if (accessToken.isEmpty()) {
        emit backupFinished(
            false,
            "Google access token is empty.");
        return;
    }

    cleanupSnapshot();

    accessToken_ = accessToken;

    QString errorMessage;

    snapshotPath_ =
        createDatabaseSnapshot(sourceDatabase, errorMessage);

    if (snapshotPath_.isEmpty()) {
        emit backupFinished(false, errorMessage);
        return;
    }

    findBackupFolder();
}

void GoogleDriveClient::findBackupFolder()
{
    const QString query =
        QString("name = '%1' and mimeType = '%2' and trashed = false")
            .arg(FOLDER_NAME)
            .arg(FOLDER_MIME_TYPE);

    QNetworkReply* reply = networkManager_.get(
        authorizedRequest(
            filesUrl(query),
            accessToken_));

    connect(reply, &QNetworkReply::finished, this,
            [this, reply]() {

        if (reply->error() != QNetworkReply::NoError) {
            const QString message = replyError(reply);
            reply->deleteLater();

            cleanupSnapshot();
            emit backupFinished(false, message);
            return;
        }

        const QJsonDocument document =
            QJsonDocument::fromJson(reply->readAll());

        reply->deleteLater();

        const QJsonArray files =
            document.object().value("files").toArray();

        if (!files.isEmpty()) {
            folderId_ =
                files.first().toObject().value("id").toString();

            findBackupFile();
        } else {
            createBackupFolder();
        }
    });
}

void GoogleDriveClient::createBackupFolder()
{
    QUrl url(QString(DRIVE_API) + "/files");

    QNetworkRequest request =
        authorizedRequest(url, accessToken_);

    request.setHeader(
        QNetworkRequest::ContentTypeHeader,
        "application/json");

    const QJsonObject metadata{
        {"name", FOLDER_NAME},
        {"mimeType", FOLDER_MIME_TYPE}
    };

    QNetworkReply* reply = networkManager_.post(
        request,
        QJsonDocument(metadata).toJson());

    connect(reply, &QNetworkReply::finished, this,
            [this, reply]() {

        if (reply->error() != QNetworkReply::NoError) {
            const QString message = replyError(reply);
            reply->deleteLater();

            cleanupSnapshot();
            emit backupFinished(false, message);
            return;
        }

        const QJsonDocument document =
            QJsonDocument::fromJson(reply->readAll());

        folderId_ =
            document.object().value("id").toString();

        reply->deleteLater();

        if (folderId_.isEmpty()) {
            cleanupSnapshot();
            emit backupFinished(
                false,
                "Google Drive did not return a folder ID.");
            return;
        }

        findBackupFile();
    });
}

void GoogleDriveClient::findBackupFile()
{
    const QString query =
        QString("name = '%1' and '%2' in parents and trashed = false")
            .arg(BACKUP_NAME)
            .arg(escapeDriveQuery(folderId_));

    QNetworkReply* reply = networkManager_.get(
        authorizedRequest(
            filesUrl(query),
            accessToken_));

    connect(reply, &QNetworkReply::finished, this,
            [this, reply]() {

        if (reply->error() != QNetworkReply::NoError) {
            const QString message = replyError(reply);
            reply->deleteLater();

            cleanupSnapshot();
            emit backupFinished(false, message);
            return;
        }

        const QJsonDocument document =
            QJsonDocument::fromJson(reply->readAll());

        const QJsonArray files =
            document.object().value("files").toArray();

        if (!files.isEmpty()) {
            backupFileId_ =
                files.first().toObject().value("id").toString();
        } else {
            backupFileId_.clear();
        }

        reply->deleteLater();

        uploadBackup();
    });
}

void GoogleDriveClient::uploadBackup()
{
    QFile file(snapshotPath_);

    if (!file.open(QIODevice::ReadOnly)) {
        cleanupSnapshot();
        emit backupFinished(
            false,
            "Could not read the temporary database snapshot.");
        return;
    }

    const QByteArray databaseContents = file.readAll();
    file.close();

    if (databaseContents.isEmpty()) {
        cleanupSnapshot();
        emit backupFinished(
            false,
            "The database snapshot is empty.");
        return;
    }

    QNetworkReply* reply = nullptr;

    if (!backupFileId_.isEmpty()) {
        // Replace the contents of the existing Drive file.
        QUrl url(
            QString(DRIVE_UPLOAD_API) +
            "/files/" + backupFileId_);

        QUrlQuery query;
        query.addQueryItem("uploadType", "media");
        url.setQuery(query);

        QNetworkRequest request =
            authorizedRequest(url, accessToken_);

        request.setHeader(
            QNetworkRequest::ContentTypeHeader,
            "application/octet-stream");

        reply = networkManager_.sendCustomRequest(
            request,
            "PATCH",
            databaseContents);
    } else {
        // Create a new Drive file using multipart upload.
        const QString boundary =
            "PasswordManager_" +
            QUuid::createUuid().toString(QUuid::WithoutBraces);

        QJsonObject metadata;
        metadata.insert("name", BACKUP_NAME);
        metadata.insert(
            "parents",
            QJsonArray{folderId_});

        const QByteArray boundaryBytes =
            boundary.toUtf8();

        QByteArray body;

        body += "--" + boundaryBytes + "\r\n";
        body += "Content-Type: application/json; charset=UTF-8\r\n\r\n";
        body += QJsonDocument(metadata).toJson();
        body += "\r\n--" + boundaryBytes + "\r\n";
        body += "Content-Type: application/octet-stream\r\n\r\n";
        body += databaseContents;
        body += "\r\n--" + boundaryBytes + "--";

        QUrl url(QString(DRIVE_UPLOAD_API) + "/files");

        QUrlQuery query;
        query.addQueryItem("uploadType", "multipart");
        query.addQueryItem("fields", "id,name");
        url.setQuery(query);

        QNetworkRequest request =
            authorizedRequest(url, accessToken_);

        request.setHeader(
            QNetworkRequest::ContentTypeHeader,
            "multipart/related; boundary=" + boundary);

        reply = networkManager_.post(request, body);
    }

    connect(reply, &QNetworkReply::finished, this,
            [this, reply]() {

        if (reply->error() != QNetworkReply::NoError) {
            const QString message = replyError(reply);
            reply->deleteLater();

            cleanupSnapshot();
            emit backupFinished(false, message);
            return;
        }

        reply->deleteLater();

        cleanupSnapshot();

        emit backupFinished(
            true,
            "Google Drive backup completed successfully.");
    });
}

void GoogleDriveClient::downloadBackup(
    const QString& accessToken)
{
    if (accessToken.isEmpty()) {
        emit restoreFailed(
            "Google access token is empty.");
        return;
    }

    accessToken_ = accessToken;

    const QString query =
        QString("name = '%1' and mimeType = '%2' and trashed = false")
            .arg(FOLDER_NAME)
            .arg(FOLDER_MIME_TYPE);

    QNetworkReply* reply = networkManager_.get(
        authorizedRequest(
            filesUrl(query),
            accessToken_));

    connect(reply, &QNetworkReply::finished, this,
            [this, reply]() {

        if (reply->error() != QNetworkReply::NoError) {
            const QString message = replyError(reply);
            reply->deleteLater();

            emit restoreFailed(message);
            return;
        }

        const QJsonDocument document =
            QJsonDocument::fromJson(reply->readAll());

        const QJsonArray files =
            document.object().value("files").toArray();

        reply->deleteLater();

        if (files.isEmpty()) {
            emit restoreFailed(
                "The PasswordManager folder was not found in Google Drive.");
            return;
        }

        folderId_ =
            files.first().toObject().value("id").toString();

        const QString fileQuery =
            QString("name = '%1' and '%2' in parents and trashed = false")
                .arg(BACKUP_NAME)
                .arg(escapeDriveQuery(folderId_));

        QNetworkReply* fileReply = networkManager_.get(
            authorizedRequest(
                filesUrl(fileQuery),
                accessToken_));

        connect(fileReply, &QNetworkReply::finished, this,
                [this, fileReply]() {

            if (fileReply->error() != QNetworkReply::NoError) {
                const QString message = replyError(fileReply);
                fileReply->deleteLater();

                emit restoreFailed(message);
                return;
            }

            const QJsonDocument fileDocument =
                QJsonDocument::fromJson(fileReply->readAll());

            const QJsonArray backupFiles =
                fileDocument.object().value("files").toArray();

            fileReply->deleteLater();

            if (backupFiles.isEmpty()) {
                emit restoreFailed(
                    "No PasswordManager backup was found in Google Drive.");
                return;
            }

            backupFileId_ =
                backupFiles.first().toObject().value("id").toString();

            downloadBackupFile();
        });
    });
}

void GoogleDriveClient::downloadBackupFile()
{
    QUrl url(
        QString(DRIVE_API) +
        "/files/" + backupFileId_);

    QUrlQuery query;
    query.addQueryItem("alt", "media");
    url.setQuery(query);

    QNetworkReply* reply = networkManager_.get(
        authorizedRequest(url, accessToken_));

    connect(reply, &QNetworkReply::finished, this,
            [this, reply]() {

        if (reply->error() != QNetworkReply::NoError) {
            const QString message = replyError(reply);
            reply->deleteLater();

            emit restoreFailed(message);
            return;
        }

        const QByteArray contents = reply->readAll();
        reply->deleteLater();

        if (contents.isEmpty()) {
            emit restoreFailed(
                "The downloaded backup is empty.");
            return;
        }

        QTemporaryFile temporaryFile(
            QDir::tempPath() +
            "/PasswordManagerRestore_XXXXXX.db");

        temporaryFile.setAutoRemove(false);

        if (!temporaryFile.open()) {
            emit restoreFailed(
                "Could not create temporary restore file.");
            return;
        }

        const QString path = temporaryFile.fileName();

        const qint64 bytesWritten =
            temporaryFile.write(contents);

        temporaryFile.close();

        if (bytesWritten != contents.size()) {
            QFile::remove(path);

            emit restoreFailed(
                "Could not write the downloaded backup.");
            return;
        }

        if (!isValidSqliteDatabase(path)) {
            QFile::remove(path);

            emit restoreFailed(
                "The downloaded file is not a valid SQLite database.");
            return;
        }

        emit restoreDownloaded(path);
    });
}

void GoogleDriveClient::cleanupSnapshot()
{
    if (!snapshotPath_.isEmpty()) {
        QFile::remove(snapshotPath_);
        snapshotPath_.clear();
    }
}
