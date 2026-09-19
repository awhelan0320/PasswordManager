#pragma once

#include <array>
#include <string>
#include <vector>

#include "MasterKey.h"




/**
 * Creates and validates an encrypted marker used to confirm that a vault was
 * opened with the correct master key.
 *
 * The verifier does not expose the marker's plaintext; callers retain the
 * returned ciphertext and nonce and pass them to verify() when validating a
 * key.
 */
class VaultVerifier
{
public:
    /** Encrypted verification data stored with a vault. */
    struct Record
    {
        std::vector<unsigned char> ciphertext;
        std::array<unsigned char, 24> nonce;
    };

    /** Creates a verification record authenticated by the supplied key. */
    static Record create(
        const MasterKey::Key& key);

    /** Returns whether the supplied key can successfully validate the record. */
    static bool verify(
        const MasterKey::Key& key,
        const Record& record);
};

