#include "TotpEngine.h"
#include <ctime>
#include <random>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <windows.h>
#include <bcrypt.h>

#pragma comment(lib, "bcrypt.lib")

std::vector<uint8_t> TotpEngine::Base32Decode(const std::string& input) {
    std::vector<uint8_t> result;
    int buffer = 0;
    int bitsLeft = 0;

    for (char c : input) {
        if (c == ' ' || c == '-' || c == '=') continue;
        int val = -1;
        if (c >= 'A' && c <= 'Z') val = c - 'A';
        else if (c >= 'a' && c <= 'z') val = c - 'a';
        else if (c >= '2' && c <= '7') val = c - '2' + 26;

        if (val < 0) continue;

        buffer = (buffer << 5) | val;
        bitsLeft += 5;

        if (bitsLeft >= 8) {
            bitsLeft -= 8;
            result.push_back(static_cast<uint8_t>((buffer >> bitsLeft) & 0xFF));
        }
    }
    return result;
}

bool TotpEngine::IsValidBase32(const std::string& secret) {
    std::string clean;
    for (char c : secret) {
        if (c == ' ' || c == '-' || c == '=') continue;
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '2' && c <= '7'))) {
            return false;
        }
        clean.push_back(c);
    }
    return !clean.empty();
}

std::vector<uint8_t> TotpEngine::HmacSha1(const std::vector<uint8_t>& key, const uint8_t* data, size_t dataLen) {
    BCRYPT_ALG_HANDLE hAlg = NULL;
    BCRYPT_HASH_HANDLE hHash = NULL;
    std::vector<uint8_t> hash(20, 0);

    NTSTATUS status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA1_ALGORITHM, NULL, BCRYPT_ALG_HANDLE_HMAC_FLAG);
    if (status != 0) return hash;

    status = BCryptCreateHash(hAlg, &hHash, NULL, 0, (PUCHAR)key.data(), static_cast<ULONG>(key.size()), 0);
    if (status == 0) {
        BCryptHashData(hHash, (PUCHAR)data, static_cast<ULONG>(dataLen), 0);
        BCryptFinishHash(hHash, hash.data(), static_cast<ULONG>(hash.size()), 0);
        BCryptDestroyHash(hHash);
    }
    BCryptCloseAlgorithmProvider(hAlg, 0);
    return hash;
}

std::string TotpEngine::GenerateCode(const std::string& base32Secret, int64_t timestamp, int period) {
    std::vector<uint8_t> key = Base32Decode(base32Secret);
    if (key.empty()) {
        return "------";
    }

    if (timestamp < 0) {
        timestamp = static_cast<int64_t>(time(nullptr));
    }

    uint64_t counter = static_cast<uint64_t>(timestamp / period);
    uint8_t msg[8];
    for (int i = 7; i >= 0; --i) {
        msg[i] = static_cast<uint8_t>(counter & 0xFF);
        counter >>= 8;
    }

    std::vector<uint8_t> hash = HmacSha1(key, msg, 8);
    if (hash.size() < 20) {
        return "------";
    }

    int offset = hash[19] & 0x0F;
    uint32_t binary = ((hash[offset] & 0x7F) << 24) |
                      ((hash[offset + 1] & 0xFF) << 16) |
                      ((hash[offset + 2] & 0xFF) << 8) |
                      (hash[offset + 3] & 0xFF);

    uint32_t otp = binary % 1000000;
    std::ostringstream ss;
    ss << std::setw(6) << std::setfill('0') << otp;
    return ss.str();
}

std::string TotpEngine::FormatCode(const std::string& code) {
    if (code.length() == 6) {
        return code.substr(0, 3) + " " + code.substr(3);
    }
    return code;
}

int TotpEngine::GetRemainingSeconds(int period, int64_t timestamp) {
    if (timestamp < 0) {
        timestamp = static_cast<int64_t>(time(nullptr));
    }
    int rem = period - static_cast<int>(timestamp % period);
    if (rem <= 0) rem = period;
    return rem;
}

std::string TotpEngine::GenerateRandomSecret(int length) {
    static const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(0, 31);

    std::string secret;
    secret.reserve(length);
    for (int i = 0; i < length; ++i) {
        secret.push_back(alphabet[dis(gen)]);
    }
    return secret;
}
