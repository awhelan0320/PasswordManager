#pragma once

#include <QObject>
#include <QDateTime>
#include <QNetworkAccessManager>
#include <QUrl>

/**
 * @brief The OAuthToken struct holds the details of an OAuth 2.0 token.
 *
 * This struct contains the access token, refresh token, token type, scope, and
 * expiration time associated with an OAuth 2.0 authentication process.
 */

struct OAuthToken
{
    QString accessToken;
    QString refreshToken;
    QString tokenType;
    QString scope;
    QDateTime expiresAt;
};

/**
 * @brief The GoogleTokenClient class handles OAuth 2.0 token exchange and refresh operations.
 *
 * This class provides methods to exchange an authorization code for an access token
 * and to refresh an existing access token using a refresh token. It emits signals
 * upon successful token retrieval or failure.
 */
class GoogleTokenClient : public QObject
{
    Q_OBJECT

public:
    explicit GoogleTokenClient(QObject* parent = nullptr);

    // Allows tests to provide a local token endpoint.
    explicit GoogleTokenClient(
        const QUrl& tokenEndpoint,
        QObject* parent = nullptr);

    /**
     * @brief Exchanges an authorization code for an access token.
     * @param clientId The client ID for the OAuth 2.0 application.
     * @param clientSecret The client secret for the OAuth 2.0 application.
     * @param code The authorization code received from the Google OAuth 2.0 server.
     * @param redirectUri The redirect URI used in the authorization request.
     * @param codeVerifier The PKCE code verifier for the OAuth 2.0 flow.
     */
    void exchangeAuthorizationCode(
        const QString& clientId,
        const QString& clientSecret,
        const QString& code,
        const QString& redirectUri,
        const QString& codeVerifier);
    
    /**
     * @brief Refreshes an access token using a refresh token.
     * @param clientId The client ID for the OAuth 2.0 application.
     * @param clientSecret The client secret for the OAuth 2.0 application.
     * @param refreshToken The refresh token used to obtain a new access token.
     */
    void refreshAccessToken(
        const QString& clientId,
        const QString& clientSecret,
        const QString& refreshToken);

signals:
    /**
     * @brief Signal emitted when an OAuth token is successfully received.
     * @param token The received OAuthToken containing access and refresh tokens, token type, scope, and expiration time.
     */
    void tokenReceived(const OAuthToken& token);

    /**
     * @brief Signal emitted when the token exchange or refresh operation fails.
     * @param message A string describing the error that occurred during the operation.
     */
    void tokenExchangeFailed(const QString& message);

private:
    // Network access manager for handling HTTP requests.
    QNetworkAccessManager networkManager_;
    // The token endpoint URL for exchanging and refreshing tokens.
    QUrl tokenEndpoint_;
};