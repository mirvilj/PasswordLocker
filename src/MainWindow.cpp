#include "MainWindow.h"
#include "ItemCardPanel.h"
#include "EntryDialog.h"
#include "DetailDialog.h"
#include "PinDialog.h"
#include "TotpHubDialog.h"
#include "TotpEngine.h"
#include <algorithm>

const int ID_NEW_ENTRY = 1001;
const int ID_NEW_TAB = 1002;
const int ID_TOTP_HUB = 1003;
const int ID_SORT_TIME = 1004;
const int ID_LOCK_VAULT = 1005;
const int ID_CHANGE_PIN = 1006;
const int ID_SEARCH = 1007;

wxBEGIN_EVENT_TABLE(MainWindow, wxFrame)
    EVT_BUTTON(ID_NEW_ENTRY, MainWindow::OnNewEntry)
    EVT_BUTTON(ID_NEW_TAB, MainWindow::OnAddNewTab)
    EVT_BUTTON(ID_TOTP_HUB, MainWindow::OnOpenTotpHub)
    EVT_BUTTON(ID_SORT_TIME, MainWindow::OnSortByTime)
    EVT_BUTTON(ID_LOCK_VAULT, MainWindow::OnLockVault)
    EVT_BUTTON(ID_CHANGE_PIN, MainWindow::OnChangePin)
    EVT_TEXT(ID_SEARCH, MainWindow::OnSearchUpdated)
wxEND_EVENT_TABLE()

MainWindow::MainWindow(const wxString& title)
    : wxFrame(nullptr, wxID_ANY, "Password Locker", wxDefaultPosition, wxSize(980, 720)),
      m_storage("data/vault.dat"),
      m_currentPin("1234"),
      m_isUnlockedSession(false),
      m_zoomLevel(1.0),
      m_zoomText(nullptr),
      m_notebook(nullptr),
      m_searchCtrl(nullptr),
      m_statusLabel(nullptr),
      m_itemCountLabel(nullptr) {

    SetMinSize(wxSize(800, 540));
    SetBackgroundColour(wxColour(241, 245, 249));

    // Initialize or load offline vault
    if (!m_storage.Exists()) {
        m_storage.Load("1234");
    } else {
        m_storage.Load("1234");
    }

    SetupUI();
    SetupNotebookTabs();
    RefreshAllTabs();

    // Mouse wheel zoom support with Ctrl key
    Bind(wxEVT_MOUSEWHEEL, &MainWindow::OnMouseWheel, this);
    if (m_notebook) {
        m_notebook->Bind(wxEVT_MOUSEWHEEL, &MainWindow::OnMouseWheel, this);
    }
}

MainWindow::~MainWindow() {
}

