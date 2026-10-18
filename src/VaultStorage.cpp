#include "VaultStorage.h"
#include "VaultCrypto.h"
#include <fstream>
#include <algorithm>
#include <iostream>
#include <ctime>
#include <windows.h>

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

    m_settings.autoLockMinutes = 5;
    m_settings.autoBackupDays = 7;
    m_settings.lastBackupTimestamp = 0;
    m_settings.lastPinChangeTimestamp = static_cast<int64_t>(std::time(nullptr));
    m_settings.theme = "light";
    m_settings.usePassphraseMode = false;
    m_settings.clipboardClearSeconds = 120;

    // Create starter welcome items
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

void VaultStorage::RecordPinChanged() {
    m_settings.lastPinChangeTimestamp = static_cast<int64_t>(std::time(nullptr));
}

bool VaultStorage::ChangePin(const std::string& currentPin, const std::string& newPin) {
    if (!VerifyPin(currentPin)) return false;
    m_salt = VaultCrypto::GenerateRandomBytes(16);
    m_pinHash = VaultCrypto::HashPin(newPin, m_salt);
    RecordPinChanged();
    return Save(newPin);
}

bool VaultStorage::Load(const std::string& pin) {
    if (!Exists()) {
        InitializeNewVault(pin.empty() ? "1234" : pin);
        return true;
    }

    std::vector<std::string> pathsToTry = { m_filePath, m_filePath + ".tmp", "data/vault_recovery.dat" };
    bool loadedSuccessfully = false;

    for (const auto& path : pathsToTry) {
        std::ifstream inFile(path);
        if (!inFile.is_open()) continue;

        std::string jsonStr((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
        inFile.close();
        if (jsonStr.empty()) continue;

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
                continue;
            }

            nlohmann::json inner = nlohmann::json::parse(plainJson);

            // Load settings
            if (inner.contains("settings") && inner["settings"].is_object()) {
                const auto& sj = inner["settings"];
                m_settings.autoLockMinutes = sj.value("autoLockMinutes", 5);
                m_settings.autoBackupDays = sj.value("autoBackupDays", 7);
                m_settings.lastBackupTimestamp = sj.value("lastBackupTimestamp", (int64_t)0);
                m_settings.lastPinChangeTimestamp = sj.value("lastPinChangeTimestamp", (int64_t)0);
                m_settings.theme = sj.value("theme", "light");
                m_settings.usePassphraseMode = sj.value("usePassphraseMode", false);
                m_settings.clipboardClearSeconds = sj.value("clipboardClearSeconds", 120);
            }

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

            loadedSuccessfully = true;
            break;
        } catch (...) {
            continue;
        }
    }

    return loadedSuccessfully;
}

