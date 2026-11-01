#pragma once

#include <QObject>
#include <QDateTime>
#include <QNetworkAccessManager>

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


    void exchangeAuthorizationCode(
        const QString& clientId,
        const QString& clientSecret,
        const QString& code,
        const QString& redirectUri,
        const QString& codeVerifier);

signals:
    void tokenReceived(const OAuthToken& token);
    void tokenExchangeFailed(const QString& message);

private:
    QNetworkAccessManager networkManager_;
};