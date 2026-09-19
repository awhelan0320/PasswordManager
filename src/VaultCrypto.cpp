#include "VaultCrypto.h"

#include <sodium.h>
#include <algorithm>
#include <stdexcept>

std::vector<unsigned char> VaultCrypto::encrypt(
    const std::string& plaintext,
    const MasterKey::Key& key)
{
    constexpr std::size_t nonceSize =
        crypto_aead_xchacha20poly1305_ietf_NPUBBYTES;

    std::array<unsigned char, nonceSize> nonce{};

    randombytes_buf(
        nonce.data(),
        nonce.size()
    );

    std::vector<unsigned char> result(
        nonce.size() +
        plaintext.size() +
        crypto_aead_xchacha20poly1305_ietf_ABYTES
    );

    std::copy(
        nonce.begin(),
        nonce.end(),
        result.begin()
    );

    unsigned long long ciphertextLength = 0;

    const int rc =
        crypto_aead_xchacha20poly1305_ietf_encrypt(
            result.data() + nonce.size(),
            &ciphertextLength,

            reinterpret_cast<const unsigned char*>(
                plaintext.data()
            ),
            plaintext.size(),

            nullptr,
            0,

            nullptr,

            nonce.data(),
            key.data()
        );

    if (rc != 0)
    {
        throw std::runtime_error(
            "Failed to encrypt password."
        );
    }

    result.resize(
        nonce.size() +
        static_cast<std::size_t>(ciphertextLength)
    );

    return result;
}

std::string VaultCrypto::decrypt(
    const std::vector<unsigned char>& ciphertext,
    const MasterKey::Key& key)
{
    constexpr std::size_t nonceSize =
        crypto_aead_xchacha20poly1305_ietf_NPUBBYTES;

    constexpr std::size_t tagSize =
        crypto_aead_xchacha20poly1305_ietf_ABYTES;

    if (ciphertext.size() < nonceSize + tagSize)
    {
        throw std::runtime_error(
            "Invalid encrypted password."
        );
    }

    const unsigned char* nonce =
        ciphertext.data();

    const unsigned char* encryptedData =
        ciphertext.data() + nonceSize;

    const std::size_t encryptedSize =
        ciphertext.size() - nonceSize;

    std::vector<unsigned char> plaintext(
        encryptedSize - tagSize
    );

    unsigned long long plaintextLength = 0;

    const int rc =
        crypto_aead_xchacha20poly1305_ietf_decrypt(
            plaintext.data(),
            &plaintextLength,

            nullptr,

            encryptedData,
            encryptedSize,

            nullptr,
            0,

            nonce,
            key.data()
        );

    if (rc != 0)
    {
        throw std::runtime_error(
            "Failed to decrypt password."
        );
    }

    return std::string(
        reinterpret_cast<const char*>(
            plaintext.data()
        ),
        plaintextLength
    );
}