void MainWindow::SetupUI() {
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // 1. Top Modern Header Bar
    wxPanel* headerPanel = new wxPanel(this, wxID_ANY);
    headerPanel->SetBackgroundColour(wxColour(15, 23, 42)); // Slate 900
    wxBoxSizer* headerSizer = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer* topRow = new wxBoxSizer(wxHORIZONTAL);

    wxStaticText* appTitle = new wxStaticText(headerPanel, wxID_ANY, "Password Locker");
    appTitle->SetForegroundColour(*wxWHITE);
    wxFont tf = appTitle->GetFont();
    tf.SetPointSize(14);
    tf.SetWeight(wxFONTWEIGHT_BOLD);
    appTitle->SetFont(tf);

    m_statusLabel = new wxStaticText(headerPanel, wxID_ANY, "Vault: Locked");
    m_statusLabel->SetForegroundColour(wxColour(251, 191, 36)); // Amber
    wxFont sf = m_statusLabel->GetFont();
    sf.SetWeight(wxFONTWEIGHT_BOLD);
    m_statusLabel->SetFont(sf);

    topRow->Add(appTitle, 0, wxALIGN_CENTER_VERTICAL | wxALL, 10);
    topRow->Add(m_statusLabel, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 8);
    topRow->AddStretchSpacer();

    // Toolbar action buttons
    wxButton* newEntryBtn = new wxButton(headerPanel, ID_NEW_ENTRY, "+ New Account", wxDefaultPosition, wxSize(120, 32));
    newEntryBtn->SetBackgroundColour(wxColour(37, 99, 235));
    newEntryBtn->SetForegroundColour(*wxWHITE);

    wxButton* newTabBtn = new wxButton(headerPanel, ID_NEW_TAB, "+ New Tab", wxDefaultPosition, wxSize(95, 32));
    wxButton* totpHubBtn = new wxButton(headerPanel, ID_TOTP_HUB, "Authenticator", wxDefaultPosition, wxSize(110, 32));
    wxButton* sortBtn = new wxButton(headerPanel, ID_SORT_TIME, "Sort by Time", wxDefaultPosition, wxSize(105, 32));
    wxButton* lockBtn = new wxButton(headerPanel, ID_LOCK_VAULT, "Lock Vault", wxDefaultPosition, wxSize(90, 32));
    wxButton* pinBtn = new wxButton(headerPanel, ID_CHANGE_PIN, "PIN Settings", wxDefaultPosition, wxSize(95, 32));

    topRow->Add(newEntryBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);
    topRow->Add(newTabBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);
    topRow->Add(totpHubBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);
    topRow->Add(sortBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);
    topRow->Add(lockBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);
    topRow->Add(pinBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);

    headerSizer->Add(topRow, 0, wxEXPAND);

    // Filter / Search Bar Row with Zoom Controls
    wxBoxSizer* searchRow = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* searchPrompt = new wxStaticText(headerPanel, wxID_ANY, "Filter Accounts:");
    searchPrompt->SetForegroundColour(wxColour(148, 163, 184));

    m_searchCtrl = new wxTextCtrl(headerPanel, ID_SEARCH, "", wxDefaultPosition, wxSize(260, 26));
    m_searchCtrl->SetHint("Search accounts, usernames, emails...");

    m_itemCountLabel = new wxStaticText(headerPanel, wxID_ANY, "0 accounts listed");
    m_itemCountLabel->SetForegroundColour(wxColour(148, 163, 184));

    searchRow->Add(searchPrompt, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 10);
    searchRow->Add(m_searchCtrl, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 12);
    searchRow->Add(m_itemCountLabel, 0, wxALIGN_CENTER_VERTICAL);

    searchRow->AddStretchSpacer();

    // Zoom tool controls
    wxStaticText* zoomPrompt = new wxStaticText(headerPanel, wxID_ANY, "Zoom (Ctrl+Scroll):");
    zoomPrompt->SetForegroundColour(wxColour(148, 163, 184));

    wxButton* zoomOutBtn = new wxButton(headerPanel, wxID_ANY, " - ", wxDefaultPosition, wxSize(28, 26));
    zoomOutBtn->SetToolTip("Zoom Out");
    zoomOutBtn->Bind(wxEVT_BUTTON, &MainWindow::OnZoomOut, this);

    m_zoomText = new wxStaticText(headerPanel, wxID_ANY, "100%", wxDefaultPosition, wxSize(45, -1), wxALIGN_CENTER_HORIZONTAL);
    m_zoomText->SetForegroundColour(*wxWHITE);
    wxFont zf = m_zoomText->GetFont();
    zf.SetWeight(wxFONTWEIGHT_BOLD);
    m_zoomText->SetFont(zf);

    wxButton* zoomInBtn = new wxButton(headerPanel, wxID_ANY, " + ", wxDefaultPosition, wxSize(28, 26));
    zoomInBtn->SetToolTip("Zoom In (up to 200%)");
    zoomInBtn->Bind(wxEVT_BUTTON, &MainWindow::OnZoomIn, this);

    wxButton* zoomResetBtn = new wxButton(headerPanel, wxID_ANY, "Reset", wxDefaultPosition, wxSize(50, 26));
    zoomResetBtn->SetToolTip("Reset zoom to 100%");
    zoomResetBtn->Bind(wxEVT_BUTTON, &MainWindow::OnZoomReset, this);

    searchRow->Add(zoomPrompt, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);
    searchRow->Add(zoomOutBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);
    searchRow->Add(m_zoomText, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);
    searchRow->Add(zoomInBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);
    searchRow->Add(zoomResetBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);

    headerSizer->Add(searchRow, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);
    headerPanel->SetSizer(headerSizer);
    mainSizer->Add(headerPanel, 0, wxEXPAND);

    // 2. Dynamic Notebook Tabs
    m_notebook = new wxNotebook(this, wxID_ANY);
    m_notebook->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &MainWindow::OnNotebookPageChanged, this);
    mainSizer->Add(m_notebook, 1, wxEXPAND | wxALL, 8);

    SetSizer(mainSizer);
    Center();
}

