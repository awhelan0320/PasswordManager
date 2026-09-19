#include "VaultVerifier.h"
#include <stdexcept>
#include <sodium.h>


/**
 * @brief Creates a verification record authenticated by the supplied key.
 * @param key The master key used to encrypt the verification record.
 * @return A Record containing the encrypted verification data and nonce.
 * @throws std::runtime_error if encryption fails.
 */
VaultVerifier::Record VaultVerifier::create(
    const MasterKey::Key& key)
{
    static constexpr char verificationText[] =
        "PasswordManager vault verification";

    Record record;

    randombytes_buf(
        record.nonce.data(),
        record.nonce.size()
    );

    record.ciphertext.resize(
        sizeof(verificationText) - 1 +
        crypto_aead_xchacha20poly1305_ietf_ABYTES
    );

    unsigned long long ciphertextLength = 0;

    const int result =
        crypto_aead_xchacha20poly1305_ietf_encrypt(
            record.ciphertext.data(),
            &ciphertextLength,

            reinterpret_cast<const unsigned char*>(
                verificationText
            ),
            sizeof(verificationText) - 1,

            nullptr,
            0,

            nullptr,

            record.nonce.data(),
            key.data()
        );

    if (result != 0)
    {
        throw std::runtime_error(
            "Failed to encrypt vault verification record."
        );
    }

    record.ciphertext.resize(
        static_cast<std::size_t>(ciphertextLength)
    );

    return record;
}

/**
 * @brief Returns whether the supplied key can successfully validate the record.
 * @param key The master key used to decrypt the verification record.
 * @param record The verification record to validate.
 * @return true if the key is valid, false otherwise.
 */
bool VaultVerifier::verify(
    const MasterKey::Key& key,
    const Record& record)
{
    static constexpr char verificationText[] =
        "PasswordManager vault verification";

    std::array<unsigned char, sizeof(verificationText) - 1>
        plaintext{};

    unsigned long long plaintextLength = 0;

    const int result =
        crypto_aead_xchacha20poly1305_ietf_decrypt(
            plaintext.data(),
            &plaintextLength,

            nullptr,

            record.ciphertext.data(),
            record.ciphertext.size(),

            nullptr,
            0,

            record.nonce.data(),
            key.data()
        );

    if (result != 0)
    {
        return false;
    }

    if (plaintextLength != sizeof(verificationText) - 1)
    {
        return false;
    }

    return sodium_memcmp(
        plaintext.data(),
        verificationText,
        sizeof(verificationText) - 1
    ) == 0;
}