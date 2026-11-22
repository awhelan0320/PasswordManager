#pragma once

#include <array>
#include <string>
#include <vector>
#include <sodium.h>

/**
 * @brief The MasterKey class provides functionality for generating and deriving cryptographic keys.
 *
 * This class is responsible for generating a random salt and deriving a master key from a password
 * using the Argon2id key derivation function. It ensures that the derived key is suitable for
 * cryptographic operations, such as encrypting and decrypting sensitive data.
 */
class MasterKey
{
public: 
    
    using Key = std::array<unsigned char, 32>;

    using Salt = std::array<unsigned char, 16>;

    static Salt generateSalt();

    static Key derive(
        const std::string& password,
        const Salt& salt);
};