#include "VaultStorage.h"
#include "VaultCrypto.h"
#include <fstream>
#include <algorithm>
#include <iostream>

VaultStorage::VaultStorage(const std::string& dataFilePath)
    : m_filePath(dataFilePath) {
    EnsureDefaultTabs();
}

void VaultStorage::EnsureDefaultTabs() {
    if (m_tabs.empty()) {
        m_tabs = { "General", "Personal", "Work", "Finances" };
    }
}

bool VaultStorage::Exists() const {
    std::ifstream f(m_filePath.c_str());
    return f.good();
}

bool VaultStorage::NeedsInitialPin() const {
    return !Exists();
}

void VaultStorage::InitializeNewVault(const std::string& initialPin) {
    m_salt = VaultCrypto::GenerateRandomBytes(16);
    m_pinHash = VaultCrypto::HashPin(initialPin, m_salt);
    m_items.clear();
    EnsureDefaultTabs();

    // Create a starter welcome item
    VaultItem sample = VaultItem::CreateNew(AccountType::Basic, "Sample App Account", "General");
    sample.username = "johndoe";
    sample.email = "john.doe@example.com";
    sample.password = "Secur3P@ssw0rd!2026";
    sample.website = "https://example.com";
    sample.notes = "This is a sample password entry. Click Unlock to view full details.";
    sample.totpSecret = "JBSWY3DPEHPK3PXP"; // Standard RFC test key
    m_items.push_back(sample);

    VaultItem sampleSq = VaultItem::CreateNew(AccountType::SecurityQuestions, "Online Banking Portal", "Finances");
    sampleSq.username = "bank_user";
    sampleSq.password = "VaultBank#9912";
    sampleSq.securityQuestions.push_back({ "What was the name of your first school?", "Lincoln Elementary" });
    sampleSq.securityQuestions.push_back({ "What is your mother's maiden name?", "Smith" });
    m_items.push_back(sampleSq);

    VaultItem sampleMisc = VaultItem::CreateNew(AccountType::Miscellaneous, "Cloud Server Access & License", "Work");
    sampleMisc.customFields.push_back({ "Server IP", "192.168.1.105" });
    sampleMisc.customFields.push_back({ "SSH Port", "2222" });
    sampleMisc.customFields.push_back({ "License Key", "XXXX-YYYY-ZZZZ-2026" });
    sampleMisc.notes = "Production bastion host credentials.";
    m_items.push_back(sampleMisc);

    // Default sort by timestamp
    SortByTimestamp(true);
    Save(initialPin);
}

bool VaultStorage::VerifyPin(const std::string& pin) {
    if (m_salt.empty() || m_pinHash.empty()) {
        if (!Load(pin)) return false;
    }
    return VaultCrypto::VerifyPin(pin, m_pinHash, m_salt);
}

bool VaultStorage::ChangePin(const std::string& currentPin, const std::string& newPin) {
    if (!VerifyPin(currentPin)) return false;
    m_salt = VaultCrypto::GenerateRandomBytes(16);
    m_pinHash = VaultCrypto::HashPin(newPin, m_salt);
    return Save(newPin);
}

