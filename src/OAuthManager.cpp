#include "OAuthManager.h"

#include <QDesktopServices>
#include <QHostAddress>
#include <QRandomGenerator>
#include <QTcpSocket>
#include <QUrl>
#include <QUrlQuery>

namespace
{
QString generateState()
{
    QByteArray bytes(32, Qt::Uninitialized);
    auto* rng = QRandomGenerator::system();

    for (qsizetype i = 0; i < bytes.size(); i += 4)
    {
        const quint32 value = rng->generate();

        for (qsizetype j = 0;
             j < 4 && i + j < bytes.size();
             ++j)
        {
            bytes[i + j] =
                static_cast<char>((value >> (j * 8)) & 0xFF);
        }
    }

    return QString::fromLatin1(
        bytes.toBase64(
            QByteArray::Base64UrlEncoding |
            QByteArray::OmitTrailingEquals));
}
}

OAuthManager::OAuthManager(QObject* parent)
    : QObject(parent)
{
    connect(
        &server_,
        &QTcpServer::newConnection,
        this,
        &OAuthManager::handleNewConnection);
}

bool OAuthManager::startAuthorization(const QString& clientId)
{
    if (active_)
    {
        emit authorizationFailed(
            QStringLiteral("An authorization attempt is already active."));
        return false;
    }

    if (clientId.trimmed().isEmpty())
    {
        emit authorizationFailed(
            QStringLiteral("Google OAuth client ID is empty."));
        return false;
    }

    if (!server_.listen(QHostAddress(QStringLiteral("127.0.0.1")), 0))
    {
        emit authorizationFailed(
            QStringLiteral("Could not start localhost listener: %1")
                .arg(server_.errorString()));
        return false;
    }

    pkce_ = PkceHelper::generate();
    state_ = generateState();

    // redirectUri_ = QStringLiteral("http://127.0.0.1:%1/oauth2callback")
    //                    .arg(server_.serverPort());

    redirectUri_ = QStringLiteral("http://127.0.0.1:%1")
                       .arg(server_.serverPort());

    QUrl authorizationUrl(
        QStringLiteral("https://accounts.google.com/o/oauth2/v2/auth"));

    QUrlQuery query;
    query.addQueryItem(QStringLiteral("client_id"), clientId);
    query.addQueryItem(QStringLiteral("redirect_uri"), redirectUri_);
    query.addQueryItem(QStringLiteral("response_type"),
                       QStringLiteral("code"));
    query.addQueryItem(QStringLiteral("scope"),
                       QStringLiteral(
                           "https://www.googleapis.com/auth/drive.file"));
    query.addQueryItem(QStringLiteral("state"), state_);
    query.addQueryItem(QStringLiteral("code_challenge"),
                       pkce_.challenge);
    query.addQueryItem(QStringLiteral("code_challenge_method"),
                       QStringLiteral("S256"));

    authorizationUrl.setQuery(query);
    active_ = true;

    if (!QDesktopServices::openUrl(authorizationUrl))
    {
        cancel();
        emit authorizationFailed(
            QStringLiteral("Could not open the system browser."));
        return false;
    }

    return true;
}

void OAuthManager::cancel()
{
    active_ = false;
    server_.close();
    state_.clear();
    redirectUri_.clear();
    pkce_ = {};
}

void OAuthManager::handleNewConnection()
{
    while (server_.hasPendingConnections())
    {
        QTcpSocket* socket = server_.nextPendingConnection();

        if (!socket)
            continue;

        socket->setProperty("requestBuffer", QByteArray{});

        connect(socket, &QTcpSocket::readyRead,
                this, [this, socket]()
        {
            QByteArray buffer =
                socket->property("requestBuffer").toByteArray();

            buffer += socket->readAll();
            socket->setProperty("requestBuffer", buffer);

            if (buffer.contains("\r\n\r\n"))
                handleRequest(socket);
        });

        connect(socket, &QTcpSocket::disconnected,
                socket, &QObject::deleteLater);
    }
}

