#pragma once

#include <array>
#include <string>
#include <vector>

#include "MasterKey.h"

class VaultCrypto
{
public:
    static std::vector<unsigned char> encrypt(
        const std::string& plaintext,
        const MasterKey::Key& key);

    static std::string decrypt(
        const std::vector<unsigned char>& ciphertext,
        const MasterKey::Key& key);
};