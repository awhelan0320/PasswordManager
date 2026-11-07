#pragma once

#include <QObject>
#include <QDateTime>
#include <QNetworkAccessManager>
#include <QUrl>

struct OAuthToken
{
    QString accessToken;
    QString refreshToken;
    QString tokenType;
    QString scope;
    QDateTime expiresAt;
};

class GoogleTokenClient : public QObject
{
    Q_OBJECT

public:
    explicit GoogleTokenClient(QObject* parent = nullptr);

    // Allows tests to provide a local token endpoint.
    explicit GoogleTokenClient(
        const QUrl& tokenEndpoint,
        QObject* parent = nullptr);

    void exchangeAuthorizationCode(
        const QString& clientId,
        const QString& clientSecret,
        const QString& code,
        const QString& redirectUri,
        const QString& codeVerifier);

    void refreshAccessToken(
        const QString& clientId,
        const QString& clientSecret,
        const QString& refreshToken);

signals:
    void tokenReceived(const OAuthToken& token);
    void tokenExchangeFailed(const QString& message);

private:
    QNetworkAccessManager networkManager_;
    QUrl tokenEndpoint_;
};