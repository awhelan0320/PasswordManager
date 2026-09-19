#pragma once

#include <string>

#include "MasterKey.h"
#include "SqliteDatabase.h"
#include "VaultVerifier.h"

class VaultMetadata
{
public:
    explicit VaultMetadata(SqliteDatabase& database);

    void initialize();

    void create(
        const std::string& masterPassword
    );

    MasterKey::Salt salt() const;

    VaultVerifier::Record verificationRecord() const;

    bool exists() const;

private:
    SqliteDatabase& database_;
};