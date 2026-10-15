#ifndef VAULT_ITEM_H
#define VAULT_ITEM_H

#include <string>
#include <vector>
#include <cstdint>
#include "json.hpp"

enum class AccountType {
    Basic = 0,
    SecurityQuestions = 1,
    Miscellaneous = 2
};

std::string AccountTypeToString(AccountType type);
AccountType StringToAccountType(const std::string& str);

struct SecurityQuestion {
    std::string question;
    std::string answer;
};

struct CustomField {
    std::string label;
    std::string value;
};

class VaultItem {
public:
    std::string id;
    AccountType type;
    std::string label;
    std::string username;
    std::string email;
    std::string password;
    std::string website;
    std::string notes;
    std::string totpSecret;
    std::string tabName;               // Target category / tab section
    int64_t submittedTimestamp;        // Unix timestamp when submitted
    std::string submittedTimeFormatted;// Bold display string
    int sortOrder;                     // Used for manual drag-and-drop grouping

    std::vector<SecurityQuestion> securityQuestions;
    std::vector<CustomField> customFields;

    std::string attachedImageBase64;   // Encrypted Base64 image attachment
    std::string attachedImageName;     // Original image filename

    VaultItem();

    // Factory method to create an item with current timestamp
    static VaultItem CreateNew(AccountType type, const std::string& label, const std::string& tab = "General");

    // Serialization
    nlohmann::json ToJson() const;
    static VaultItem FromJson(const nlohmann::json& j);

    // Formats submission timestamp to human readable bold representation
    static std::string FormatTimestamp(int64_t ts);
};

#endif // VAULT_ITEM_H
