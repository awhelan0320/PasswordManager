#include "GoogleTokenClient.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>

GoogleTokenClient::GoogleTokenClient(QObject* parent)
    : QObject(parent)
{
}

void GoogleTokenClient::exchangeAuthorizationCode(
    const QString& clientId,
    const QString& clientSecret,
    const QString& code,
    const QString& redirectUri,
    const QString& codeVerifier)
{
    // if (clientId.isEmpty() ||
    //     clientSecret.isEmpty() ||
    //     code.isEmpty() ||
    //     redirectUri.isEmpty() ||
    //     codeVerifier.isEmpty())
    // {
    //     emit tokenExchangeFailed(
    //         "Missing information required for token exchange.");
    //     return;
    // }

if (clientId.isEmpty() ||
    clientSecret.isEmpty() ||
    code.isEmpty() ||
    redirectUri.isEmpty() ||
    codeVerifier.isEmpty())
{
    qWarning() << "OAuth token exchange parameters:"
               << "clientId present:" << !clientId.isEmpty()
               << "clientSecret present:" << !clientSecret.isEmpty()
               << "code present:" << !code.isEmpty()
               << "redirectUri present:" << !redirectUri.isEmpty()
               << "codeVerifier present:" << !codeVerifier.isEmpty();

    emit tokenExchangeFailed(
        "Missing information required for token exchange.");
    return;
} else {
    qWarning() << "OAuth token exchange parameter: All required parameters are present.";
}

    const QUrl tokenUrl(
        "https://oauth2.googleapis.com/token");

    QNetworkRequest request(tokenUrl);

    request.setHeader(
        QNetworkRequest::ContentTypeHeader,
        "application/x-www-form-urlencoded");

    QUrlQuery parameters;

    parameters.addQueryItem("client_id", clientId);
    parameters.addQueryItem("code", code);
    parameters.addQueryItem("code_verifier", codeVerifier);
    parameters.addQueryItem("redirect_uri", redirectUri);
    parameters.addQueryItem("client_secret", clientSecret);
    parameters.addQueryItem("grant_type", "authorization_code");

    const QByteArray postData =
        parameters.toString(QUrl::FullyEncoded).toUtf8();

    QNetworkReply* reply =
        networkManager_.post(request, postData);

    connect(reply, &QNetworkReply::finished,
            this, [this, reply]()
    {
        const QByteArray responseData = reply->readAll();

        const auto networkError = reply->error();
        const QString networkErrorText = reply->errorString();

        reply->deleteLater();

        QJsonParseError parseError;

        const QJsonDocument document =
            QJsonDocument::fromJson(responseData, &parseError);

        if (parseError.error != QJsonParseError::NoError ||
            !document.isObject())
        {
            emit tokenExchangeFailed(
                networkError == QNetworkReply::NoError
                    ? "Google returned an invalid token response."
                    : "Network error while exchanging authorization code: "
                          + networkErrorText);
            return;
        }

        const QJsonObject json = document.object();

        if (networkError != QNetworkReply::NoError ||
            json.contains("error"))
        {
            const QString description =
                json.value("error_description").toString();

            emit tokenExchangeFailed(
                description.isEmpty()
                    ? "Google rejected the token exchange."
                    : description);
            return;
        }

        OAuthToken token;

        token.accessToken =
            json.value("access_token").toString();

        token.refreshToken =
            json.value("refresh_token").toString();

        token.tokenType =
            json.value("token_type").toString();

        token.scope =
            json.value("scope").toString();

        const qint64 expiresIn =
            json.value("expires_in").toVariant().toLongLong();

        if (token.accessToken.isEmpty() || expiresIn <= 0)
        {
            emit tokenExchangeFailed(
                "Google's token response is missing required fields.");
            return;
        }

        token.expiresAt =
            QDateTime::currentDateTimeUtc().addSecs(expiresIn);

        emit tokenReceived(token);
    });
}