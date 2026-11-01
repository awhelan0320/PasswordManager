#include "PkceHelper.h"

#include <QByteArray>
#include <QCryptographicHash>

#include <gtest/gtest.h>

TEST(PkceHelperTests, GeneratesVerifierOfCorrectLength)
{
    const PkcePair pair = PkceHelper::generate();

    EXPECT_EQ(pair.verifier.size(), 43);
}

TEST(PkceHelperTests, VerifierUsesBase64UrlCharacters)
{
    const PkcePair pair = PkceHelper::generate();

    for (const QChar ch : pair.verifier)
    {
        const bool valid =
            (ch >= 'A' && ch <= 'Z') ||
            (ch >= 'a' && ch <= 'z') ||
            (ch >= '0' && ch <= '9') ||
            ch == '-' || ch == '_';

        EXPECT_TRUE(valid) << ch.toLatin1();
    }
}

TEST(PkceHelperTests, GeneratesChallengeOfCorrectLength)
{
    const PkcePair pair = PkceHelper::generate();

    EXPECT_EQ(pair.challenge.size(), 43);
}

TEST(PkceHelperTests, ChallengeMatchesSha256OfVerifier)
{
    const PkcePair pair = PkceHelper::generate();

    const QByteArray expectedHash = QCryptographicHash::hash(
        pair.verifier.toLatin1(),
        QCryptographicHash::Sha256);

    const QString expectedChallenge = QString::fromLatin1(
        expectedHash.toBase64(
            QByteArray::Base64UrlEncoding |
            QByteArray::OmitTrailingEquals));

    EXPECT_EQ(pair.challenge, expectedChallenge);
}

TEST(PkceHelperTests, DifferentCallsGenerateDifferentVerifiers)
{
    const PkcePair first = PkceHelper::generate();
    const PkcePair second = PkceHelper::generate();

    EXPECT_NE(first.verifier, second.verifier);
}

TEST(PkceHelperTests, ChallengeIsDeterministic)
{
    const QString verifier =
        QStringLiteral("test-verifier-for-pkce");

    EXPECT_EQ(
        PkceHelper::createChallenge(verifier),
        PkceHelper::createChallenge(verifier));
}