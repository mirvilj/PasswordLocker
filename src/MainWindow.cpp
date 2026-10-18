#include "MainWindow.h"
#include "ItemCardPanel.h"
#include "EntryDialog.h"
#include "DetailDialog.h"
#include "PinDialog.h"
#include "TotpHubDialog.h"
#include "SettingsDialog.h"
#include "BackupRestoreDialog.h"
#include "PasswordAnalyzerDialog.h"
#include "HelpDialog.h"
#include "ThemeManager.h"
#include "ClipboardSanitizer.h"
#include "TotpEngine.h"
#include <algorithm>

// Event IDs
const int ID_NEW_ENTRY = 1001;
const int ID_NEW_TAB = 1002;
const int ID_TOTP_HUB = 1003;
const int ID_LOCK_VAULT = 1005;
const int ID_CHANGE_PIN = 1006;
const int ID_SEARCH = 1007;

// Edit IDs
const int ID_UNDO = 1101;
const int ID_REDO = 1102;

// View / Sort IDs
const int ID_SORT_TIME_NEWEST = 1201;
const int ID_SORT_TIME_OLDEST = 1202;
const int ID_SORT_ALPHA_ASC = 1203;
const int ID_SORT_ALPHA_DESC = 1204;
const int ID_SORT_TYPE = 1205;
const int ID_SORT_TAGS_DEFAULT = 1206;
const int ID_SORT_TAGS_ITEMS = 1207;
const int ID_SORT_MENU = 1208;

// Theme IDs
const int ID_THEME_LIGHT = 1211;
const int ID_THEME_DARK = 1212;
const int ID_THEME_HIGH_CONTRAST = 1213;

// Tab switch range
const int ID_TAB_BASE = 1300;

// Tools IDs
const int ID_BACKUP_VAULT = 1401;
const int ID_RESTORE_VAULT = 1402;
const int ID_PASSWORD_ANALYZER = 1403;

// Configure IDs
const int ID_SETTINGS = 1501;

// Window IDs
const int ID_WINDOW_HOME = 1601;
const int ID_WINDOW_TOTP = 1602;

// Help IDs
const int ID_HELP_DOCS = 1701;
const int ID_HELP_ABOUT = 1702;

// Timer ID
const int ID_TICKER_TIMER = 1801;

// Event Filter for Global Inactivity Monitoring
ActivityEventFilter::ActivityEventFilter(MainWindow* win) : m_win(win) {}

int ActivityEventFilter::FilterEvent(wxEvent& event) {
    wxEventType t = event.GetEventType();
    if (t == wxEVT_MOTION || t == wxEVT_LEFT_DOWN || t == wxEVT_RIGHT_DOWN ||
        t == wxEVT_KEY_DOWN || t == wxEVT_CHAR_HOOK || t == wxEVT_MOUSEWHEEL) {
        if (m_win) {
            m_win->ResetInactivityTimer();
        }
    }
    return Event_Skip;
}

wxBEGIN_EVENT_TABLE(MainWindow, wxFrame)
    EVT_MENU(wxID_NEW, MainWindow::OnNewEntry)
    EVT_MENU(wxID_EXIT, MainWindow::OnExit)
    EVT_MENU(ID_UNDO, MainWindow::OnUndo)
    EVT_MENU(ID_REDO, MainWindow::OnRedo)

    EVT_MENU(ID_SORT_TIME_NEWEST, MainWindow::OnSortByTimeNewest)
    EVT_MENU(ID_SORT_TIME_OLDEST, MainWindow::OnSortByTimeOldest)
    EVT_MENU(ID_SORT_ALPHA_ASC, MainWindow::OnSortAlphabeticalAsc)
    EVT_MENU(ID_SORT_ALPHA_DESC, MainWindow::OnSortAlphabeticalDesc)
    EVT_MENU(ID_SORT_TYPE, MainWindow::OnSortByType)
    EVT_MENU(ID_SORT_TAGS_DEFAULT, MainWindow::OnSortByTagsDefault)
    EVT_MENU(ID_SORT_TAGS_ITEMS, MainWindow::OnSortByTagsAndItems)

    EVT_MENU(ID_THEME_LIGHT, MainWindow::OnSetThemeLight)
    EVT_MENU(ID_THEME_DARK, MainWindow::OnSetThemeDark)
    EVT_MENU(ID_THEME_HIGH_CONTRAST, MainWindow::OnSetThemeHighContrast)

    EVT_MENU(ID_BACKUP_VAULT, MainWindow::OnBackupVault)
    EVT_MENU(ID_RESTORE_VAULT, MainWindow::OnRestoreVault)
    EVT_MENU(ID_PASSWORD_ANALYZER, MainWindow::OnPasswordAnalyzer)

    EVT_MENU(ID_SETTINGS, MainWindow::OnOpenSettings)
    EVT_MENU(ID_CHANGE_PIN, MainWindow::OnChangePin)

    EVT_MENU(ID_WINDOW_HOME, MainWindow::OnWindowHome)
    EVT_MENU(ID_WINDOW_TOTP, MainWindow::OnOpenTotpHub)

    EVT_MENU(ID_HELP_DOCS, MainWindow::OnOpenHelp)
    EVT_MENU(ID_HELP_ABOUT, MainWindow::OnAbout)

    EVT_BUTTON(ID_SORT_MENU, MainWindow::OnSortBtnClicked)
    EVT_BUTTON(ID_SETTINGS, MainWindow::OnOpenSettings)
    EVT_BUTTON(ID_NEW_ENTRY, MainWindow::OnNewEntry)
    EVT_BUTTON(ID_NEW_TAB, MainWindow::OnAddNewTab)
    EVT_BUTTON(ID_TOTP_HUB, MainWindow::OnOpenTotpHub)
    EVT_BUTTON(ID_LOCK_VAULT, MainWindow::OnLockVault)
    EVT_BUTTON(ID_CHANGE_PIN, MainWindow::OnChangePin)
    EVT_TEXT(ID_SEARCH, MainWindow::OnSearchUpdated)

    EVT_TIMER(ID_TICKER_TIMER, MainWindow::OnTickerTimer)
wxEND_EVENT_TABLE()

