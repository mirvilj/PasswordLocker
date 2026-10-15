#ifndef VAULT_STORAGE_H
#define VAULT_STORAGE_H

#include "VaultItem.h"
#include <string>
#include <vector>
#include <memory>

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

    // Item Operations
    void AddItem(VaultItem item);
    bool UpdateItem(const VaultItem& item);
    bool DeleteItem(const std::string& id);
    VaultItem* GetItemById(const std::string& id);
    const std::vector<VaultItem>& GetAllItems() const { return m_items; }

    // Sorting & Reordering (Drag-and-Drop)
    void SortByTimestamp(bool newestFirst = true);
    void MoveItem(size_t fromIndex, size_t toIndex);
    void ReorderTabItems(const std::string& tabName, const std::vector<std::string>& itemIdsInOrder);

    // Tab / Category Management
    const std::vector<std::string>& GetCustomTabs() const { return m_tabs; }
    bool AddTab(const std::string& tabName);
    bool RemoveTab(const std::string& tabName);
    void MoveItemToTab(const std::string& itemId, const std::string& targetTab);

    // Filtering items
    std::vector<VaultItem*> GetFilteredItems(const std::string& tabName, const std::string& searchFilter = "");

    // Path helper
    std::string GetFilePath() const { return m_filePath; }

private:
    std::string m_filePath;
    std::vector<VaultItem> m_items;
    std::vector<std::string> m_tabs;
    std::vector<uint8_t> m_salt;
    std::string m_pinHash;

    void EnsureDefaultTabs();
    void InitializeNewVault(const std::string& initialPin);
};

#endif // VAULT_STORAGE_H
