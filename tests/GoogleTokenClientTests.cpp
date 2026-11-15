
#include <gtest/gtest.h>

#include "GoogleTokenClient.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QEventLoop>
#include <QHostAddress>
#include <QNetworkReply>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QUrlQuery>

class FakeTokenServer : public QObject
{
public:
    explicit FakeTokenServer(QObject* parent = nullptr)
        : QObject(parent)
    {
        connect(
            &server_,
            &QTcpServer::newConnection,
            this,
            &FakeTokenServer::handleNewConnection);
    }

    bool start()
    {
        return server_.listen(
            QHostAddress::LocalHost,
            0);
    }

    QUrl url() const
    {
        return QUrl(
            QString("http://127.0.0.1:%1/token")
                .arg(server_.serverPort()));
    }

    void setResponse(
        const QByteArray& statusCode,
        const QByteArray& body)
    {
        statusCode_ = statusCode;
        responseBody_ = body;
    }

    QByteArray requestBody() const
    {
        return requestBody_;
    }

    QByteArray requestHeaders() const
    {
        return requestHeaders_;
    }

private:
    void handleNewConnection()
    {
        while (server_.hasPendingConnections())
        {
            QTcpSocket* socket = server_.nextPendingConnection();

            if (!socket)
                continue;

            connect(
                socket,
                &QTcpSocket::readyRead,
                this,
                [this, socket]()
                {
                    handleReadyRead(socket);
                });

            connect(
                socket,
                &QTcpSocket::disconnected,
                socket,
                &QObject::deleteLater);
        }
    }

    void handleReadyRead(QTcpSocket* socket)
    {
        requestBuffer_ += socket->readAll();

        const qsizetype headerEnd =
            requestBuffer_.indexOf("\r\n\r\n");

        if (headerEnd < 0)
            return;

        requestHeaders_ = requestBuffer_.left(headerEnd);

        int contentLength = 0;

        for (const QByteArray& line : requestHeaders_.split('\n'))
        {
            const QByteArray trimmed = line.trimmed();

            if (trimmed.toLower().startsWith("content-length:"))
            {
                contentLength =
                    trimmed.mid(15).trimmed().toInt();
                break;
            }
        }

        requestBody_ = requestBuffer_.mid(headerEnd + 4);

        if (requestBody_.size() < contentLength)
            return;

        // Avoid sending the same response more than once.
        disconnect(
            socket,
            &QTcpSocket::readyRead,
            this,
            nullptr);

        const QByteArray response =
            "HTTP/1.1 " + statusCode_ +
            "\r\nContent-Type: application/json\r\n"
            "Content-Length: " +
            QByteArray::number(responseBody_.size()) +
            "\r\nConnection: close\r\n\r\n" +
            responseBody_;

        socket->write(response);
        socket->disconnectFromHost();
    }

    QTcpServer server_;

    QByteArray requestBuffer_;
    QByteArray requestHeaders_;
    QByteArray requestBody_;

    QByteArray statusCode_{"200 OK"};
    QByteArray responseBody_;
};


// ------------------------------------------------------------
// Test 1: Successful token refresh
// ------------------------------------------------------------

TEST(
    GoogleTokenClientTests,
    RefreshAccessTokenSucceeds)
{
    FakeTokenServer server;
    ASSERT_TRUE(server.start());

    server.setResponse(
        "200 OK",
        R"({
            "access_token": "new_test_access_token",
            "expires_in": 3600,
            "token_type": "Bearer",
            "scope": "https://www.googleapis.com/auth/drive.file"
        })");

    GoogleTokenClient client(server.url());

    QEventLoop loop;
    QTimer timeout;

    timeout.setSingleShot(true);

    OAuthToken receivedToken;
    bool received = false;
    QString errorMessage;

    QObject::connect(
        &client,
        &GoogleTokenClient::tokenReceived,
        &loop,
        [&](const OAuthToken& token)
        {
            receivedToken = token;
            received = true;
            loop.quit();
        });

    QObject::connect(
        &client,
        &GoogleTokenClient::tokenExchangeFailed,
        &loop,
        [&](const QString& message)
        {
            errorMessage = message;
            loop.quit();
        });

    QObject::connect(
        &timeout,
        &QTimer::timeout,
        &loop,
        &QEventLoop::quit);

    timeout.start(5000);

    client.refreshAccessToken(
        "test_client_id",
        "test_client_secret",
        "test_refresh_token");

    loop.exec();

    EXPECT_TRUE(received)
        << errorMessage.toStdString();

    EXPECT_EQ(
        receivedToken.accessToken,
        "new_test_access_token");

    EXPECT_TRUE(
        receivedToken.refreshToken.isEmpty());

    EXPECT_EQ(
        receivedToken.tokenType,
        "Bearer");

    EXPECT_EQ(
        receivedToken.scope,
        "https://www.googleapis.com/auth/drive.file");

    EXPECT_TRUE(
        receivedToken.expiresAt.isValid());

    EXPECT_GT(
        receivedToken.expiresAt,
        QDateTime::currentDateTimeUtc());

    EXPECT_FALSE(
        server.requestBody().isEmpty());
}


