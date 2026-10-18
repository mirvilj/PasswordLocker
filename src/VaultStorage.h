#ifndef VAULT_STORAGE_H
#define VAULT_STORAGE_H

#include "VaultItem.h"
#include <string>
#include <vector>
#include <memory>
<<<<<<< HEAD
#include <cstdint>

struct VaultSettings {
    int autoLockMinutes = 5;            // 1, 2, 5, 10, 15, 30 (0 = disabled)
    int autoBackupDays = 7;             // 1, 3, 7, 30 (0 = disabled)
    int64_t lastBackupTimestamp = 0;    // Unix timestamp of last backup
    int64_t lastPinChangeTimestamp = 0; // Unix timestamp when PIN/passphrase was changed
    std::string theme = "light";        // "light", "dark", "high_contrast"
    bool usePassphraseMode = false;     // false = 4-digit PIN, true = passphrase (6-63 chars)
    int clipboardClearSeconds = 120;    // 2 minutes auto-clear
};

struct VaultSnapshot {
    std::vector<VaultItem> items;
    std::vector<std::string> tabs;
    std::string actionName;
};
=======
>>>>>>> origin

class VaultStorage {
public:
    VaultStorage(const std::string& dataFilePath = "data/vault.dat");

    // Initialization & Encryption
    bool Load(const std::string& pin);
    bool Save(const std::string& pin);
    bool Exists() const;
    bool NeedsInitialPin() const;
    bool VerifyPin(const std::string& pin);
    bool ChangePin(const std::string& currentPin, const std::string& newPin);

<<<<<<< HEAD
    // Settings
    const VaultSettings& GetSettings() const { return m_settings; }
    void UpdateSettings(const VaultSettings& s) { m_settings = s; }
    void RecordPinChanged();

=======
>>>>>>> origin
    // Item Operations
    void AddItem(VaultItem item);
    bool UpdateItem(const VaultItem& item);
    bool DeleteItem(const std::string& id);
    VaultItem* GetItemById(const std::string& id);
    const std::vector<VaultItem>& GetAllItems() const { return m_items; }

    // Sorting & Reordering (Drag-and-Drop)
    void SortByTimestamp(bool newestFirst = true);
<<<<<<< HEAD
    void SortAlphabetical(bool aToZ = true);
    void SortByType();
    void SortByTags(bool sortItemsAlphabetical = false);
=======
>>>>>>> origin
    void MoveItem(size_t fromIndex, size_t toIndex);
    void ReorderTabItems(const std::string& tabName, const std::vector<std::string>& itemIdsInOrder);

    // Tab / Category Management
    const std::vector<std::string>& GetCustomTabs() const { return m_tabs; }
    bool AddTab(const std::string& tabName);
    bool RemoveTab(const std::string& tabName);
    void MoveItemToTab(const std::string& itemId, const std::string& targetTab);

    // Filtering items
    std::vector<VaultItem*> GetFilteredItems(const std::string& tabName, const std::string& searchFilter = "");

<<<<<<< HEAD
    // Undo / Redo
    void PushUndoState(const std::string& actionName);
    bool CanUndo() const;
    bool CanRedo() const;
    std::string GetUndoAction() const;
    std::string GetRedoAction() const;
    bool Undo();
    bool Redo();

    // Backup & Restore
    bool CreateBackup(const std::string& backupFilePath, const std::string& encryptionKey, std::string& outError);
    bool RestoreBackup(const std::string& backupFilePath, const std::string& encryptionKey, std::string& outError, int& outRestoredItemsCount, int& outRestoredTabsCount);
    bool CheckAndPerformAutoBackup(const std::string& currentPin);

=======
>>>>>>> origin
    // Path helper
    std::string GetFilePath() const { return m_filePath; }

private:
    std::string m_filePath;
    std::vector<VaultItem> m_items;
    std::vector<std::string> m_tabs;
<<<<<<< HEAD
    VaultSettings m_settings;
    std::vector<uint8_t> m_salt;
    std::string m_pinHash;

    // Undo / Redo stacks
    std::vector<VaultSnapshot> m_undoStack;
    std::vector<VaultSnapshot> m_redoStack;

    void EnsureDefaultTabs();
    void InitializeNewVault(const std::string& initialPin);
    bool SaveToFile(const std::string& path, const std::string& pin);
=======
    std::vector<uint8_t> m_salt;
    std::string m_pinHash;

    void EnsureDefaultTabs();
    void InitializeNewVault(const std::string& initialPin);
>>>>>>> origin
};

#endif // VAULT_STORAGE_H
