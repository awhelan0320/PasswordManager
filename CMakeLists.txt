cmake_minimum_required(VERSION 3.24)

project(
    PasswordManager
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(GTest REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(SQLite3 REQUIRED)
find_package(libsodium REQUIRED)


add_executable(PasswordManager
    apps/PasswordManagerApp.cpp
    src/VaultDatabase.cpp
    src/SqliteStatement.cpp
    src/SqliteDatabase.cpp
    src/MainWindow.cpp
    src/EntryDialog.cpp
    src/MasterKey.cpp
    src/VaultMetadata.cpp
    src/VaultVerifier.cpp
    src/Vault.cpp
    src/VaultCrypto.cpp
    src/UnlockDialog.cpp
    src/CreateVaultDialog.cpp
    src/AppConfig.cpp
    include/MainWindow.h
    include/EntryDialog.h
    include/AppConfig.h
    include/VaultMetadata.h
    include/VaultVerifier.h
    include/Vault.h
    include/MasterKey.h
    include/VaultCrypto.h
    include/UnlockDialog.h
    include/CreateVaultDialog.h
)

target_include_directories(PasswordManager
    PRIVATE
        include
)

target_link_libraries(PasswordManager
    PRIVATE
        SQLite::SQLite3
        Qt6::Widgets
        libsodium::libsodium
)

enable_testing()


add_executable(VaultMetadataTest
    tests/VaultMetadataTests.cpp
    src/VaultMetadata.cpp
    src/MasterKey.cpp
    src/SqliteStatement.cpp
    src/SqliteDatabase.cpp
    src/VaultVerifier.cpp
    src/Vault.cpp
    src/VaultCrypto.cpp
)

target_include_directories(VaultMetadataTest
    PRIVATE
        include
)

target_link_libraries(VaultMetadataTest
    PRIVATE
        GTest::gtest_main
        SQLite::SQLite3
        libsodium::libsodium
)

add_executable(VaultVerifierTest
    tests/VaultVerifierTests.cpp
    src/VaultVerifier.cpp
    src/MasterKey.cpp
)

target_include_directories(VaultVerifierTest
    PRIVATE
        include
)

target_link_libraries(VaultVerifierTest
    PRIVATE
        GTest::gtest_main
        SQLite::SQLite3
        libsodium::libsodium
)


add_executable(VaultTest
    tests/VaultTests.cpp
    src/Vault.cpp
    src/VaultMetadata.cpp
    src/MasterKey.cpp
    src/SqliteStatement.cpp
    src/SqliteDatabase.cpp
    src/VaultVerifier.cpp
    src/VaultDatabase.cpp
    src/VaultCrypto.cpp
)

target_include_directories(VaultTest
    PRIVATE
        include
)

target_link_libraries(VaultTest
    PRIVATE
        GTest::gtest_main
        SQLite::SQLite3
        libsodium::libsodium
)


add_executable(VaultCryptoTest
    tests/VaultCryptoTests.cpp
    src/VaultCrypto.cpp
    src/MasterKey.cpp
)

target_include_directories(VaultCryptoTest
    PRIVATE
        include
)

target_link_libraries(VaultCryptoTest
    PRIVATE
        GTest::gtest_main
        SQLite::SQLite3
        libsodium::libsodium
)


add_executable(VaultDatabaseTests
    tests/VaultDatabaseTests.cpp
    src/VaultDatabase.cpp
    src/SqliteStatement.cpp
    src/SqliteDatabase.cpp
    src/MasterKey.cpp
    src/VaultVerifier.cpp
    src/VaultMetadata.cpp
    src/Vault.cpp
    src/VaultCrypto.cpp
)

add_executable(MasterKeyTests
    tests/MasterKeyTests.cpp
    src/MasterKey.cpp
)


target_include_directories(MasterKeyTests
    PRIVATE
        include
)

target_link_libraries(MasterKeyTests
    PRIVATE
        GTest::gtest_main
        libsodium::libsodium
)

target_include_directories(VaultDatabaseTests
    PRIVATE
        include
)

target_link_libraries(VaultDatabaseTests
    PRIVATE
        SQLite::SQLite3
        GTest::gtest_main
        libsodium::libsodium
)


add_executable(AppConfigTest
    tests/AppConfigTest.cpp
    src/AppConfig.cpp
    src/MasterKey.cpp
)

target_include_directories(AppConfigTest
    PRIVATE
        include
)

target_link_libraries(AppConfigTest
    PRIVATE
        GTest::gtest_main
        libsodium::libsodium
        Qt6::Widgets
)

include(GoogleTest)

gtest_discover_tests(AppConfigTest)
gtest_discover_tests(VaultMetadataTest)
gtest_discover_tests(VaultDatabaseTests)
gtest_discover_tests(MasterKeyTests)