void OAuthManager::handleRequest(QTcpSocket* socket)
{
    // Respond to every HTTP request, including invalid callbacks.
    const QByteArray request =
        socket->property("requestBuffer").toByteArray();

    const QByteArray requestLine =
        request.left(request.indexOf("\r\n"));

    const QList<QByteArray> parts = requestLine.split(' ');

    if (!active_ || parts.size() < 2 ||
        parts.at(0) != "GET")
    {
        socket->write(
            "HTTP/1.1 400 Bad Request\r\n"
            "Connection: close\r\n"
            "Content-Length: 0\r\n\r\n");
        socket->disconnectFromHost();
        return;
    }

    const QUrl requestUrl =
        QUrl::fromEncoded(parts.at(1));

    if (requestUrl.path() != QStringLiteral("/"))
    {
        socket->write(
            "HTTP/1.1 404 Not Found\r\n"
            "Connection: close\r\n"
            "Content-Length: 0\r\n\r\n");
        socket->disconnectFromHost();
        return;
    }


    // if (requestUrl.path() != QStringLiteral("/oauth2callback"))
    // {
    //     socket->write(
    //         "HTTP/1.1 404 Not Found\r\n"
    //         "Connection: close\r\n"
    //         "Content-Length: 0\r\n\r\n");
    //     socket->disconnectFromHost();
    //     return;
    // }

    const QUrlQuery query(requestUrl);

    const QString returnedState =
        query.queryItemValue(QStringLiteral("state"));

    if (returnedState != state_)
    {
        socket->write(
            "HTTP/1.1 400 Bad Request\r\n"
            "Connection: close\r\n"
            "Content-Length: 0\r\n\r\n");
        socket->disconnectFromHost();

        emit authorizationFailed(
            QStringLiteral("OAuth state validation failed."));
        cancel();
        return;
    }

    const QString error =
        query.queryItemValue(QStringLiteral("error"));

    if (!error.isEmpty())
    {

        const QByteArray deniedBody =
            "Authorization received. You may close this window.";      

        // socket->write(
        //     "HTTP/1.1 200 OK\r\n"
        //     "Connection: close\r\n"
        //     "Content-Type: text/plain; charset=utf-8\r\n"
        //     "Content-Length: 29\r\n\r\n"
        //     "Google authorization was denied.");

        const QByteArray deniedResponse =
            "HTTP/1.1 200 OK\r\n"
            "Connection: close\r\n"
            "Content-Type: text/plain; charset=utf-8\r\n"
            "Content-Length: " +
            QByteArray::number(deniedBody.size()) +
            "\r\n\r\n" + deniedBody;

        socket->write(deniedResponse);

        socket->disconnectFromHost();

        cancel();
        emit authorizationFailed(
            QStringLiteral("Google authorization failed: %1").arg(error));
        return;
    }

    const QString code =
        query.queryItemValue(QStringLiteral("code"));

    if (code.isEmpty())
    {
        socket->write(
            "HTTP/1.1 400 Bad Request\r\n"
            "Connection: close\r\n"
            "Content-Length: 0\r\n\r\n");
        socket->disconnectFromHost();

        emit authorizationFailed(
            QStringLiteral("OAuth callback did not contain a code."));
        cancel();
        return;
    }

    // Keep the verifier and redirect URI for the token exchange.
    const QString verifier = pkce_.verifier;
    const QString redirectUri = redirectUri_;

    // socket->write(
    //     "HTTP/1.1 200 OK\r\n"
    //     "Connection: close\r\n"
    //     "Content-Type: text/plain; charset=utf-8\r\n"
    //     "Content-Length: 32\r\n\r\n"
    //     "Authorization received. You may close this window.");

    const QByteArray receivedBody =
        "Authorization received. You may close this window.";

    const QByteArray receivedResponse =
        "HTTP/1.1 200 OK\r\n"
        "Connection: close\r\n"
        "Content-Type: text/plain; charset=utf-8\r\n"
        "Content-Length: " +
        QByteArray::number(receivedBody.size()) +
        "\r\n\r\n" + receivedBody;

    socket->write(receivedResponse);
    socket->disconnectFromHost();

    cancel();

    emit authorizationCodeReceived(
        code, redirectUri, verifier);
}