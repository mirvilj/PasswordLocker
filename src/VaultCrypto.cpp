#include "VaultCrypto.h"
#include <windows.h>
#include <bcrypt.h>
#include <sstream>
#include <iomanip>

#pragma comment(lib, "bcrypt.lib")

std::vector<uint8_t> VaultCrypto::GenerateRandomBytes(size_t length) {
    std::vector<uint8_t> buffer(length, 0);
    BCryptGenRandom(NULL, buffer.data(), static_cast<ULONG>(length), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    return buffer;
}

std::vector<uint8_t> VaultCrypto::DeriveKey(const std::string& pin, const std::vector<uint8_t>& salt, int iterations) {
    BCRYPT_ALG_HANDLE hAlg = NULL;
    std::vector<uint8_t> key(32, 0); // 256-bit key

    if (BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, NULL, BCRYPT_ALG_HANDLE_HMAC_FLAG) == 0) {
        BCryptDeriveKeyPBKDF2(hAlg, (PUCHAR)pin.data(), static_cast<ULONG>(pin.size()),
                              (PUCHAR)salt.data(), static_cast<ULONG>(salt.size()),
                              iterations, key.data(), static_cast<ULONG>(key.size()), 0);
        BCryptCloseAlgorithmProvider(hAlg, 0);
    }
    return key;
}

std::string VaultCrypto::HashPin(const std::string& pin, const std::vector<uint8_t>& salt) {
    BCRYPT_ALG_HANDLE hAlg = NULL;
    BCRYPT_HASH_HANDLE hHash = NULL;
    std::vector<uint8_t> hash(32, 0);

    if (BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, NULL, 0) == 0) {
        if (BCryptCreateHash(hAlg, &hHash, NULL, 0, NULL, 0, 0) == 0) {
            BCryptHashData(hHash, (PUCHAR)salt.data(), static_cast<ULONG>(salt.size()), 0);
            BCryptHashData(hHash, (PUCHAR)pin.data(), static_cast<ULONG>(pin.size()), 0);
            BCryptFinishHash(hHash, hash.data(), static_cast<ULONG>(hash.size()), 0);
            BCryptDestroyHash(hHash);
        }
        BCryptCloseAlgorithmProvider(hAlg, 0);
    }
    return ToHex(hash);
}

bool VaultCrypto::VerifyPin(const std::string& pin, const std::string& storedHash, const std::vector<uint8_t>& salt) {
    std::string computed = HashPin(pin, salt);
    return computed == storedHash;
}

std::vector<uint8_t> VaultCrypto::Encrypt(const std::string& plaintext, const std::vector<uint8_t>& key, const std::vector<uint8_t>& iv) {
    BCRYPT_ALG_HANDLE hAlg = NULL;
    BCRYPT_KEY_HANDLE hKey = NULL;
    std::vector<uint8_t> ciphertext;

    if (BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_AES_ALGORITHM, NULL, 0) == 0) {
        BCryptSetProperty(hAlg, BCRYPT_CHAINING_MODE, (PUCHAR)BCRYPT_CHAIN_MODE_CBC, sizeof(BCRYPT_CHAIN_MODE_CBC), 0);
        if (BCryptGenerateSymmetricKey(hAlg, &hKey, NULL, 0, (PUCHAR)key.data(), static_cast<ULONG>(key.size()), 0) == 0) {
            // PKCS7 padding to 16 bytes
            std::vector<uint8_t> padded(plaintext.begin(), plaintext.end());
            uint8_t padVal = 16 - static_cast<uint8_t>(padded.size() % 16);
            padded.insert(padded.end(), padVal, padVal);

            std::vector<uint8_t> ivCopy = iv;
            ULONG outLen = 0;
            BCryptEncrypt(hKey, padded.data(), static_cast<ULONG>(padded.size()), NULL, ivCopy.data(), static_cast<ULONG>(ivCopy.size()), NULL, 0, &outLen, 0);
            ciphertext.resize(outLen);
            ivCopy = iv;
            BCryptEncrypt(hKey, padded.data(), static_cast<ULONG>(padded.size()), NULL, ivCopy.data(), static_cast<ULONG>(ivCopy.size()), ciphertext.data(), outLen, &outLen, 0);
            BCryptDestroyKey(hKey);
        }
        BCryptCloseAlgorithmProvider(hAlg, 0);
    }
    return ciphertext;
}

std::string VaultCrypto::Decrypt(const std::vector<uint8_t>& ciphertext, const std::vector<uint8_t>& key, const std::vector<uint8_t>& iv) {
    BCRYPT_ALG_HANDLE hAlg = NULL;
    BCRYPT_KEY_HANDLE hKey = NULL;
    std::string plaintext;

    if (ciphertext.empty()) return "";

    if (BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_AES_ALGORITHM, NULL, 0) == 0) {
        BCryptSetProperty(hAlg, BCRYPT_CHAINING_MODE, (PUCHAR)BCRYPT_CHAIN_MODE_CBC, sizeof(BCRYPT_CHAIN_MODE_CBC), 0);
        if (BCryptGenerateSymmetricKey(hAlg, &hKey, NULL, 0, (PUCHAR)key.data(), static_cast<ULONG>(key.size()), 0) == 0) {
            std::vector<uint8_t> ivCopy = iv;
            ULONG outLen = 0;
            std::vector<uint8_t> dec(ciphertext.size());
            if (BCryptDecrypt(hKey, (PUCHAR)ciphertext.data(), static_cast<ULONG>(ciphertext.size()), NULL, ivCopy.data(), static_cast<ULONG>(ivCopy.size()), dec.data(), static_cast<ULONG>(dec.size()), &outLen, 0) == 0) {
                if (outLen > 0) {
                    uint8_t padVal = dec[outLen - 1];
                    if (padVal >= 1 && padVal <= 16 && outLen >= padVal) {
                        plaintext.assign(reinterpret_cast<char*>(dec.data()), outLen - padVal);
                    }
                }
            }
            BCryptDestroyKey(hKey);
        }
        BCryptCloseAlgorithmProvider(hAlg, 0);
    }
    return plaintext;
}

std::string VaultCrypto::ToHex(const std::vector<uint8_t>& data) {
    std::ostringstream ss;
    for (uint8_t b : data) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
    }
    return ss.str();
}

std::vector<uint8_t> VaultCrypto::FromHex(const std::string& hex) {
    std::vector<uint8_t> result;
    if (hex.length() % 2 != 0) return result;
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        uint8_t byte = static_cast<uint8_t>(strtol(byteString.c_str(), nullptr, 16));
        result.push_back(byte);
    }
    return result;
}

static const char b64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string VaultCrypto::Base64Encode(const std::vector<uint8_t>& data) {
    std::string ret;
    int val = 0, valb = -6;
    for (uint8_t c : data) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            ret.push_back(b64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) ret.push_back(b64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    while (ret.size() % 4) ret.push_back('=');
    return ret;
}

std::vector<uint8_t> VaultCrypto::Base64Decode(const std::string& base64) {
    std::vector<uint8_t> ret;
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++) T[b64_chars[i]] = i;

    int val = 0, valb = -8;
    for (char c : base64) {
        if (c == '=') break;
        if (T[(unsigned char)c] == -1) continue;
        val = (val << 6) + T[(unsigned char)c];
        valb += 6;
        if (valb >= 0) {
            ret.push_back(uint8_t((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return ret;
}