// ------------------------------------------------------------
// Test 2: Verify outgoing refresh request parameters
// ------------------------------------------------------------

TEST(
    GoogleTokenClientTests,
    RefreshRequestContainsRequiredParameters)
{
    FakeTokenServer server;
    ASSERT_TRUE(server.start());

    server.setResponse(
        "200 OK",
        R"({
            "access_token": "test_access",
            "expires_in": 3600,
            "token_type": "Bearer"
        })");

    GoogleTokenClient client(server.url());

    QEventLoop loop;
    QTimer timeout;

    timeout.setSingleShot(true);

    bool completed = false;

    QObject::connect(
        &client,
        &GoogleTokenClient::tokenReceived,
        &loop,
        [&]()
        {
            completed = true;
            loop.quit();
        });

    QObject::connect(
        &client,
        &GoogleTokenClient::tokenExchangeFailed,
        &loop,
        [&](const QString&)
        {
            loop.quit();
        });

    QObject::connect(
        &timeout,
        &QTimer::timeout,
        &loop,
        &QEventLoop::quit);

    timeout.start(5000);

    client.refreshAccessToken(
        "my_client_id",
        "my_client_secret",
        "my_refresh_token");

    loop.exec();

    ASSERT_TRUE(completed);

    const QByteArray headers =
        server.requestHeaders();

    EXPECT_TRUE(
        headers.startsWith("POST /token HTTP/1.1"));

    EXPECT_TRUE(
        headers.toLower().contains(
            "content-type: application/x-www-form-urlencoded"));

    const QUrlQuery query(
        QString::fromUtf8(server.requestBody()));

    EXPECT_EQ(
        query.queryItemValue("client_id"),
        "my_client_id");

    EXPECT_EQ(
        query.queryItemValue("client_secret"),
        "my_client_secret");

    EXPECT_EQ(
        query.queryItemValue("refresh_token"),
        "my_refresh_token");

    EXPECT_EQ(
        query.queryItemValue("grant_type"),
        "refresh_token");
}


// ------------------------------------------------------------
// Test 3: Google rejects the refresh request
// ------------------------------------------------------------

TEST(
    GoogleTokenClientTests,
    RefreshAccessTokenReportsGoogleError)
{
    FakeTokenServer server;
    ASSERT_TRUE(server.start());

    server.setResponse(
        "400 Bad Request",
        R"({
            "error": "invalid_grant",
            "error_description": "Token has been expired or revoked."
        })");

    GoogleTokenClient client(server.url());

    QEventLoop loop;
    QTimer timeout;

    timeout.setSingleShot(true);

    QString errorMessage;

    QObject::connect(
        &client,
        &GoogleTokenClient::tokenExchangeFailed,
        &loop,
        [&](const QString& message)
        {
            errorMessage = message;
            loop.quit();
        });

    QObject::connect(
        &client,
        &GoogleTokenClient::tokenReceived,
        &loop,
        &QEventLoop::quit);

    QObject::connect(
        &timeout,
        &QTimer::timeout,
        &loop,
        &QEventLoop::quit);

    timeout.start(5000);

    client.refreshAccessToken(
        "test_client_id",
        "test_client_secret",
        "expired_refresh_token");

    loop.exec();

    EXPECT_EQ(
        errorMessage,
        "Token has been expired or revoked.");
}


// ------------------------------------------------------------
// Test executable entry point
// ------------------------------------------------------------

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
