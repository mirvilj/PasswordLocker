#ifndef VAULT_CRYPTO_H
#define VAULT_CRYPTO_H

#include <string>
#include <vector>
#include <cstdint>

class VaultCrypto {
public:
    // Derives an AES-256 key from a PIN and salt using PBKDF2-SHA256
    static std::vector<uint8_t> DeriveKey(const std::string& pin, const std::vector<uint8_t>& salt, int iterations = 10000);

    // Computes a SHA-256 hash of (salt + pin)
    static std::string HashPin(const std::string& pin, const std::vector<uint8_t>& salt);

    // Verifies a PIN against stored hash and salt
    static bool VerifyPin(const std::string& pin, const std::string& storedHash, const std::vector<uint8_t>& salt);

    // Encrypts plaintext string using AES-256-CBC with PKCS7 padding
    static std::vector<uint8_t> Encrypt(const std::string& plaintext, const std::vector<uint8_t>& key, const std::vector<uint8_t>& iv);

    // Decrypts ciphertext using AES-256-CBC with PKCS7 padding
    static std::string Decrypt(const std::vector<uint8_t>& ciphertext, const std::vector<uint8_t>& key, const std::vector<uint8_t>& iv);

    // Generates cryptographically secure random bytes
    static std::vector<uint8_t> GenerateRandomBytes(size_t length);

    // Utility: Hex string encoding and decoding
    static std::string ToHex(const std::vector<uint8_t>& data);
    static std::vector<uint8_t> FromHex(const std::string& hex);

    // Utility: Base64 encoding and decoding for image and file attachments
    static std::string Base64Encode(const std::vector<uint8_t>& data);
    static std::vector<uint8_t> Base64Decode(const std::string& base64);
};

#endif // VAULT_CRYPTO_H