void MainWindow::SetupNotebookTabs() {
    m_notebook->DeleteAllPages();
    m_tabPages.clear();

    // Standard preset tabs
    std::vector<std::string> tabList = {
        "All Items",
        "Basic Passwords",
        "Security Questions",
        "Miscellaneous"
    };

    // User-created custom tabs
    for (const auto& ct : m_storage.GetCustomTabs()) {
        if (std::find(tabList.begin(), tabList.end(), ct) == tabList.end()) {
            tabList.push_back(ct);
        }
    }

    for (const auto& tabName : tabList) {
        wxPanel* pagePanel = new wxPanel(m_notebook, wxID_ANY);
        pagePanel->SetBackgroundColour(wxColour(241, 245, 249));
        wxBoxSizer* pageSizer = new wxBoxSizer(wxVERTICAL);

        wxScrolledWindow* scrollWin = new wxScrolledWindow(pagePanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL);
        scrollWin->SetScrollRate(0, 10);
        wxBoxSizer* cardsSizer = new wxBoxSizer(wxVERTICAL);
        scrollWin->SetSizer(cardsSizer);

        pageSizer->Add(scrollWin, 1, wxEXPAND | wxALL, 6);
        pagePanel->SetSizer(pageSizer);

        m_notebook->AddPage(pagePanel, wxString::FromUTF8(tabName.c_str()));

        TabPageInfo info;
        info.tabName = tabName;
        info.scrollWin = scrollWin;
        info.cardsSizer = cardsSizer;
        m_tabPages.push_back(info);
    }
}

std::string MainWindow::GetCurrentTabName() const {
    int sel = m_notebook->GetSelection();
    if (sel >= 0 && sel < static_cast<int>(m_tabPages.size())) {
        return m_tabPages[sel].tabName;
    }
    return "All Items";
}

void MainWindow::RefreshCurrentTabList() {
    int sel = m_notebook->GetSelection();
    if (sel < 0 || sel >= static_cast<int>(m_tabPages.size())) return;

    auto& pageInfo = m_tabPages[sel];
    pageInfo.cardsSizer->Clear(true);

    std::string search = m_searchCtrl->GetValue().ToStdString();
    std::string tab = pageInfo.tabName;
    if (tab == "All Items") tab = "All";

    auto items = m_storage.GetFilteredItems(tab, search);

    int totalCount = static_cast<int>(items.size());
    m_itemCountLabel->SetLabel(wxString::Format("%d account%s listed", totalCount, totalCount == 1 ? "" : "s"));

    if (items.empty()) {
        wxPanel* emptyCard = new wxPanel(pageInfo.scrollWin, wxID_ANY);
        emptyCard->SetBackgroundColour(*wxWHITE);
        wxBoxSizer* emptySizer = new wxBoxSizer(wxVERTICAL);

        wxStaticText* emptyTxt = new wxStaticText(emptyCard, wxID_ANY,
            "No accounts found in this section.\n\nClick '+ New Account' above to add an entry, or switch tabs.");
        emptyTxt->SetForegroundColour(wxColour(100, 116, 139));
        wxFont ef = emptyTxt->GetFont();
        ef.SetPointSize(11);
        emptyTxt->SetFont(ef);

        emptySizer->Add(emptyTxt, 0, wxALL | wxALIGN_CENTER, 30);
        emptyCard->SetSizer(emptySizer);
        pageInfo.cardsSizer->Add(emptyCard, 0, wxEXPAND | wxALL, 12);
    } else {
        std::vector<std::string> allTabs = m_storage.GetCustomTabs();
        for (int i = 0; i < totalCount; ++i) {
            VaultItem* itm = items[i];
            ItemCardPanel* card = new ItemCardPanel(
                pageInfo.scrollWin,
                *itm,
                i,
                totalCount,
                [this](const std::string& id) { OnUnlockItem(id); },
                [this](const std::string& id) { OnDeleteItem(id); },
                [this](const std::string& id, int dir) { OnMoveItemRelative(id, dir); },
                [this](const std::string& dId, const std::string& tId) { OnDropReorder(dId, tId); },
                [this](const std::string& id, const std::string& targetTab) { OnMoveItemToTab(id, targetTab); },
                allTabs,
                m_zoomLevel
            );
            pageInfo.cardsSizer->Add(card, 0, wxEXPAND | wxBOTTOM, 8);
        }
    }

    pageInfo.scrollWin->Bind(wxEVT_MOUSEWHEEL, &MainWindow::OnMouseWheel, this);
    pageInfo.scrollWin->FitInside();
    pageInfo.scrollWin->Layout();
}