MainWindow::MainWindow(const wxString&)
    : wxFrame(nullptr, wxID_ANY, "Password Locker", wxDefaultPosition, wxSize(1020, 740)),
      m_storage("data/vault.dat"),
      m_currentPin("1234"),
      m_isUnlockedSession(false),
      m_sortDisplayMode(SortDisplayMode::None),
      m_pinReminderShown(false),
      m_tickerTimer(this, ID_TICKER_TIMER),
      m_lastActivityTime(std::time(nullptr)),
      m_eventFilter(nullptr),
      m_menuBar(nullptr),
      m_fileMenu(nullptr),
      m_editMenu(nullptr),
      m_viewMenu(nullptr),
      m_tabsSubMenu(nullptr),
      m_toolsMenu(nullptr),
      m_configMenu(nullptr),
      m_windowMenu(nullptr),
      m_helpMenu(nullptr),
      m_headerPanel(nullptr),
      m_notebook(nullptr),
      m_searchCtrl(nullptr),
      m_appTitle(nullptr),
      m_statusLabel(nullptr),
      m_itemCountLabel(nullptr),
      m_clipboardTimerLabel(nullptr),
      m_searchPrompt(nullptr),
      m_zoomPrompt(nullptr),
      m_newEntryBtn(nullptr),
      m_newTabBtn(nullptr),
      m_totpHubBtn(nullptr),
      m_sortBtn(nullptr),
      m_lockBtn(nullptr),
      m_settingsBtn(nullptr),
      m_pinBtn(nullptr),
      m_zoomOutBtn(nullptr),
      m_zoomInBtn(nullptr),
      m_zoomResetBtn(nullptr),
      m_zoomLevel(1.0),
      m_zoomText(nullptr) {

    SetMinSize(wxSize(850, 560));
    SetIcon(wxICON(app_icon));

    // Initialize or load offline vault
    if (!m_storage.Exists()) {
        m_storage.Load("1234");
    } else {
        m_storage.Load("1234");
    }

    // Apply persisted theme
    ThemeManager::SetThemeByName(m_storage.GetSettings().theme);

    SetupMenuBar();
    SetupUI();
    ApplyTheme();
    SetupNotebookTabs();
    RefreshAllTabs();
    UpdateEditMenuState();

    // Inactivity Filter
    m_eventFilter = new ActivityEventFilter(this);
    wxEvtHandler::AddFilter(m_eventFilter);

    // Start 1-second ticker timer for inactivity & clipboard clear
    m_tickerTimer.Start(1000);

    // Mouse wheel zoom support with Ctrl key
    Bind(wxEVT_MOUSEWHEEL, &MainWindow::OnMouseWheel, this);
    if (m_notebook) {
        m_notebook->Bind(wxEVT_MOUSEWHEEL, &MainWindow::OnMouseWheel, this);
    }

    // Run auto-backup check on startup
    m_storage.CheckAndPerformAutoBackup(m_currentPin);

    // Check 2-week PIN hygiene reminder
    CheckPinReminder();
}

MainWindow::~MainWindow() {
    if (m_eventFilter) {
        wxEvtHandler::RemoveFilter(m_eventFilter);
        delete m_eventFilter;
        m_eventFilter = nullptr;
    }
    if (m_tickerTimer.IsRunning()) {
        m_tickerTimer.Stop();
    }
}

void MainWindow::ResetInactivityTimer() {
    m_lastActivityTime = std::time(nullptr);
}

void MainWindow::SetupMenuBar() {
    m_menuBar = new wxMenuBar();

    // 1. File Menu
    m_fileMenu = new wxMenu();
    m_fileMenu->Append(wxID_NEW, "&New Account...\tCtrl+N", "Create a new offline vault entry");
    m_fileMenu->Append(ID_LOCK_VAULT, "&Lock Vault\tCtrl+L", "Immediately lock the offline vault");
    m_fileMenu->AppendSeparator();
    m_fileMenu->Append(wxID_EXIT, "E&xit\tAlt+F4", "Exit Password Locker");
    m_menuBar->Append(m_fileMenu, "&File");

    // 2. Edit Menu (Undo / Redo)
    m_editMenu = new wxMenu();
    m_editMenu->Append(ID_UNDO, "&Undo\tCtrl+Z", "Undo last action");
    m_editMenu->Append(ID_REDO, "&Redo\tCtrl+Y", "Redo last undone action");
    m_menuBar->Append(m_editMenu, "&Edit");

    // 3. View Menu
    m_viewMenu = new wxMenu();

    m_tabsSubMenu = new wxMenu();
    m_viewMenu->AppendSubMenu(m_tabsSubMenu, "&Sections / Tabs", "Switch to any vault tab section");

    wxMenu* sortSub = new wxMenu();
    sortSub->Append(ID_SORT_TIME_NEWEST, "Sort by Time Created (Newest First)");
    sortSub->Append(ID_SORT_TIME_OLDEST, "Sort by Time Created (Oldest First)");
    sortSub->Append(ID_SORT_ALPHA_ASC, "Sort Alphabetical (A - Z)");
    sortSub->Append(ID_SORT_ALPHA_DESC, "Sort Alphabetical (Z - A)");
    sortSub->Append(ID_SORT_TYPE, "Sort by Type");
    sortSub->Append(ID_SORT_TAGS_DEFAULT, "Sort Tags Alphabetical (Default: Types A-Z, Items by Date)");
    sortSub->Append(ID_SORT_TAGS_ITEMS, "Sort Tags & Items Alphabetical (Both A-Z)");
    m_viewMenu->AppendSubMenu(sortSub, "&Sort Accounts", "Order vault credentials");

    wxMenu* themeSub = new wxMenu();
    themeSub->Append(ID_THEME_LIGHT, "Minimalist");
    themeSub->Append(ID_THEME_DARK, "Dark Mode");
    themeSub->Append(ID_THEME_HIGH_CONTRAST, "High-Contrast Mode (Accessibility)");
    m_viewMenu->AppendSubMenu(themeSub, "&Appearance / Theme", "Switch visual contrast mode");

    m_viewMenu->AppendSeparator();
    m_viewMenu->Append(wxID_ZOOM_IN, "Zoom In\tCtrl++", "Increase card scale");
    m_viewMenu->Append(wxID_ZOOM_OUT, "Zoom Out\tCtrl+-", "Decrease card scale");
    m_viewMenu->Append(wxID_ZOOM_100, "Reset Zoom (100%)\tCtrl+0", "Reset scale to 100%");
    Bind(wxEVT_MENU, &MainWindow::OnZoomIn, this, wxID_ZOOM_IN);
    Bind(wxEVT_MENU, &MainWindow::OnZoomOut, this, wxID_ZOOM_OUT);
    Bind(wxEVT_MENU, &MainWindow::OnZoomReset, this, wxID_ZOOM_100);

    m_menuBar->Append(m_viewMenu, "&View");

    // 4. Tools Menu (Backup & Restore, Password Analyzer)
    m_toolsMenu = new wxMenu();
    m_toolsMenu->Append(ID_BACKUP_VAULT, "&Backup Vault to File...\tCtrl+B", "Create encrypted .ecvault backup container");
    m_toolsMenu->Append(ID_RESTORE_VAULT, "&Restore Vault from File...\tCtrl+R", "Restore vault from an encrypted .ecvault file");
    m_toolsMenu->AppendSeparator();
    m_toolsMenu->Append(ID_PASSWORD_ANALYZER, "&Password Health & Analyzer...", "Audit reused passwords and weak credentials");
    m_menuBar->Append(m_toolsMenu, "&Tools");

    // 5. Configure Menu
    m_configMenu = new wxMenu();
    m_configMenu->Append(ID_SETTINGS, "&Settings & Preferences...", "Configure timeouts, auto-backups, and security");
    m_configMenu->Append(ID_CHANGE_PIN, "Change Master &PIN / Passphrase...", "Update vault master authentication key");
    m_menuBar->Append(m_configMenu, "&Configure");

    // 6. Window Menu
    m_windowMenu = new wxMenu();
    m_windowMenu->Append(ID_WINDOW_HOME, "&Home (All Accounts)\tCtrl+H", "Switch to All Accounts overview");
    m_windowMenu->Append(ID_WINDOW_TOTP, "&Authenticator Hub\tCtrl+T", "Open offline 2FA Authenticator Hub");
    m_menuBar->Append(m_windowMenu, "&Window");

    // 7. Help Menu
    m_helpMenu = new wxMenu();
    m_helpMenu->Append(ID_HELP_DOCS, "&How to Use Password Locker\tF1", "View complete formatted FAQ chapters");
    m_helpMenu->Append(ID_HELP_ABOUT, "&About Password Locker", "About offline vault security");
    m_menuBar->Append(m_helpMenu, "&Help");

    SetMenuBar(m_menuBar);
}