bool VaultStorage::Load(const std::string& pin) {
    if (!Exists()) {
        InitializeNewVault(pin.empty() ? "1234" : pin);
        return true;
    }

    std::ifstream inFile(m_filePath);
    if (!inFile.is_open()) return false;

    std::string jsonStr((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
    inFile.close();

    try {
        nlohmann::json root = nlohmann::json::parse(jsonStr);
        std::string saltHex = root.value("salt", "");
        std::string ivHex = root.value("iv", "");
        m_pinHash = root.value("pinHash", "");
        std::string cipherHex = root.value("ciphertext", "");

        m_salt = VaultCrypto::FromHex(saltHex);
        std::vector<uint8_t> iv = VaultCrypto::FromHex(ivHex);
        std::vector<uint8_t> cipher = VaultCrypto::FromHex(cipherHex);

        if (!pin.empty() && !m_pinHash.empty()) {
            if (!VaultCrypto::VerifyPin(pin, m_pinHash, m_salt)) {
                return false; // Invalid PIN
            }
        }

        std::vector<uint8_t> key = VaultCrypto::DeriveKey(pin, m_salt);
        std::string plainJson = VaultCrypto::Decrypt(cipher, key, iv);
        if (plainJson.empty()) {
            return false;
        }

        nlohmann::json inner = nlohmann::json::parse(plainJson);
        m_tabs.clear();
        if (inner.contains("tabs") && inner["tabs"].is_array()) {
            for (const auto& t : inner["tabs"]) {
                m_tabs.push_back(t.get<std::string>());
            }
        }
        EnsureDefaultTabs();

        m_items.clear();
        if (inner.contains("items") && inner["items"].is_array()) {
            for (const auto& itemJson : inner["items"]) {
                m_items.push_back(VaultItem::FromJson(itemJson));
            }
        }

        return true;
    } catch (...) {
        return false;
    }
}

bool VaultStorage::Save(const std::string& pin) {
    if (m_salt.empty()) {
        m_salt = VaultCrypto::GenerateRandomBytes(16);
    }
    if (m_pinHash.empty() && !pin.empty()) {
        m_pinHash = VaultCrypto::HashPin(pin, m_salt);
    }

    std::vector<uint8_t> iv = VaultCrypto::GenerateRandomBytes(16);
    std::vector<uint8_t> key = VaultCrypto::DeriveKey(pin, m_salt);

    nlohmann::json inner;
    inner["tabs"] = m_tabs;

    nlohmann::json itemsArr = nlohmann::json::array();
    for (const auto& item : m_items) {
        itemsArr.push_back(item.ToJson());
    }
    inner["items"] = itemsArr;

    std::string innerStr = inner.dump();
    std::vector<uint8_t> cipher = VaultCrypto::Encrypt(innerStr, key, iv);

    nlohmann::json root;
    root["version"] = 1;
    root["salt"] = VaultCrypto::ToHex(m_salt);
    root["iv"] = VaultCrypto::ToHex(iv);
    root["pinHash"] = m_pinHash;
    root["ciphertext"] = VaultCrypto::ToHex(cipher);

    std::ofstream outFile(m_filePath);
    if (!outFile.is_open()) return false;
    outFile << root.dump(2);
    outFile.close();
    return true;
}

void VaultStorage::AddItem(VaultItem item) {
    if (item.submittedTimestamp == 0) {
        item.submittedTimestamp = static_cast<int64_t>(std::time(nullptr));
        item.submittedTimeFormatted = VaultItem::FormatTimestamp(item.submittedTimestamp);
    }
    item.sortOrder = static_cast<int>(m_items.size());
    m_items.push_back(item);
}

bool VaultStorage::UpdateItem(const VaultItem& updated) {
    for (size_t i = 0; i < m_items.size(); ++i) {
        if (m_items[i].id == updated.id) {
            // Keep original submission timestamp so order integrity is preserved
            VaultItem itemCopy = updated;
            if (itemCopy.submittedTimestamp == 0) {
                itemCopy.submittedTimestamp = m_items[i].submittedTimestamp;
                itemCopy.submittedTimeFormatted = m_items[i].submittedTimeFormatted;
            }
            m_items[i] = itemCopy;
            return true;
        }
    }
    return false;
}

bool VaultStorage::DeleteItem(const std::string& id) {
    auto it = std::remove_if(m_items.begin(), m_items.end(), [&id](const VaultItem& item) {
        return item.id == id;
    });
    if (it != m_items.end()) {
        m_items.erase(it, m_items.end());
        return true;
    }
    return false;
}

VaultItem* VaultStorage::GetItemById(const std::string& id) {
    for (auto& item : m_items) {
        if (item.id == id) {
            return &item;
        }
    }
    return nullptr;
}

void VaultStorage::SortByTimestamp(bool newestFirst) {
    std::sort(m_items.begin(), m_items.end(), [newestFirst](const VaultItem& a, const VaultItem& b) {
        if (newestFirst) {
            return a.submittedTimestamp > b.submittedTimestamp;
        } else {
            return a.submittedTimestamp < b.submittedTimestamp;
        }
    });
    for (size_t i = 0; i < m_items.size(); ++i) {
        m_items[i].sortOrder = static_cast<int>(i);
    }
}

void VaultStorage::MoveItem(size_t fromIndex, size_t toIndex) {
    if (fromIndex >= m_items.size() || toIndex >= m_items.size() || fromIndex == toIndex) {
        return;
    }
    VaultItem moved = m_items[fromIndex];
    m_items.erase(m_items.begin() + fromIndex);
    m_items.insert(m_items.begin() + toIndex, moved);
    for (size_t i = 0; i < m_items.size(); ++i) {
        m_items[i].sortOrder = static_cast<int>(i);
    }
}

void VaultStorage::ReorderTabItems(const std::string& tabName, const std::vector<std::string>& itemIdsInOrder) {
    // Collect non-tab items
    std::vector<VaultItem> remaining;
    std::vector<VaultItem> tabItems;

    for (const auto& item : m_items) {
        if (tabName == "All" || item.tabName == tabName) {
            tabItems.push_back(item);
        } else {
            remaining.push_back(item);
        }
    }

    // Sort tabItems according to itemIdsInOrder
    std::vector<VaultItem> reordered;
    for (const auto& id : itemIdsInOrder) {
        for (const auto& item : tabItems) {
            if (item.id == id) {
                reordered.push_back(item);
                break;
            }
        }
    }

    // Append any tab items not in itemIdsInOrder
    for (const auto& item : tabItems) {
        bool found = false;
        for (const auto& r : reordered) {
            if (r.id == item.id) { found = true; break; }
        }
        if (!found) reordered.push_back(item);
    }

    if (tabName == "All") {
        m_items = reordered;
    } else {
        m_items = reordered;
        m_items.insert(m_items.end(), remaining.begin(), remaining.end());
    }

    for (size_t i = 0; i < m_items.size(); ++i) {
        m_items[i].sortOrder = static_cast<int>(i);
    }
}

bool VaultStorage::AddTab(const std::string& tabName) {
    if (tabName.empty()) return false;
    for (const auto& t : m_tabs) {
        if (t == tabName) return false;
    }
    m_tabs.push_back(tabName);
    return true;
}

bool VaultStorage::RemoveTab(const std::string& tabName) {
    auto it = std::find(m_tabs.begin(), m_tabs.end(), tabName);
    if (it != m_tabs.end()) {
        m_tabs.erase(it);
        // Move items from removed tab to "General"
        for (auto& item : m_items) {
            if (item.tabName == tabName) {
                item.tabName = "General";
            }
        }
        return true;
    }
    return false;
}

void VaultStorage::MoveItemToTab(const std::string& itemId, const std::string& targetTab) {
    VaultItem* item = GetItemById(itemId);
    if (item) {
        item->tabName = targetTab;
    }
}

std::vector<VaultItem*> VaultStorage::GetFilteredItems(const std::string& tabName, const std::string& searchFilter) {
    std::vector<VaultItem*> result;
    std::string q = searchFilter;
    std::transform(q.begin(), q.end(), q.begin(), ::tolower);

    for (auto& item : m_items) {
        bool matchesTab = (tabName == "All" || tabName.empty() || item.tabName == tabName);
        if (!matchesTab) {
            // Also allow filtering by account type tab names
            if (tabName == "Basic Passwords" && item.type == AccountType::Basic) matchesTab = true;
            else if (tabName == "Security Questions" && item.type == AccountType::SecurityQuestions) matchesTab = true;
            else if (tabName == "Miscellaneous" && item.type == AccountType::Miscellaneous) matchesTab = true;
        }

        if (!matchesTab) continue;

        if (!q.empty()) {
            std::string labelLower = item.label;
            std::transform(labelLower.begin(), labelLower.end(), labelLower.begin(), ::tolower);
            std::string userLower = item.username;
            std::transform(userLower.begin(), userLower.end(), userLower.begin(), ::tolower);
            std::string emailLower = item.email;
            std::transform(emailLower.begin(), emailLower.end(), emailLower.begin(), ::tolower);

            if (labelLower.find(q) == std::string::npos &&
                userLower.find(q) == std::string::npos &&
                emailLower.find(q) == std::string::npos) {
                continue;
            }
        }

        result.push_back(&item);
    }
    return result;
}