void MainWindow::SetZoom(double zoom) {
    m_zoomLevel = std::max(0.8, std::min(2.0, zoom));
    if (m_zoomText) {
        m_zoomText->SetLabel(wxString::Format("%d%%", static_cast<int>(m_zoomLevel * 100.0 + 0.5)));
    }
    RefreshCurrentTabList();
}

void MainWindow::OnZoomIn(wxCommandEvent& evt) {
    SetZoom(m_zoomLevel + 0.1);
}

void MainWindow::OnZoomOut(wxCommandEvent& evt) {
    SetZoom(m_zoomLevel - 0.1);
}

void MainWindow::OnZoomReset(wxCommandEvent& evt) {
    SetZoom(1.0);
}

void MainWindow::OnMouseWheel(wxMouseEvent& evt) {
    if (evt.ControlDown()) {
        int rot = evt.GetWheelRotation();
        if (rot > 0) {
            SetZoom(m_zoomLevel + 0.1);
        } else if (rot < 0) {
            SetZoom(m_zoomLevel - 0.1);
        }
        return;
    }
    evt.Skip();
}

void MainWindow::RefreshAllTabs() {
    RefreshCurrentTabList();
}

bool MainWindow::PromptAndVerifyPin(const std::string& purposeLabel) {
    if (m_isUnlockedSession) return true;

    PinDialog dlg(this, PinDialogMode::UnlockItem, purposeLabel);
    if (dlg.ShowModal() == wxID_OK) {
        std::string enteredPin = dlg.GetEnteredPin();
        if (m_storage.VerifyPin(enteredPin)) {
            m_currentPin = enteredPin;
            m_isUnlockedSession = true;
            m_statusLabel->SetLabel("Vault: Unlocked");
            m_statusLabel->SetForegroundColour(wxColour(34, 197, 94)); // Green
            return true;
        } else {
            wxMessageBox("Incorrect PIN. Access denied.", "Authentication Failed", wxOK | wxICON_ERROR, this);
            return false;
        }
    }
    return false;
}

void MainWindow::OnNewEntry(wxCommandEvent& evt) {
    std::string curTab = GetCurrentTabName();
    if (curTab == "All Items" || curTab == "Basic Passwords" || curTab == "Security Questions" || curTab == "Miscellaneous") {
        curTab = "General";
    }

    EntryDialog dlg(this, m_storage.GetCustomTabs(), curTab);
    if (dlg.ShowModal() == wxID_OK) {
        VaultItem newItem = dlg.GetVaultItem();
        m_storage.AddItem(newItem);
        m_storage.Save(m_currentPin);

        // Ensure newly created tab is tracked
        if (m_storage.AddTab(newItem.tabName)) {
            SetupNotebookTabs();
        }
        RefreshAllTabs();
        wxMessageBox("Account '" + wxString::FromUTF8(newItem.label.c_str()) + "' submitted and recorded successfully!", "Account Added", wxOK | wxICON_INFORMATION, this);
    }
}

void MainWindow::OnAddNewTab(wxCommandEvent& evt) {
    wxTextEntryDialog dlg(this, "Enter name for new Tab / Section:", "Create New Section Tab");
    if (dlg.ShowModal() == wxID_OK) {
        wxString newTab = dlg.GetValue().Trim();
        if (!newTab.empty()) {
            if (m_storage.AddTab(newTab.ToStdString())) {
                m_storage.Save(m_currentPin);
                SetupNotebookTabs();
                // Switch to newly created tab
                for (size_t i = 0; i < m_tabPages.size(); ++i) {
                    if (m_tabPages[i].tabName == newTab.ToStdString()) {
                        m_notebook->SetSelection(static_cast<int>(i));
                        break;
                    }
                }
                RefreshAllTabs();
            } else {
                wxMessageBox("Tab already exists.", "Notice", wxOK | wxICON_WARNING, this);
            }
        }
    }
}

void MainWindow::OnOpenTotpHub(wxCommandEvent& evt) {
    if (!PromptAndVerifyPin("2-Factor Authenticator Hub")) return;

    TotpHubDialog dlg(this, m_storage, m_currentPin);
    dlg.ShowModal();
    RefreshAllTabs();
}

void MainWindow::OnSortByTime(wxCommandEvent& evt) {
    // Automatically sort all entries according to submission timestamp (newest first)
    m_storage.SortByTimestamp(true);
    m_storage.Save(m_currentPin);
    RefreshAllTabs();
    wxMessageBox("List automatically ordered by submission timestamp (newest first).", "Order Updated", wxOK | wxICON_INFORMATION, this);
}

