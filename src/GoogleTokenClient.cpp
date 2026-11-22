#include "GoogleTokenClient.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>

/**
 * @brief Constructs a GoogleTokenClient with an optional parent QObject.
 * @param parent The parent QObject, default is nullptr.
 *
 * This constructor initializes the GoogleTokenClient, setting up the necessary
 * network access manager for handling HTTP requests to Google's OAuth 2.0 token endpoint.
 */
GoogleTokenClient::GoogleTokenClient(QObject* parent)
    : GoogleTokenClient(
          QUrl("https://oauth2.googleapis.com/token"),
          parent)
{
}

/**
 * @brief Constructs a GoogleTokenClient with a specified token endpoint and an optional parent QObject.
 * @param tokenEndpoint The URL of the OAuth 2.0 token endpoint.
 * @param parent The parent QObject, default is nullptr.
 *
 * This constructor initializes the GoogleTokenClient with a custom token endpoint,
 * allowing for flexibility in testing or using different OAuth 2.0 providers.
 */
GoogleTokenClient::GoogleTokenClient(
    const QUrl& tokenEndpoint,
    QObject* parent)
    : QObject(parent),
      tokenEndpoint_(tokenEndpoint)
{
}

/**
 * @brief Exchanges an authorization code for an access token.
 *
 * This method sends a POST request to the OAuth 2.0 token endpoint with the provided
 * client ID, client secret, authorization code, redirect URI, and code verifier.
 * Upon receiving a response, it parses the JSON to extract the access token, refresh
 * token, token type, scope, and expiration time. It emits signals for success or failure.
 *
 * @param clientId The client ID for the OAuth 2.0 application.
 * @param clientSecret The client secret for the OAuth 2.0 application.
 * @param code The authorization code received from the Google OAuth 2.0 server.
 * @param redirectUri The redirect URI used in the authorization request.
 * @param codeVerifier The PKCE code verifier for the OAuth 2.0 flow.
 */
void GoogleTokenClient::exchangeAuthorizationCode(
    const QString& clientId,
    const QString& clientSecret,
    const QString& code,
    const QString& redirectUri,
    const QString& codeVerifier)
{

qInfo() << "Starting Google authorization-code exchange.";


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

    const QUrl tokenUrl = tokenEndpoint_;
    // const QUrl tokenUrl(
    //     "https://oauth2.googleapis.com/token");

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

        qWarning() << "HTTP status:"
                << reply->attribute(
                        QNetworkRequest::HttpStatusCodeAttribute);

        qWarning() << "Google OAuth response:"
                << responseData;

        qWarning() << "Network error:"
                << reply->errorString();

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

/**
 * @brief Refreshes an access token using a refresh token.
 *
 * This method sends a POST request to the OAuth 2.0 token endpoint with the provided
 * client ID, client secret, and refresh token. Upon receiving a response, it parses
 * the JSON to extract the new access token, refresh token (if provided), token type,
 * scope, and expiration time. It emits signals for success or failure.
 *
 * @param clientId The client ID for the OAuth 2.0 application.
 * @param clientSecret The client secret for the OAuth 2.0 application.
 * @param refreshToken The refresh token used to obtain a new access token.
 */
void GoogleTokenClient::refreshAccessToken(
    const QString& clientId,
    const QString& clientSecret,
    const QString& refreshToken)
{
    qInfo() << "Starting Google refresh-token request.";
    if (clientId.isEmpty() ||
        clientSecret.isEmpty() ||
        refreshToken.isEmpty())
    {
        emit tokenExchangeFailed(
            "Missing information required to refresh access token.");
        return;
    }

    // const QUrl tokenUrl(
    //     "https://oauth2.googleapis.com/token");

    const QUrl tokenUrl = tokenEndpoint_;

    QNetworkRequest request(tokenUrl);

    request.setHeader(
        QNetworkRequest::ContentTypeHeader,
        "application/x-www-form-urlencoded");

    QUrlQuery parameters;

    parameters.addQueryItem("client_id", clientId);
    parameters.addQueryItem("client_secret", clientSecret);
    parameters.addQueryItem("refresh_token", refreshToken);
    parameters.addQueryItem("grant_type", "refresh_token");

    const QByteArray postData =
        parameters.toString(QUrl::FullyEncoded).toUtf8();

    QNetworkReply* reply =
        networkManager_.post(request, postData);

    connect(reply, &QNetworkReply::finished,
            this, [this, reply, refreshToken]()
    {
        const QByteArray responseData = reply->readAll();

        qWarning() << "HTTP status:"
                << reply->attribute(
                        QNetworkRequest::HttpStatusCodeAttribute);

        qWarning() << "Google OAuth response:"
                << responseData;

        qWarning() << "Network error:"
                << reply->errorString();



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
                    ? "Google returned an invalid refresh response."
                    : "Network error while refreshing access token: "
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
                    ? "Google rejected the token refresh request."
                    : description);
            return;
        }

        OAuthToken token;

        token.accessToken =
            json.value("access_token").toString();

        // Google normally does not return a new refresh token here.
        token.refreshToken =
            json.value("refresh_token").toString();

        if (token.refreshToken.isEmpty())
        {
            token.refreshToken = refreshToken;
        }

        token.tokenType =
            json.value("token_type").toString();

        token.scope =
            json.value("scope").toString();

        const qint64 expiresIn =
            json.value("expires_in").toVariant().toLongLong();

        if (token.accessToken.isEmpty() || expiresIn <= 0)
        {
            emit tokenExchangeFailed(
                "Google's refresh response is missing required fields.");
            return;
        }

        token.expiresAt =
            QDateTime::currentDateTimeUtc().addSecs(expiresIn);

        emit tokenReceived(token);
    });
}