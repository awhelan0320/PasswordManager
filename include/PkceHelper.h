#pragma once

#include <QString>

struct PkcePair
{
    QString verifier;
    QString challenge;
};

class PkceHelper
{
public:
    // Generate a random verifier and its S256 challenge.
    static PkcePair generate();

    // Calculate the S256 challenge for a verifier.
    static QString createChallenge(const QString& verifier);
};