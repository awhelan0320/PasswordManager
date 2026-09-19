#pragma once

#include <array>
#include <string>
#include <vector>
#include <sodium.h>

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