void MainWindow::OnLockVault(wxCommandEvent& evt) {
    m_isUnlockedSession = false;
    m_statusLabel->SetLabel("Vault: Locked");
    m_statusLabel->SetForegroundColour(wxColour(251, 191, 36)); // Amber
    RefreshAllTabs();
    wxMessageBox("Vault locked. PIN will be required to view account details.", "Vault Locked", wxOK | wxICON_INFORMATION, this);
}

void MainWindow::OnChangePin(wxCommandEvent& evt) {
    PinDialog dlg(this, PinDialogMode::ChangePin);
    if (dlg.ShowModal() == wxID_OK) {
        std::string cur = dlg.GetEnteredPin();
        std::string nw = dlg.GetNewPin();
        if (m_storage.ChangePin(cur, nw)) {
            m_currentPin = nw;
            wxMessageBox("Master PIN changed successfully!", "Success", wxOK | wxICON_INFORMATION, this);
        } else {
            wxMessageBox("Current PIN was incorrect.", "Error", wxOK | wxICON_ERROR, this);
        }
    }
}

void MainWindow::OnSearchUpdated(wxCommandEvent& evt) {
    RefreshCurrentTabList();
}

void MainWindow::OnNotebookPageChanged(wxBookCtrlEvent& evt) {
    RefreshCurrentTabList();
    evt.Skip();
}

void MainWindow::OnUnlockItem(const std::string& itemId) {
    VaultItem* item = m_storage.GetItemById(itemId);
    if (!item) return;

    if (!PromptAndVerifyPin(item->label)) return;

    DetailDialog dlg(this, *item);
    dlg.ShowModal();

    if (dlg.WasItemModified()) {
        m_storage.UpdateItem(dlg.GetItem());
        m_storage.Save(m_currentPin);
        RefreshAllTabs();
    }

    if (dlg.GetSelectedAction() == DetailAction_Edit) {
        EntryDialog editDlg(this, m_storage.GetCustomTabs(), item->tabName, item);
        if (editDlg.ShowModal() == wxID_OK) {
            VaultItem updated = editDlg.GetVaultItem();
            m_storage.UpdateItem(updated);
            m_storage.Save(m_currentPin);
            RefreshAllTabs();
        }
    } else if (dlg.GetSelectedAction() == DetailAction_Delete) {
        m_storage.DeleteItem(itemId);
        m_storage.Save(m_currentPin);
        RefreshAllTabs();
    }
}

void MainWindow::OnDeleteItem(const std::string& itemId) {
    VaultItem* item = m_storage.GetItemById(itemId);
    if (!item) return;

    int res = wxMessageBox(
        "Are you sure you want to permanently delete '" + wxString::FromUTF8(item->label.c_str()) + "'?",
        "Confirm Remove",
        wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION,
        this
    );
    if (res == wxYES) {
        m_storage.DeleteItem(itemId);
        m_storage.Save(m_currentPin);
        RefreshAllTabs();
    }
}

void MainWindow::OnMoveItemRelative(const std::string& itemId, int direction) {
    const auto& items = m_storage.GetAllItems();
    int curIdx = -1;
    for (size_t i = 0; i < items.size(); ++i) {
        if (items[i].id == itemId) {
            curIdx = static_cast<int>(i);
            break;
        }
    }
    if (curIdx < 0) return;

    int targetIdx = curIdx + direction;
    if (targetIdx >= 0 && targetIdx < static_cast<int>(items.size())) {
        m_storage.MoveItem(curIdx, targetIdx);
        m_storage.Save(m_currentPin);
        RefreshAllTabs();
    }
}

void MainWindow::OnDropReorder(const std::string& draggedId, const std::string& targetId) {
    const auto& items = m_storage.GetAllItems();
    int fromIdx = -1;
    int toIdx = -1;
    for (size_t i = 0; i < items.size(); ++i) {
        if (items[i].id == draggedId) fromIdx = static_cast<int>(i);
        if (items[i].id == targetId) toIdx = static_cast<int>(i);
    }
    if (fromIdx >= 0 && toIdx >= 0 && fromIdx != toIdx) {
        m_storage.MoveItem(fromIdx, toIdx);
        m_storage.Save(m_currentPin);
        RefreshAllTabs();
    }
}

void MainWindow::OnMoveItemToTab(const std::string& itemId, const std::string& targetTab) {
    m_storage.MoveItemToTab(itemId, targetTab);
    m_storage.Save(m_currentPin);
    RefreshAllTabs();
}
