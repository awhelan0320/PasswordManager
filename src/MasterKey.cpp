#include "MasterKey.h"

#include <stdexcept>

/**
 * Implementation of the MasterKey class.
 */

/* What  is a salt? A salt is a random value that is used in conjunction with 
* a password to derive a cryptographic key. The purpose of a salt is to add 
* randomness to the key derivation process, making it more difficult for 
* attackers to use precomputed tables (rainbow tables) to crack passwords. 
* By using a unique salt for each password, even if two users have the same 
* password, their derived keys will be different, enhancing security.   
*/


 /**
  * Generates a random salt for key derivation.
  * @return A random salt.
  */
MasterKey::Salt MasterKey::generateSalt()
{
    Salt salt{};

    randombytes_buf(
        salt.data(),
        salt.size()
    );

    return salt;
}

/**
 * Derives a key from a password and salt.
 * @param password The password to derive the key from.
 * @param salt The salt to use for key derivation.
 * @return The derived key.
 */
MasterKey::Key MasterKey::derive(
    const std::string& password,
    const Salt& salt)
{
    Key key{};

    if (crypto_pwhash(
            key.data(),
            key.size(),
            password.data(),
            password.size(),
            salt.data(),
            crypto_pwhash_OPSLIMIT_MODERATE,
            crypto_pwhash_MEMLIMIT_MODERATE,
            crypto_pwhash_ALG_ARGON2ID13) != 0)
    {
        throw std::runtime_error(
            "Password key derivation failed."
        );
    }

    return key;
}