void MainWindow::SetupUI() {
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // 1. Top Modern Header & Toolbar Bar
    m_headerPanel = new wxPanel(this, wxID_ANY);
    wxBoxSizer* headerSizer = new wxBoxSizer(wxVERTICAL);

    // Top row with title, status, and familiar toolbar action buttons
    wxBoxSizer* topRow = new wxBoxSizer(wxHORIZONTAL);

    m_appTitle = new wxStaticText(m_headerPanel, wxID_ANY, "Password Locker");
    m_appTitle->SetForegroundColour(*wxWHITE);
    wxFont tf = m_appTitle->GetFont();
    tf.SetPointSize(14);
    tf.SetWeight(wxFONTWEIGHT_BOLD);
    m_appTitle->SetFont(tf);

    m_statusLabel = new wxStaticText(m_headerPanel, wxID_ANY, "Vault: Locked");
    wxFont sf = m_statusLabel->GetFont();
    sf.SetWeight(wxFONTWEIGHT_BOLD);
    m_statusLabel->SetFont(sf);

    m_clipboardTimerLabel = new wxStaticText(m_headerPanel, wxID_ANY, "");
    m_clipboardTimerLabel->SetForegroundColour(wxColour(147, 197, 253));
    m_clipboardTimerLabel->Show(false);

    topRow->Add(m_appTitle, 0, wxALIGN_CENTER_VERTICAL | wxALL, 10);
    topRow->Add(m_statusLabel, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 8);
    topRow->Add(m_clipboardTimerLabel, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 12);
    topRow->AddStretchSpacer();

    // Toolbar action buttons (Undo and Redo are in the Edit menu only)
    m_newEntryBtn = new wxButton(m_headerPanel, ID_NEW_ENTRY, "+ New Account", wxDefaultPosition, wxSize(120, 32));
    m_newEntryBtn->SetBackgroundColour(wxColour(37, 99, 235));
    m_newEntryBtn->SetForegroundColour(*wxWHITE);

    m_newTabBtn = new wxButton(m_headerPanel, ID_NEW_TAB, "+ New Tab", wxDefaultPosition, wxSize(90, 32));
    m_totpHubBtn = new wxButton(m_headerPanel, ID_TOTP_HUB, "Authenticator", wxDefaultPosition, wxSize(105, 32));
    m_sortBtn = new wxButton(m_headerPanel, ID_SORT_MENU, "Sort...", wxDefaultPosition, wxSize(75, 32));
    m_lockBtn = new wxButton(m_headerPanel, ID_LOCK_VAULT, "Lock Vault", wxDefaultPosition, wxSize(90, 32));
    m_settingsBtn = new wxButton(m_headerPanel, ID_SETTINGS, "Settings", wxDefaultPosition, wxSize(85, 32));
    m_pinBtn = new wxButton(m_headerPanel, ID_CHANGE_PIN, "PIN Settings", wxDefaultPosition, wxSize(95, 32));

    topRow->Add(m_newEntryBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);
    topRow->Add(m_newTabBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);
    topRow->Add(m_totpHubBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);
    topRow->Add(m_sortBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);
    topRow->Add(m_lockBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);
    topRow->Add(m_settingsBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);
    topRow->Add(m_pinBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);

    headerSizer->Add(topRow, 0, wxEXPAND);

    // Filter / Search Bar Row with Zoom Controls
    wxBoxSizer* searchRow = new wxBoxSizer(wxHORIZONTAL);
    m_searchPrompt = new wxStaticText(m_headerPanel, wxID_ANY, "Filter Accounts:");
    m_searchPrompt->SetForegroundColour(wxColour(148, 163, 184));

    m_searchCtrl = new wxTextCtrl(m_headerPanel, ID_SEARCH, "", wxDefaultPosition, wxSize(260, 26));
    m_searchCtrl->SetHint("Search accounts, usernames, emails...");

    m_itemCountLabel = new wxStaticText(m_headerPanel, wxID_ANY, "0 accounts listed");
    m_itemCountLabel->SetForegroundColour(wxColour(148, 163, 184));

    searchRow->Add(m_searchPrompt, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 10);
    searchRow->Add(m_searchCtrl, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 12);
    searchRow->Add(m_itemCountLabel, 0, wxALIGN_CENTER_VERTICAL);

    searchRow->AddStretchSpacer();

    // Zoom tool controls
    m_zoomPrompt = new wxStaticText(m_headerPanel, wxID_ANY, "Zoom (Ctrl+Scroll):");
    m_zoomPrompt->SetForegroundColour(wxColour(148, 163, 184));

    m_zoomOutBtn = new wxButton(m_headerPanel, wxID_ANY, " - ", wxDefaultPosition, wxSize(28, 26));
    m_zoomOutBtn->SetToolTip("Zoom Out");
    m_zoomOutBtn->Bind(wxEVT_BUTTON, &MainWindow::OnZoomOut, this);

    m_zoomText = new wxStaticText(m_headerPanel, wxID_ANY, "100%", wxDefaultPosition, wxSize(45, -1), wxALIGN_CENTER_HORIZONTAL);
    m_zoomText->SetForegroundColour(*wxWHITE);
    wxFont zf = m_zoomText->GetFont();
    zf.SetWeight(wxFONTWEIGHT_BOLD);
    m_zoomText->SetFont(zf);

    m_zoomInBtn = new wxButton(m_headerPanel, wxID_ANY, " + ", wxDefaultPosition, wxSize(28, 26));
    m_zoomInBtn->SetToolTip("Zoom In (up to 200%)");
    m_zoomInBtn->Bind(wxEVT_BUTTON, &MainWindow::OnZoomIn, this);

    m_zoomResetBtn = new wxButton(m_headerPanel, wxID_ANY, "Reset", wxDefaultPosition, wxSize(50, 26));
    m_zoomResetBtn->SetToolTip("Reset zoom to 100%");
    m_zoomResetBtn->Bind(wxEVT_BUTTON, &MainWindow::OnZoomReset, this);

    searchRow->Add(m_zoomPrompt, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);
    searchRow->Add(m_zoomOutBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);
    searchRow->Add(m_zoomText, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);
    searchRow->Add(m_zoomInBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);
    searchRow->Add(m_zoomResetBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);

    headerSizer->Add(searchRow, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);
    m_headerPanel->SetSizer(headerSizer);
    mainSizer->Add(m_headerPanel, 0, wxEXPAND);

    // 2. Dynamic Notebook Tabs
    m_notebook = new wxNotebook(this, wxID_ANY);
    m_notebook->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &MainWindow::OnNotebookPageChanged, this);
    mainSizer->Add(m_notebook, 1, wxEXPAND | wxALL, 8);

    SetSizer(mainSizer);
    Center();
}

