#pragma once

#include <QObject>
#include <QTcpServer>

#include "PkceHelper.h"

class OAuthManager : public QObject
{
    Q_OBJECT

public:
    explicit OAuthManager(QObject* parent = nullptr);

    bool startAuthorization(const QString& clientId);
    void cancel();

signals:
    void authorizationCodeReceived(
        const QString& code,
        const QString& redirectUri,
        const QString& verifier);

    void authorizationFailed(const QString& message);

private:
    void handleNewConnection();
    void handleRequest(QTcpSocket* socket);

    QTcpServer server_;
    QString state_;
    QString redirectUri_;
    PkcePair pkce_;
    bool active_{false};
};