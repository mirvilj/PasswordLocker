#ifndef TOTP_ENGINE_H
#define TOTP_ENGINE_H

#include <string>
#include <vector>
#include <cstdint>

class TotpEngine {
public:
    // Decodes a Base32-encoded string into raw bytes
    static std::vector<uint8_t> Base32Decode(const std::string& input);

    // Checks if the string is a valid Base32 secret
    static bool IsValidBase32(const std::string& secret);

    // Computes HMAC-SHA1 using Windows BCrypt API
    static std::vector<uint8_t> HmacSha1(const std::vector<uint8_t>& key, const uint8_t* data, size_t dataLen);

    // Generates a 6-digit TOTP code for the specified Unix timestamp (default current time)
    static std::string GenerateCode(const std::string& base32Secret, int64_t timestamp = -1, int period = 30);

    // Formats a 6-digit code with a space (e.g. "123 456")
    static std::string FormatCode(const std::string& code);

    // Returns seconds remaining until next rotation (0 to 29)
    static int GetRemainingSeconds(int period = 30, int64_t timestamp = -1);

    // Generates a random Base32 secret key for testing or new 2FA setup (16 characters)
    static std::string GenerateRandomSecret(int length = 16);
};

#endif // TOTP_ENGINE_H
