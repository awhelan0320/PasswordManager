#include <gtest/gtest.h>

#include "GoogleTokenClient.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QHostAddress>
#include <QNetworkReply>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QUrlQuery>

static void ensureQtApplication()
{
    if (!QCoreApplication::instance())
    {
        static int argc = 1;
        static char appName[] = "PasswordManagerTests";
        static char* argv[] = { appName, nullptr };

        static QCoreApplication app(argc, argv);
        Q_UNUSED(app);
    }
}

class FakeTokenServer
{
public:
    FakeTokenServer()
    {
        QObject::connect(
            &server_,
            &QTcpServer::newConnection,
            [&]()
            {
                QTcpSocket* socket = server_.nextPendingConnection();

                QObject::connect(
                    socket,
                    &QTcpSocket::readyRead,
                    socket,
                    [this, socket]()
                    {
                        request_ += socket->readAll();

                        const qsizetype headerEnd =
                            request_.indexOf("\r\n\r\n");

                        if (headerEnd < 0)
                            return;

                        const QByteArray headers =
                            request_.left(headerEnd);

                        int contentLength = 0;

                        for (const QByteArray& line : headers.split('\n'))
                        {
                            const QByteArray trimmed = line.trimmed();

                            if (trimmed.toLower().startsWith("content-length:"))
                            {
                                contentLength =
                                    trimmed.mid(15).trimmed().toInt();
                                break;
                            }
                        }

                        const QByteArray body =
                            request_.mid(headerEnd + 4);

                        if (body.size() < contentLength)
                            return;

                        const QByteArray response =
                            "HTTP/1.1 " + statusCode_ +
                            "\r\nContent-Type: application/json\r\n"
                            "Content-Length: " +
                            QByteArray::number(responseBody_.size()) +
                            "\r\nConnection: close\r\n\r\n" +
                            responseBody_;

                        socket->write(response);
                        socket->disconnectFromHost();
                    });
            });
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
        const qsizetype headerEnd =
            request_.indexOf("\r\n\r\n");

        if (headerEnd < 0)
            return {};

        return request_.mid(headerEnd + 4);
    }

    QByteArray requestHeaders() const
    {
        const qsizetype headerEnd =
            request_.indexOf("\r\n\r\n");

        if (headerEnd < 0)
            return {};

        return request_.left(headerEnd);
    }

private:
    QTcpServer server_;
    QByteArray request_;
    QByteArray statusCode_{"200 OK"};
    QByteArray responseBody_;
};

TEST(GoogleTokenClientTests, RefreshAccessTokenSucceeds)
{
    ensureQtApplication();

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

    EXPECT_TRUE(received) << errorMessage.toStdString();

    EXPECT_EQ(
        receivedToken.accessToken,
        "new_test_access_token");

    EXPECT_TRUE(receivedToken.refreshToken.isEmpty());

    EXPECT_EQ(receivedToken.tokenType, "Bearer");

    EXPECT_EQ(
        receivedToken.scope,
        "https://www.googleapis.com/auth/drive.file");

    EXPECT_TRUE(receivedToken.expiresAt.isValid());

    EXPECT_GT(
        receivedToken.expiresAt,
        QDateTime::currentDateTimeUtc());

    EXPECT_FALSE(server.requestBody().isEmpty());
}

TEST(GoogleTokenClientTests, RefreshRequestContainsRequiredParameters)
{
    ensureQtApplication();

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

    QObject::connect(
        &client,
        &GoogleTokenClient::tokenReceived,
        &loop,
        &QEventLoop::quit);

    QObject::connect(
        &client,
        &GoogleTokenClient::tokenExchangeFailed,
        &loop,
        &QEventLoop::quit);

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

    const QByteArray headers = server.requestHeaders();

    EXPECT_TRUE(headers.startsWith("POST /token HTTP/1.1"));
    EXPECT_TRUE(
        headers.toLower().contains("content-type: application/x-www-form-urlencoded"));

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

TEST(GoogleTokenClientTests, RefreshAccessTokenReportsGoogleError)
{
    ensureQtApplication();

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