bool VaultStorage::SaveToFile(const std::string& path, const std::string& pin) {
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

    nlohmann::json sj;
    sj["autoLockMinutes"] = m_settings.autoLockMinutes;
    sj["autoBackupDays"] = m_settings.autoBackupDays;
    sj["lastBackupTimestamp"] = m_settings.lastBackupTimestamp;
    sj["lastPinChangeTimestamp"] = m_settings.lastPinChangeTimestamp;
    sj["theme"] = m_settings.theme;
    sj["usePassphraseMode"] = m_settings.usePassphraseMode;
    sj["clipboardClearSeconds"] = m_settings.clipboardClearSeconds;
    inner["settings"] = sj;

    nlohmann::json itemsArr = nlohmann::json::array();
    for (const auto& item : m_items) {
        itemsArr.push_back(item.ToJson());
    }
    inner["items"] = itemsArr;

    std::string innerStr = inner.dump();
    std::vector<uint8_t> cipher = VaultCrypto::Encrypt(innerStr, key, iv);

    nlohmann::json root;
    root["version"] = 2;
    root["salt"] = VaultCrypto::ToHex(m_salt);
    root["iv"] = VaultCrypto::ToHex(iv);
    root["pinHash"] = m_pinHash;
    root["ciphertext"] = VaultCrypto::ToHex(cipher);

    // Atomic write using a temp file first to prevent crash corruption
    std::string tempPath = path + ".tmp";
    {
        std::ofstream outFile(tempPath, std::ios::out | std::ios::trunc);
        if (!outFile.is_open()) return false;
        outFile << root.dump(2);
        outFile.flush();
        outFile.close();
    }

    // Atomic rename/move
    if (!MoveFileExA(tempPath.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        // Fallback standard overwrite if rename fails
        std::ofstream fallback(path, std::ios::out | std::ios::trunc);
        if (!fallback.is_open()) return false;
        fallback << root.dump(2);
        fallback.close();
    }

    // Keep crash recovery copy
    std::string recoveryPath = "data/vault_recovery.dat";
    if (path != recoveryPath) {
        CopyFileA(path.c_str(), recoveryPath.c_str(), FALSE);
    }

    return true;
}

bool VaultStorage::Save(const std::string& pin) {
    return SaveToFile(m_filePath, pin);
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

void VaultStorage::SortAlphabetical(bool aToZ) {
    std::sort(m_items.begin(), m_items.end(), [aToZ](const VaultItem& a, const VaultItem& b) {
        std::string lA = a.label;
        std::string lB = b.label;
        std::transform(lA.begin(), lA.end(), lA.begin(), ::tolower);
        std::transform(lB.begin(), lB.end(), lB.begin(), ::tolower);
        if (aToZ) {
            return lA < lB;
        } else {
            return lA > lB;
        }
    });
    for (size_t i = 0; i < m_items.size(); ++i) {
        m_items[i].sortOrder = static_cast<int>(i);
    }
}

void VaultStorage::SortByType() {
    std::sort(m_items.begin(), m_items.end(), [](const VaultItem& a, const VaultItem& b) {
        if (a.type != b.type) {
            return static_cast<int>(a.type) < static_cast<int>(b.type);
        }
        if (a.tabName != b.tabName) {
            return a.tabName < b.tabName;
        }
        std::string lA = a.label;
        std::string lB = b.label;
        std::transform(lA.begin(), lA.end(), lA.begin(), ::tolower);
        std::transform(lB.begin(), lB.end(), lB.begin(), ::tolower);
        return lA < lB;
    });
    for (size_t i = 0; i < m_items.size(); ++i) {
        m_items[i].sortOrder = static_cast<int>(i);
    }
}

void VaultStorage::SortByTags(bool sortItemsAlphabetical) {
    std::sort(m_items.begin(), m_items.end(), [sortItemsAlphabetical](const VaultItem& a, const VaultItem& b) {
        std::string tA = a.tabName;
        std::string tB = b.tabName;
        std::transform(tA.begin(), tA.end(), tA.begin(), ::tolower);
        std::transform(tB.begin(), tB.end(), tB.begin(), ::tolower);

        if (tA != tB) {
            return tA < tB;
        }

        if (sortItemsAlphabetical) {
            std::string lA = a.label;
            std::string lB = b.label;
            std::transform(lA.begin(), lA.end(), lA.begin(), ::tolower);
            std::transform(lB.begin(), lB.end(), lB.begin(), ::tolower);
            if (lA != lB) return lA < lB;
            return a.submittedTimestamp < b.submittedTimestamp;
        } else {
            std::string typeA = AccountTypeToString(a.type);
            std::string typeB = AccountTypeToString(b.type);
            if (typeA != typeB) {
                return typeA < typeB;
            }
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
    std::vector<VaultItem> remaining;
    std::vector<VaultItem> tabItems;

    for (const auto& item : m_items) {
        if (tabName == "All" || item.tabName == tabName) {
            tabItems.push_back(item);
        } else {
            remaining.push_back(item);
        }
    }

    std::vector<VaultItem> reordered;
    for (const auto& id : itemIdsInOrder) {
        for (const auto& item : tabItems) {
            if (item.id == id) {
                reordered.push_back(item);
                break;
            }
        }
    }

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

// Undo / Redo
void VaultStorage::PushUndoState(const std::string& actionName) {
    VaultSnapshot snap;
    snap.items = m_items;
    snap.tabs = m_tabs;
    snap.actionName = actionName;
    m_undoStack.push_back(snap);
    if (m_undoStack.size() > 30) {
        m_undoStack.erase(m_undoStack.begin());
    }
    m_redoStack.clear();
}

bool VaultStorage::CanUndo() const {
    return !m_undoStack.empty();
}

bool VaultStorage::CanRedo() const {
    return !m_redoStack.empty();
}

std::string VaultStorage::GetUndoAction() const {
    if (m_undoStack.empty()) return "";
    return m_undoStack.back().actionName;
}

std::string VaultStorage::GetRedoAction() const {
    if (m_redoStack.empty()) return "";
    return m_redoStack.back().actionName;
}

bool VaultStorage::Undo() {
    if (m_undoStack.empty()) return false;
    VaultSnapshot cur;
    cur.items = m_items;
    cur.tabs = m_tabs;
    cur.actionName = m_undoStack.back().actionName;
    m_redoStack.push_back(cur);

    m_items = m_undoStack.back().items;
    m_tabs = m_undoStack.back().tabs;
    m_undoStack.pop_back();
    return true;
}

bool VaultStorage::Redo() {
    if (m_redoStack.empty()) return false;
    VaultSnapshot cur;
    cur.items = m_items;
    cur.tabs = m_tabs;
    cur.actionName = m_redoStack.back().actionName;
    m_undoStack.push_back(cur);

    m_items = m_redoStack.back().items;
    m_tabs = m_redoStack.back().tabs;
    m_redoStack.pop_back();
    return true;
}

// Binary header signature for backups: non-printable binary format
static const uint8_t BACKUP_MAGIC[8] = { 0x89, 'E', 'C', 'V', 'A', 'U', 'L', 'T' };

bool VaultStorage::CreateBackup(const std::string& backupFilePath, const std::string& encryptionKey, std::string& outError) {
    if (encryptionKey.empty()) {
        outError = "Encryption key cannot be empty.";
        return false;
    }

    try {
        nlohmann::json root;
        root["app"] = "EC_Lab_Password_Locker";
        root["version"] = 2;
        root["backupTimestamp"] = static_cast<int64_t>(std::time(nullptr));

        nlohmann::json sj;
        sj["autoLockMinutes"] = m_settings.autoLockMinutes;
        sj["autoBackupDays"] = m_settings.autoBackupDays;
        sj["lastBackupTimestamp"] = m_settings.lastBackupTimestamp;
        sj["lastPinChangeTimestamp"] = m_settings.lastPinChangeTimestamp;
        sj["theme"] = m_settings.theme;
        sj["usePassphraseMode"] = m_settings.usePassphraseMode;
        sj["clipboardClearSeconds"] = m_settings.clipboardClearSeconds;
        root["settings"] = sj;

        root["tabs"] = m_tabs;

        nlohmann::json itemsArr = nlohmann::json::array();
        for (const auto& item : m_items) {
            itemsArr.push_back(item.ToJson());
        }
        root["items"] = itemsArr;

        std::string rawJson = root.dump();

        // High security derivation for backup: 25,000 PBKDF2 iterations
        std::vector<uint8_t> salt = VaultCrypto::GenerateRandomBytes(16);
        std::vector<uint8_t> iv = VaultCrypto::GenerateRandomBytes(16);
        std::vector<uint8_t> key = VaultCrypto::DeriveKey(encryptionKey, salt, 25000);

        std::vector<uint8_t> ciphertext = VaultCrypto::Encrypt(rawJson, key, iv);
        if (ciphertext.empty()) {
            outError = "Encryption failed.";
            return false;
        }

        // Write binary container format:
        // [8B Magic] [4B SaltLen] [Salt] [4B IVLen] [IV] [4B CipherLen] [Ciphertext]
        std::ofstream out(backupFilePath, std::ios::binary | std::ios::out | std::ios::trunc);
        if (!out.is_open()) {
            outError = "Could not open target backup file for writing.";
            return false;
        }

        out.write(reinterpret_cast<const char*>(BACKUP_MAGIC), sizeof(BACKUP_MAGIC));

        uint32_t saltLen = static_cast<uint32_t>(salt.size());
        out.write(reinterpret_cast<const char*>(&saltLen), sizeof(saltLen));
        out.write(reinterpret_cast<const char*>(salt.data()), saltLen);

        uint32_t ivLen = static_cast<uint32_t>(iv.size());
        out.write(reinterpret_cast<const char*>(&ivLen), sizeof(ivLen));
        out.write(reinterpret_cast<const char*>(iv.data()), ivLen);

        uint32_t cipherLen = static_cast<uint32_t>(ciphertext.size());
        out.write(reinterpret_cast<const char*>(&cipherLen), sizeof(cipherLen));
        out.write(reinterpret_cast<const char*>(ciphertext.data()), cipherLen);

        out.flush();
        out.close();
        return true;
    } catch (const std::exception& ex) {
        outError = ex.what();
        return false;
    }
}

bool VaultStorage::RestoreBackup(const std::string& backupFilePath, const std::string& encryptionKey, std::string& outError, int& outRestoredItemsCount, int& outRestoredTabsCount) {
    outRestoredItemsCount = 0;
    outRestoredTabsCount = 0;

    std::ifstream in(backupFilePath, std::ios::binary | std::ios::in);
    if (!in.is_open()) {
        outError = "Could not open backup file.";
        return false;
    }

    uint8_t magic[8];
    in.read(reinterpret_cast<char*>(magic), sizeof(magic));
    if (in.gcount() != sizeof(magic) || memcmp(magic, BACKUP_MAGIC, sizeof(BACKUP_MAGIC)) != 0) {
        outError = "Invalid backup file format or corrupt header signature.";
        return false;
    }

    uint32_t saltLen = 0;
    in.read(reinterpret_cast<char*>(&saltLen), sizeof(saltLen));
    if (saltLen != 16) {
        outError = "Corrupted backup salt size.";
        return false;
    }
    std::vector<uint8_t> salt(saltLen);
    in.read(reinterpret_cast<char*>(salt.data()), saltLen);

    uint32_t ivLen = 0;
    in.read(reinterpret_cast<char*>(&ivLen), sizeof(ivLen));
    if (ivLen != 16) {
        outError = "Corrupted backup IV size.";
        return false;
    }
    std::vector<uint8_t> iv(ivLen);
    in.read(reinterpret_cast<char*>(iv.data()), ivLen);

    uint32_t cipherLen = 0;
    in.read(reinterpret_cast<char*>(&cipherLen), sizeof(cipherLen));
    if (cipherLen == 0 || cipherLen > 100 * 1024 * 1024) { // Max 100 MB check
        outError = "Corrupted backup payload size.";
        return false;
    }
    std::vector<uint8_t> ciphertext(cipherLen);
    in.read(reinterpret_cast<char*>(ciphertext.data()), cipherLen);
    in.close();

    std::vector<uint8_t> key = VaultCrypto::DeriveKey(encryptionKey, salt, 25000);
    std::string decrypted = VaultCrypto::Decrypt(ciphertext, key, iv);
    if (decrypted.empty()) {
        outError = "Incorrect backup decryption key or corrupted backup data.";
        return false;
    }

    try {
        nlohmann::json root = nlohmann::json::parse(decrypted);

        // Push undo state before restoring
        PushUndoState("Restore Backup");

        if (root.contains("tabs") && root["tabs"].is_array()) {
            m_tabs.clear();
            for (const auto& t : root["tabs"]) {
                m_tabs.push_back(t.get<std::string>());
            }
            EnsureDefaultTabs();
        }

        if (root.contains("items") && root["items"].is_array()) {
            m_items.clear();
            for (const auto& itemJson : root["items"]) {
                m_items.push_back(VaultItem::FromJson(itemJson));
            }
        }

        if (root.contains("settings") && root["settings"].is_object()) {
            const auto& sj = root["settings"];
            m_settings.autoLockMinutes = sj.value("autoLockMinutes", 5);
            m_settings.autoBackupDays = sj.value("autoBackupDays", 7);
            m_settings.theme = sj.value("theme", "light");
            m_settings.usePassphraseMode = sj.value("usePassphraseMode", false);
            m_settings.clipboardClearSeconds = sj.value("clipboardClearSeconds", 120);
        }

        outRestoredItemsCount = static_cast<int>(m_items.size());
        outRestoredTabsCount = static_cast<int>(m_tabs.size());
        return true;
    } catch (const std::exception& ex) {
        outError = std::string("Failed to parse restored data: ") + ex.what();
        return false;
    }
}

bool VaultStorage::CheckAndPerformAutoBackup(const std::string& currentPin) {
    if (m_settings.autoBackupDays <= 0) return false;

    int64_t now = static_cast<int64_t>(std::time(nullptr));
    int64_t intervalSeconds = static_cast<int64_t>(m_settings.autoBackupDays) * 86400LL;

    if (m_settings.lastBackupTimestamp > 0 && (now - m_settings.lastBackupTimestamp) < intervalSeconds) {
        return false;
    }

    // Create backups directory
    CreateDirectoryA("data", NULL);
    CreateDirectoryA("data\\backups", NULL);

    char timeBuf[64];
    std::time_t tNow = std::time(nullptr);
    std::tm* tmInfo = std::localtime(&tNow);
    std::strftime(timeBuf, sizeof(timeBuf), "%Y%m%d_%H%M%S", tmInfo);

    std::string backupPath = "data/backups/auto_backup_" + std::string(timeBuf) + ".ecvault";
    std::string err;
    if (CreateBackup(backupPath, currentPin, err)) {
        m_settings.lastBackupTimestamp = now;
        Save(currentPin);
        return true;
    }
    return false;
}