void MainWindow::ApplyTheme() {
    const auto& th = ThemeManager::Colors;
    SetBackgroundColour(th.windowBg);

    if (m_headerPanel) {
        m_headerPanel->SetBackgroundColour(th.headerBg);
    }
    if (m_appTitle) {
        m_appTitle->SetForegroundColour(th.headerText);
    }
    if (m_statusLabel) {
        m_statusLabel->SetForegroundColour(m_isUnlockedSession ? th.success : th.warning);
    }
    if (m_clipboardTimerLabel) {
        m_clipboardTimerLabel->SetForegroundColour((ThemeManager::CurrentTheme == ThemeMode::HighContrast) ? th.accent : wxColour(147, 197, 253));
    }
    if (m_searchPrompt) {
        m_searchPrompt->SetForegroundColour((ThemeManager::CurrentTheme == ThemeMode::HighContrast) ? th.textSecondary : th.textMuted);
    }
    if (m_itemCountLabel) {
        m_itemCountLabel->SetForegroundColour((ThemeManager::CurrentTheme == ThemeMode::HighContrast) ? th.textSecondary : th.textMuted);
    }
    if (m_zoomPrompt) {
        m_zoomPrompt->SetForegroundColour((ThemeManager::CurrentTheme == ThemeMode::HighContrast) ? th.textSecondary : th.textMuted);
    }
    if (m_zoomText) {
        m_zoomText->SetForegroundColour(th.headerText);
    }

    if (m_newEntryBtn) {
        if (ThemeManager::CurrentTheme == ThemeMode::HighContrast) {
            m_newEntryBtn->SetBackgroundColour(*wxBLACK);
            m_newEntryBtn->SetForegroundColour(*wxWHITE);
        } else {
            m_newEntryBtn->SetBackgroundColour(wxColour(37, 99, 235));
            m_newEntryBtn->SetForegroundColour(*wxWHITE);
        }
    }

    wxButton* topBtns[] = { m_newTabBtn, m_totpHubBtn, m_sortBtn, m_lockBtn, m_settingsBtn, m_pinBtn, m_zoomOutBtn, m_zoomInBtn, m_zoomResetBtn };
    for (wxButton* b : topBtns) {
        if (b) {
            if (ThemeManager::CurrentTheme == ThemeMode::HighContrast) {
                b->SetBackgroundColour(*wxBLACK);
                b->SetForegroundColour(*wxWHITE);
            } else {
                b->SetBackgroundColour(th.buttonBg);
                b->SetForegroundColour(th.buttonText);
            }
        }
    }

    if (m_searchCtrl) {
        if (ThemeManager::CurrentTheme == ThemeMode::Dark) {
            m_searchCtrl->SetBackgroundColour(wxColour(30, 41, 59));
            m_searchCtrl->SetForegroundColour(wxColour(248, 250, 252));
        } else if (ThemeManager::CurrentTheme == ThemeMode::HighContrast) {
            m_searchCtrl->SetBackgroundColour(*wxBLACK);
            m_searchCtrl->SetForegroundColour(*wxWHITE);
        } else {
            m_searchCtrl->SetBackgroundColour(*wxWHITE);
            m_searchCtrl->SetForegroundColour(wxColour(15, 23, 42));
        }
    }

    if (m_notebook) {
        m_notebook->SetBackgroundColour(th.windowBg);
    }

    // Refresh tab pages background
    for (auto& page : m_tabPages) {
        if (page.pagePanel) {
            page.pagePanel->SetBackgroundColour(th.windowBg);
        }
        if (page.scrollWin) {
            page.scrollWin->SetBackgroundColour(th.windowBg);
        }
    }

    ThemeManager::ApplyThemeToWindow(this);

    Refresh();
    Layout();
}

