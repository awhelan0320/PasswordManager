#include "PkceHelper.h"

#include <QByteArray>
#include <QCryptographicHash>
#include <QRandomGenerator>

PkcePair PkceHelper::generate()
{
    // 32 random bytes produce a 43-character Base64URL verifier.
    QByteArray randomBytes(32, Qt::Uninitialized);

    auto* rng = QRandomGenerator::system();

    for (qsizetype i = 0; i < randomBytes.size(); i += 4)
    {
        const quint32 value = rng->generate();

        for (qsizetype j = 0;
             j < 4 && i + j < randomBytes.size();
             ++j)
        {
            randomBytes[i + j] =
                static_cast<char>((value >> (j * 8)) & 0xFF);
        }
    }

    const QString verifier = QString::fromLatin1(
        randomBytes.toBase64(
            QByteArray::Base64UrlEncoding |
            QByteArray::OmitTrailingEquals));

    return {
        verifier,
        createChallenge(verifier)
    };
}

QString PkceHelper::createChallenge(const QString& verifier)
{
    const QByteArray hash = QCryptographicHash::hash(
        verifier.toLatin1(),
        QCryptographicHash::Sha256);

    return QString::fromLatin1(
        hash.toBase64(
            QByteArray::Base64UrlEncoding |
            QByteArray::OmitTrailingEquals));
}