void MainWindow::UpdateEditMenuState() {
    bool canUndo = m_storage.CanUndo();
    bool canRedo = m_storage.CanRedo();

    if (!m_editMenu) return;

    wxMenuItem* undoItem = m_editMenu->FindItem(ID_UNDO);
    if (undoItem) {
        undoItem->Enable(canUndo);
        wxString label = canUndo ? "Undo " + wxString::FromUTF8(m_storage.GetUndoAction().c_str()) + "\tCtrl+Z"
                                 : "Undo\tCtrl+Z";
        undoItem->SetItemLabel(label);
    }

    wxMenuItem* redoItem = m_editMenu->FindItem(ID_REDO);
    if (redoItem) {
        redoItem->Enable(canRedo);
        wxString label = canRedo ? "Redo " + wxString::FromUTF8(m_storage.GetRedoAction().c_str()) + "\tCtrl+Y"
                                 : "Redo\tCtrl+Y";
        redoItem->SetItemLabel(label);
    }
}

void MainWindow::SetupNotebookTabs() {
    int prevSel = m_notebook->GetSelection();
    m_notebook->DeleteAllPages();
    m_tabPages.clear();

    std::vector<std::string> tabList = {
        "All Items",
        "Basic Passwords",
        "Security Questions",
        "Miscellaneous"
    };

    for (const auto& ct : m_storage.GetCustomTabs()) {
        if (std::find(tabList.begin(), tabList.end(), ct) == tabList.end()) {
            tabList.push_back(ct);
        }
    }

    // Update Tabs submenu under View
    if (m_tabsSubMenu) {
        while (m_tabsSubMenu->GetMenuItemCount() > 0) {
            m_tabsSubMenu->Delete(m_tabsSubMenu->FindItemByPosition(0));
        }
        for (size_t i = 0; i < tabList.size(); ++i) {
            int menuId = ID_TAB_BASE + static_cast<int>(i);
            m_tabsSubMenu->Append(menuId, wxString::FromUTF8(tabList[i].c_str()));
            Bind(wxEVT_MENU, &MainWindow::OnSwitchTabFromMenu, this, menuId);
        }
    }

    const auto& th = ThemeManager::Colors;
    for (const auto& tabName : tabList) {
        wxPanel* pagePanel = new wxPanel(m_notebook, wxID_ANY);
        pagePanel->SetBackgroundColour(th.windowBg);
        wxBoxSizer* pageSizer = new wxBoxSizer(wxVERTICAL);

        wxScrolledWindow* scrollWin = new wxScrolledWindow(pagePanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL);
        scrollWin->SetScrollRate(0, 10);
        scrollWin->SetBackgroundColour(th.windowBg);
        wxBoxSizer* cardsSizer = new wxBoxSizer(wxVERTICAL);
        scrollWin->SetSizer(cardsSizer);

        pageSizer->Add(scrollWin, 1, wxEXPAND | wxALL, 6);
        pagePanel->SetSizer(pageSizer);

        m_notebook->AddPage(pagePanel, wxString::FromUTF8(tabName.c_str()));

        TabPageInfo info;
        info.tabName = tabName;
        info.pagePanel = pagePanel;
        info.scrollWin = scrollWin;
        info.cardsSizer = cardsSizer;
        m_tabPages.push_back(info);
    }

    if (!m_tabPages.empty()) {
        int targetSel = (prevSel >= 0 && prevSel < static_cast<int>(m_tabPages.size())) ? prevSel : 0;
        m_notebook->SetSelection(targetSel);
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
    m_itemCountLabel->SetForegroundColour((ThemeManager::CurrentTheme == ThemeMode::HighContrast) ? ThemeManager::Colors.textSecondary : ThemeManager::Colors.textMuted);

    const auto& th = ThemeManager::Colors;

    if (items.empty()) {
        wxPanel* emptyCard = new wxPanel(pageInfo.scrollWin, wxID_ANY);
        emptyCard->SetBackgroundColour(th.cardBg);
        wxBoxSizer* emptySizer = new wxBoxSizer(wxVERTICAL);

        wxStaticText* emptyTxt = new wxStaticText(emptyCard, wxID_ANY,
            "No accounts found in this section.\n\nClick '+ New Account' above to add an entry, or switch tabs.");
        emptyTxt->SetForegroundColour(th.textMuted);
        wxFont ef = emptyTxt->GetFont();
        ef.SetPointSize(11);
        emptyTxt->SetFont(ef);

        emptySizer->Add(emptyTxt, 0, wxALL | wxALIGN_CENTER, 30);
        emptyCard->SetSizer(emptySizer);
        pageInfo.cardsSizer->Add(emptyCard, 0, wxEXPAND | wxALL, 12);
    } else {
        std::vector<std::string> allTabs = m_storage.GetCustomTabs();
        AccountType curTypeHeader = static_cast<AccountType>(-1);
        std::string curTagHeader = "";

        for (int i = 0; i < totalCount; ++i) {
            VaultItem* itm = items[i];

            // When sorted by type, show header titles matching each type (and tab name in All Items)
            if (m_sortDisplayMode == SortDisplayMode::Type) {
                if (itm->type != curTypeHeader) {
                    curTypeHeader = itm->type;
                    curTagHeader = "";

                    wxPanel* headerBanner = new wxPanel(pageInfo.scrollWin, wxID_ANY);
                    headerBanner->SetBackgroundColour(th.sectionHeaderBg);
                    wxBoxSizer* hbSizer = new wxBoxSizer(wxHORIZONTAL);

                    wxString hTitle;
                    if (curTypeHeader == AccountType::Basic) {
                        hTitle = "Type 1: Basic App Passwords (General Purpose)";
                    } else if (curTypeHeader == AccountType::SecurityQuestions) {
                        hTitle = "Type 2: Accounts with Security Questions";
                    } else {
                        hTitle = "Type 3: Miscellaneous & Documents";
                    }

                    wxStaticText* hTxt = new wxStaticText(headerBanner, wxID_ANY, hTitle);
                    hTxt->SetForegroundColour(th.sectionHeaderText);
                    wxFont hf = hTxt->GetFont();
                    hf.SetPointSize(11);
                    hf.SetWeight(wxFONTWEIGHT_BOLD);
                    hTxt->SetFont(hf);

                    hbSizer->Add(hTxt, 0, wxALL, 8);
                    headerBanner->SetSizer(hbSizer);
                    pageInfo.cardsSizer->Add(headerBanner, 0, wxEXPAND | wxTOP | wxBOTTOM, 6);
                }

                if (tab == "All" && itm->tabName != curTagHeader) {
                    curTagHeader = itm->tabName;

                    wxPanel* subBanner = new wxPanel(pageInfo.scrollWin, wxID_ANY);
                    subBanner->SetBackgroundColour(th.cardBg);
                    wxBoxSizer* sbSizer = new wxBoxSizer(wxHORIZONTAL);

                    wxStaticText* sTxt = new wxStaticText(subBanner, wxID_ANY, "  • Section: " + wxString::FromUTF8(curTagHeader.c_str()));
                    sTxt->SetForegroundColour(th.accent);
                    wxFont sf = sTxt->GetFont();
                    sf.SetPointSize(10);
                    sf.SetWeight(wxFONTWEIGHT_BOLD);
                    sTxt->SetFont(sf);

                    sbSizer->Add(sTxt, 0, wxALL, 6);
                    subBanner->SetSizer(sbSizer);
                    pageInfo.cardsSizer->Add(subBanner, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 4);
                }
            } else if (m_sortDisplayMode == SortDisplayMode::Tag && itm->tabName != curTagHeader) {
                curTagHeader = itm->tabName;

                wxPanel* headerBanner = new wxPanel(pageInfo.scrollWin, wxID_ANY);
                headerBanner->SetBackgroundColour(th.sectionHeaderBg);
                wxBoxSizer* hbSizer = new wxBoxSizer(wxHORIZONTAL);

                wxString hTitle = "Section Tag: " + wxString::FromUTF8(curTagHeader.c_str());
                wxStaticText* hTxt = new wxStaticText(headerBanner, wxID_ANY, hTitle);
                hTxt->SetForegroundColour(th.sectionHeaderText);
                wxFont hf = hTxt->GetFont();
                hf.SetPointSize(11);
                hf.SetWeight(wxFONTWEIGHT_BOLD);
                hTxt->SetFont(hf);

                hbSizer->Add(hTxt, 0, wxALL, 8);
                headerBanner->SetSizer(hbSizer);
                pageInfo.cardsSizer->Add(headerBanner, 0, wxEXPAND | wxTOP | wxBOTTOM, 6);
            }

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

void MainWindow::OnZoomIn(wxCommandEvent&) {
    SetZoom(m_zoomLevel + 0.1);
}

void MainWindow::OnZoomOut(wxCommandEvent&) {
    SetZoom(m_zoomLevel - 0.1);
}

void MainWindow::OnZoomReset(wxCommandEvent&) {
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

    PinDialog dlg(this, PinDialogMode::UnlockItem, purposeLabel, m_storage.GetSettings().usePassphraseMode);
    if (dlg.ShowModal() == wxID_OK) {
        std::string enteredPin = dlg.GetEnteredPin();
        if (m_storage.VerifyPin(enteredPin)) {
            m_currentPin = enteredPin;
            m_isUnlockedSession = true;
            const auto& th = ThemeManager::Colors;
            m_statusLabel->SetLabel("Vault: Unlocked");
            m_statusLabel->SetForegroundColour(th.success);
            ResetInactivityTimer();
            return true;
        } else {
            wxMessageBox("Incorrect PIN or Passphrase. Access denied.", "Authentication Failed", wxOK | wxICON_ERROR, this);
            return false;
        }
    }
    return false;
}

void MainWindow::LockSession(bool byInactivity) {
    m_isUnlockedSession = false;
    const auto& th = ThemeManager::Colors;
    m_statusLabel->SetLabel(byInactivity ? "Vault: Locked (Inactivity)" : "Vault: Locked");
    m_statusLabel->SetForegroundColour(th.warning);
    RefreshAllTabs();
}

void MainWindow::CheckPinReminder() {
    if (m_pinReminderShown) return;
    int64_t lastChange = m_storage.GetSettings().lastPinChangeTimestamp;
    int64_t now = static_cast<int64_t>(std::time(nullptr));
    if (lastChange > 0 && (now - lastChange) >= 14 * 86400LL) {
        m_pinReminderShown = true;
        int res = wxMessageBox(
            "Security Reminder:\n\nIt has been more than 2 weeks since your master PIN or Passphrase was last updated.\nConsider updating it periodically for optimal offline protection.\n\nWould you like to change your PIN/passphrase now?",
            "PIN Hygiene Reminder (Every 2 Weeks)",
            wxYES_NO | wxNO_DEFAULT | wxICON_INFORMATION,
            this
        );
        if (res == wxYES) {
            wxCommandEvent dummy;
            OnChangePin(dummy);
        }
    }
}

void MainWindow::OnExit(wxCommandEvent&) {
    Close(true);
}

void MainWindow::OnNewEntry(wxCommandEvent&) {
    std::string curTab = GetCurrentTabName();
    if (curTab == "All Items" || curTab == "Basic Passwords" || curTab == "Security Questions" || curTab == "Miscellaneous") {
        curTab = "General";
    }

    EntryDialog dlg(this, m_storage.GetCustomTabs(), curTab);
    if (dlg.ShowModal() == wxID_OK) {
        VaultItem newItem = dlg.GetVaultItem();
        m_storage.PushUndoState("Add '" + newItem.label + "'");
        m_storage.AddItem(newItem);
        m_storage.Save(m_currentPin);

        if (m_storage.AddTab(newItem.tabName)) {
            SetupNotebookTabs();
        }
        RefreshAllTabs();
        UpdateEditMenuState();
        wxMessageBox("Account '" + wxString::FromUTF8(newItem.label.c_str()) + "' recorded successfully!", "Account Added", wxOK | wxICON_INFORMATION, this);
    }
}

void MainWindow::OnAddNewTab(wxCommandEvent&) {
    wxTextEntryDialog dlg(this, "Enter a new tag name for this tab:", "Create a New tab");
    if (dlg.ShowModal() == wxID_OK) {
        wxString newTab = dlg.GetValue().Trim();
        if (!newTab.empty()) {
            m_storage.PushUndoState("Add Tab '" + newTab.ToStdString() + "'");
            if (m_storage.AddTab(newTab.ToStdString())) {
                m_storage.Save(m_currentPin);
                SetupNotebookTabs();
                for (size_t i = 0; i < m_tabPages.size(); ++i) {
                    if (m_tabPages[i].tabName == newTab.ToStdString()) {
                        m_notebook->SetSelection(static_cast<int>(i));
                        break;
                    }
                }
                RefreshAllTabs();
                UpdateEditMenuState();
            } else {
                wxMessageBox("Tab already exists.", "Notice", wxOK | wxICON_WARNING, this);
            }
        }
    }
}

void MainWindow::OnOpenTotpHub(wxCommandEvent&) {
    if (!PromptAndVerifyPin("2-Factor Authenticator Hub")) return;

    TotpHubDialog dlg(this, m_storage, m_currentPin);
    dlg.ShowModal();
    RefreshAllTabs();
}

void MainWindow::OnLockVault(wxCommandEvent&) {
    LockSession(false);
    wxMessageBox("Vault locked. PIN or Passphrase will be required to view account details.", "Vault Locked", wxOK | wxICON_INFORMATION, this);
}

void MainWindow::OnChangePin(wxCommandEvent&) {
    PinDialog dlg(this, PinDialogMode::ChangePin, "", m_storage.GetSettings().usePassphraseMode);
    if (dlg.ShowModal() == wxID_OK) {
        std::string cur = dlg.GetEnteredPin();
        std::string nw = dlg.GetNewPin();
        if (m_storage.ChangePin(cur, nw)) {
            m_currentPin = nw;
            VaultSettings s = m_storage.GetSettings();
            s.usePassphraseMode = dlg.IsPassphraseMode();
            m_storage.UpdateSettings(s);
            m_storage.Save(m_currentPin);

            // Auto-lock vault immediately after PIN/passphrase change!
            LockSession(false);
            m_statusLabel->SetLabel("Vault: Locked (Key Changed)");
            RefreshAllTabs();

            wxMessageBox(
                "Master Key changed successfully!\n\nThe vault has auto-locked for security. Please enter your new PIN or Passphrase to unlock.",
                "Key Changed - Re-Authentication Required",
                wxOK | wxICON_INFORMATION,
                this
            );
        } else {
            wxMessageBox("Current PIN or Passphrase was incorrect. Master Key was not updated.", "Authentication Error", wxOK | wxICON_ERROR, this);
        }
    }
}

void MainWindow::OnSearchUpdated(wxCommandEvent&) {
    RefreshCurrentTabList();
}

void MainWindow::OnNotebookPageChanged(wxBookCtrlEvent& evt) {
    RefreshCurrentTabList();
    evt.Skip();
}

// Edit Undo / Redo
void MainWindow::OnUndo(wxCommandEvent&) {
    if (m_storage.Undo()) {
        m_storage.Save(m_currentPin);
        SetupNotebookTabs();
        RefreshAllTabs();
        UpdateEditMenuState();
    }
}

void MainWindow::OnRedo(wxCommandEvent&) {
    if (m_storage.Redo()) {
        m_storage.Save(m_currentPin);
        SetupNotebookTabs();
        RefreshAllTabs();
        UpdateEditMenuState();
    }
}

// Sorting
void MainWindow::OnSortByTimeNewest(wxCommandEvent&) {
    m_sortDisplayMode = SortDisplayMode::None;
    m_storage.PushUndoState("Sort by Time (Newest)");
    m_storage.SortByTimestamp(true);
    m_storage.Save(m_currentPin);
    RefreshAllTabs();
    UpdateEditMenuState();
}

void MainWindow::OnSortByTimeOldest(wxCommandEvent&) {
    m_sortDisplayMode = SortDisplayMode::None;
    m_storage.PushUndoState("Sort by Time (Oldest)");
    m_storage.SortByTimestamp(false);
    m_storage.Save(m_currentPin);
    RefreshAllTabs();
    UpdateEditMenuState();
}

void MainWindow::OnSortAlphabeticalAsc(wxCommandEvent&) {
    m_sortDisplayMode = SortDisplayMode::None;
    m_storage.PushUndoState("Sort A-Z");
    m_storage.SortAlphabetical(true);
    m_storage.Save(m_currentPin);
    RefreshAllTabs();
    UpdateEditMenuState();
}

void MainWindow::OnSortAlphabeticalDesc(wxCommandEvent&) {
    m_sortDisplayMode = SortDisplayMode::None;
    m_storage.PushUndoState("Sort Z-A");
    m_storage.SortAlphabetical(false);
    m_storage.Save(m_currentPin);
    RefreshAllTabs();
    UpdateEditMenuState();
}

void MainWindow::OnSortByType(wxCommandEvent&) {
    m_sortDisplayMode = SortDisplayMode::Type;
    m_storage.PushUndoState("Sort by Type");
    m_storage.SortByType();
    m_storage.Save(m_currentPin);
    RefreshAllTabs();
    UpdateEditMenuState();
}

void MainWindow::OnSortByTagsDefault(wxCommandEvent&) {
    m_sortDisplayMode = SortDisplayMode::Tag;
    m_storage.PushUndoState("Sort by Tags (Default)");
    m_storage.SortByTags(false);
    m_storage.Save(m_currentPin);
    RefreshAllTabs();
    UpdateEditMenuState();
}

void MainWindow::OnSortByTagsAndItems(wxCommandEvent&) {
    m_sortDisplayMode = SortDisplayMode::Tag;
    m_storage.PushUndoState("Sort by Tags & Items");
    m_storage.SortByTags(true);
    m_storage.Save(m_currentPin);
    RefreshAllTabs();
    UpdateEditMenuState();
}

void MainWindow::OnSortBtnClicked(wxCommandEvent&) {
    wxMenu menu;
    menu.Append(ID_SORT_TIME_NEWEST, "Sort by Time Created (Newest First)");
    menu.Append(ID_SORT_TIME_OLDEST, "Sort by Time Created (Oldest First)");
    menu.Append(ID_SORT_ALPHA_ASC, "Sort Alphabetical (A - Z)");
    menu.Append(ID_SORT_ALPHA_DESC, "Sort Alphabetical (Z - A)");
    menu.Append(ID_SORT_TYPE, "Sort by Type");
    menu.Append(ID_SORT_TAGS_DEFAULT, "Sort Tags Alphabetical (Default: Types A-Z, Items by Date)");
    menu.Append(ID_SORT_TAGS_ITEMS, "Sort Tags & Items Alphabetical (Both A-Z)");
    PopupMenu(&menu);
}

void MainWindow::OnSwitchTabFromMenu(wxCommandEvent& evt) {
    int idx = evt.GetId() - ID_TAB_BASE;
    if (idx >= 0 && idx < static_cast<int>(m_tabPages.size())) {
        m_notebook->SetSelection(idx);
    }
}

// Themes
void MainWindow::OnSetThemeLight(wxCommandEvent&) {
    ThemeManager::SetTheme(ThemeMode::Minimalist);
    VaultSettings s = m_storage.GetSettings();
    s.theme = "minimalist";
    m_storage.UpdateSettings(s);
    m_storage.Save(m_currentPin);
    ApplyTheme();
    RefreshAllTabs();
}

void MainWindow::OnSetThemeDark(wxCommandEvent&) {
    ThemeManager::SetTheme(ThemeMode::Dark);
    VaultSettings s = m_storage.GetSettings();
    s.theme = "dark";
    m_storage.UpdateSettings(s);
    m_storage.Save(m_currentPin);
    ApplyTheme();
    RefreshAllTabs();
}

void MainWindow::OnSetThemeHighContrast(wxCommandEvent&) {
    ThemeManager::SetTheme(ThemeMode::HighContrast);
    VaultSettings s = m_storage.GetSettings();
    s.theme = "high_contrast";
    m_storage.UpdateSettings(s);
    m_storage.Save(m_currentPin);
    ApplyTheme();
    RefreshAllTabs();
}

// Tools
void MainWindow::OnBackupVault(wxCommandEvent&) {
    BackupRestoreDialog dlg(this, m_storage, BackupDialogMode::Create);
    dlg.ShowModal();
}

void MainWindow::OnRestoreVault(wxCommandEvent&) {
    BackupRestoreDialog dlg(this, m_storage, BackupDialogMode::Restore);
    if (dlg.ShowModal() == wxID_OK && dlg.WasRestored()) {
        m_storage.Save(m_currentPin);
        SetupNotebookTabs();
        RefreshAllTabs();
        UpdateEditMenuState();
    }
}

void MainWindow::OnPasswordAnalyzer(wxCommandEvent&) {
    PasswordAnalyzerDialog dlg(this, m_storage);
    dlg.ShowModal();
}

// Configure
void MainWindow::OnOpenSettings(wxCommandEvent&) {
    SettingsDialog dlg(this, m_storage.GetSettings());
    if (dlg.ShowModal() == wxID_OK) {
        m_storage.UpdateSettings(dlg.GetSettings());
        m_storage.Save(m_currentPin);
        ApplyTheme();
        RefreshAllTabs();
    }
}

// Window
void MainWindow::OnWindowHome(wxCommandEvent&) {
    m_notebook->SetSelection(0); // All Items
}

// Help
void MainWindow::OnOpenHelp(wxCommandEvent&) {
    HelpDialog dlg(this);
    dlg.ShowModal();
}

void MainWindow::OnAbout(wxCommandEvent&) {
    wxMessageBox(
        "Password Locker - Offline Password Vault & 2-Factor Authenticator\n\n"
        "• 100% Offline with zero internet dependency.\n"
        "• AES-256-CBC with PBKDF2-SHA256 Encryption.\n"
        "• Inactivity Auto-Lock & Automatic 2-Minute Clipboard Sanitization.\n"
        "• Drag-and-Drop Organization & Custom Tabs.\n"
        "• Fullscreen Document Viewer & Password Health Audit.\n"
        "• High-Contrast & Dark Mode support.\n\n"
        "EC_Lab Vault v2.0 - Offline Security Suite",
        "About Password Locker",
        wxOK | wxICON_INFORMATION,
        this
    );
}

// Ticker Timer (ticks every 1s)
void MainWindow::OnTickerTimer(wxTimerEvent&) {
    // 1. Sanitizer check: auto-clear clipboard after 2 minutes
    ClipboardSanitizer::CheckAndClearExpired();
    if (m_clipboardTimerLabel) {
        if (ClipboardSanitizer::HasActiveSanitizer()) {
            int rem = ClipboardSanitizer::GetRemainingSeconds();
            m_clipboardTimerLabel->SetLabel(wxString::Format("Clipboard Wipe: %ds", rem));
            m_clipboardTimerLabel->Show(true);
        } else {
            m_clipboardTimerLabel->Show(false);
        }
    }

    // 2. Inactivity check
    if (m_isUnlockedSession) {
        int lockMins = m_storage.GetSettings().autoLockMinutes;
        if (lockMins > 0) {
            std::time_t elapsed = std::time(nullptr) - m_lastActivityTime;
            if (elapsed >= static_cast<std::time_t>(lockMins * 60)) {
                LockSession(true);
            }
        }
    }
}

void MainWindow::OnUnlockItem(const std::string& itemId) {
    VaultItem* item = m_storage.GetItemById(itemId);
    if (!item) return;

    if (!PromptAndVerifyPin(item->label)) return;

    DetailDialog dlg(this, *item);
    dlg.ShowModal();

    if (dlg.WasItemModified()) {
        m_storage.PushUndoState("Update '" + item->label + "'");
        m_storage.UpdateItem(dlg.GetItem());
        m_storage.Save(m_currentPin);
        RefreshAllTabs();
        UpdateEditMenuState();
    }

    if (dlg.GetSelectedAction() == DetailAction_Edit) {
        EntryDialog editDlg(this, m_storage.GetCustomTabs(), item->tabName, item);
        if (editDlg.ShowModal() == wxID_OK) {
            VaultItem updated = editDlg.GetVaultItem();
            m_storage.PushUndoState("Edit '" + updated.label + "'");
            m_storage.UpdateItem(updated);
            m_storage.Save(m_currentPin);
            RefreshAllTabs();
            UpdateEditMenuState();
        }
    } else if (dlg.GetSelectedAction() == DetailAction_Delete) {
        m_storage.PushUndoState("Delete '" + item->label + "'");
        m_storage.DeleteItem(itemId);
        m_storage.Save(m_currentPin);
        RefreshAllTabs();
        UpdateEditMenuState();
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
        m_storage.PushUndoState("Delete '" + item->label + "'");
        m_storage.DeleteItem(itemId);
        m_storage.Save(m_currentPin);
        RefreshAllTabs();
        UpdateEditMenuState();
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
        m_storage.PushUndoState("Move Account");
        m_storage.MoveItem(curIdx, targetIdx);
        m_storage.Save(m_currentPin);
        RefreshAllTabs();
        UpdateEditMenuState();
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
        m_storage.PushUndoState("Reorder Accounts");
        m_storage.MoveItem(fromIdx, toIdx);
        m_storage.Save(m_currentPin);
        RefreshAllTabs();
        UpdateEditMenuState();
    }
}

void MainWindow::OnMoveItemToTab(const std::string& itemId, const std::string& targetTab) {
    m_storage.PushUndoState("Move to Section '" + targetTab + "'");
    m_storage.MoveItemToTab(itemId, targetTab);
    m_storage.Save(m_currentPin);
    RefreshAllTabs();
    UpdateEditMenuState();
}
