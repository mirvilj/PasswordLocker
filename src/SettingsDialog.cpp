#include "SettingsDialog.h"
#include "ThemeManager.h"

wxBEGIN_EVENT_TABLE(SettingsDialog, wxDialog)
    EVT_BUTTON(wxID_OK, SettingsDialog::OnSave)
    EVT_BUTTON(wxID_CANCEL, SettingsDialog::OnCancel)
wxEND_EVENT_TABLE()

SettingsDialog::SettingsDialog(wxWindow* parent, const VaultSettings& currentSettings)
    : wxDialog(parent, wxID_ANY, "Password Locker - Settings & Preferences", wxDefaultPosition, wxSize(520, 560), wxDEFAULT_DIALOG_STYLE),
      m_settings(currentSettings),
      m_originalTheme(currentSettings.theme),
      m_headerPanel(nullptr),
      m_title(nullptr),
      m_sub(nullptr),
      m_body(nullptr),
      m_lockBox(nullptr),
      m_lockHelp(nullptr),
      m_autoLockChoice(nullptr),
      m_backupBox(nullptr),
      m_backupHelp(nullptr),
      m_autoBackupChoice(nullptr),
      m_clipBox(nullptr),
      m_clipboardClearCheck(nullptr),
      m_keyModeBox(nullptr),
      m_keyPinRadio(nullptr),
      m_keyPassRadio(nullptr),
      m_themeBox(nullptr),
      m_themeMinimalistRadio(nullptr),
      m_themeDarkRadio(nullptr),
      m_themeHighContrastRadio(nullptr),
      m_botPanel(nullptr),
      m_cancelBtn(nullptr),
      m_saveBtn(nullptr) {

    SetupUI();
    ApplyTheme();
}

void SettingsDialog::SetupUI() {
    wxBoxSizer* rootSizer = new wxBoxSizer(wxVERTICAL);

    // Header Panel
    m_headerPanel = new wxPanel(this, wxID_ANY);
    wxBoxSizer* headerSizer = new wxBoxSizer(wxVERTICAL);

    m_title = new wxStaticText(m_headerPanel, wxID_ANY, "Application & Security Settings");
    wxFont tf = m_title->GetFont();
    tf.SetPointSize(13);
    tf.SetWeight(wxFONTWEIGHT_BOLD);
    m_title->SetFont(tf);

    m_sub = new wxStaticText(m_headerPanel, wxID_ANY, "Configure security timeouts, auto-backups, authentication mode, and themes.");

    headerSizer->Add(m_title, 0, wxALL, 12);
    headerSizer->Add(m_sub, 0, wxLEFT | wxRIGHT | wxBOTTOM, 12);
    m_headerPanel->SetSizer(headerSizer);
    rootSizer->Add(m_headerPanel, 0, wxEXPAND);

    // Body
    m_body = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL);
    m_body->SetScrollRate(0, 10);
    wxBoxSizer* bodySizer = new wxBoxSizer(wxVERTICAL);

    // Section 1: Inactivity Auto-Lock
    m_lockBox = new wxStaticBoxSizer(wxVERTICAL, m_body, "Inactivity Auto-Lock Security");
    m_lockHelp = new wxStaticText(m_lockBox->GetStaticBox(), wxID_ANY,
        "Automatically lock the vault and hide stored credentials after period of inactivity:");

    wxArrayString lockOptions;
    lockOptions.Add("1 Minute");
    lockOptions.Add("2 Minutes");
    lockOptions.Add("5 Minutes (Recommended)");
    lockOptions.Add("10 Minutes");
    lockOptions.Add("15 Minutes");
    lockOptions.Add("30 Minutes");
    lockOptions.Add("Disabled (Never Lock)");

    m_autoLockChoice = new wxChoice(m_lockBox->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxSize(240, -1), lockOptions);

    int selLock = 2; // Default 5 min
    switch (m_settings.autoLockMinutes) {
        case 1: selLock = 0; break;
        case 2: selLock = 1; break;
        case 5: selLock = 2; break;
        case 10: selLock = 3; break;
        case 15: selLock = 4; break;
        case 30: selLock = 5; break;
        case 0: selLock = 6; break;
        default: selLock = 2; break;
    }
    m_autoLockChoice->SetSelection(selLock);

    m_lockBox->Add(m_lockHelp, 0, wxALL, 6);
    m_lockBox->Add(m_autoLockChoice, 0, wxLEFT | wxRIGHT | wxBOTTOM, 8);
    bodySizer->Add(m_lockBox, 0, wxEXPAND | wxALL, 10);

    // Section 2: Automatic Backup Schedule
    m_backupBox = new wxStaticBoxSizer(wxVERTICAL, m_body, "Automated Encrypted Backups");
    m_backupHelp = new wxStaticText(m_backupBox->GetStaticBox(), wxID_ANY,
        "Automatic backup frequency to create secure crash-recovery snapshots:");

    wxArrayString backupOptions;
    backupOptions.Add("Every Day");
    backupOptions.Add("Every 3 Days");
    backupOptions.Add("Every Week (Recommended)");
    backupOptions.Add("Every Month");
    backupOptions.Add("Disabled (Manual Only)");

    m_autoBackupChoice = new wxChoice(m_backupBox->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxSize(240, -1), backupOptions);

    int selBackup = 2; // Default weekly
    switch (m_settings.autoBackupDays) {
        case 1: selBackup = 0; break;
        case 3: selBackup = 1; break;
        case 7: selBackup = 2; break;
        case 30: selBackup = 3; break;
        case 0: selBackup = 4; break;
        default: selBackup = 2; break;
    }
    m_autoBackupChoice->SetSelection(selBackup);

    m_backupBox->Add(m_backupHelp, 0, wxALL, 6);
    m_backupBox->Add(m_autoBackupChoice, 0, wxLEFT | wxRIGHT | wxBOTTOM, 8);
    bodySizer->Add(m_backupBox, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // Section 3: Clipboard Auto-Clear
    m_clipBox = new wxStaticBoxSizer(wxVERTICAL, m_body, "Clipboard");
    m_clipboardClearCheck = new wxCheckBox(m_clipBox->GetStaticBox(), wxID_ANY,
        "Automatically clear copied passwords and 2FA tokens after 2 minutes");
    m_clipboardClearCheck->SetValue(m_settings.clipboardClearSeconds > 0);
    m_clipBox->Add(m_clipboardClearCheck, 0, wxALL, 8);
    bodySizer->Add(m_clipBox, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // Section 4: Master Key Format (PIN vs Passphrase)
    m_keyModeBox = new wxStaticBoxSizer(wxVERTICAL, m_body, "Authentication Security Mode");
    m_keyPinRadio = new wxRadioButton(m_keyModeBox->GetStaticBox(), wxID_ANY,
        "4-Digit PIN (4-8 numeric digits)", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    m_keyPassRadio = new wxRadioButton(m_keyModeBox->GetStaticBox(), wxID_ANY,
        "Master Passphrase / Password (6 to 63 characters)");

    if (m_settings.usePassphraseMode) {
        m_keyPassRadio->SetValue(true);
    } else {
        m_keyPinRadio->SetValue(true);
    }

    m_keyModeBox->Add(m_keyPinRadio, 0, wxALL, 6);
    m_keyModeBox->Add(m_keyPassRadio, 0, wxLEFT | wxRIGHT | wxBOTTOM, 6);
    bodySizer->Add(m_keyModeBox, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // Section 5: Visual Theme & Accessibility
    m_themeBox = new wxStaticBoxSizer(wxVERTICAL, m_body, "Appearance");
    m_themeMinimalistRadio = new wxRadioButton(m_themeBox->GetStaticBox(), wxID_ANY,
        "Minimalist", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    m_themeDarkRadio = new wxRadioButton(m_themeBox->GetStaticBox(), wxID_ANY,
        "Dark Mode");
    m_themeHighContrastRadio = new wxRadioButton(m_themeBox->GetStaticBox(), wxID_ANY,
        "High-Contrast Mode (Accessibility)");

    if (m_settings.theme == "dark") {
        m_themeDarkRadio->SetValue(true);
    } else if (m_settings.theme == "high_contrast") {
        m_themeHighContrastRadio->SetValue(true);
    } else {
        m_themeMinimalistRadio->SetValue(true);
    }

    m_themeMinimalistRadio->Bind(wxEVT_RADIOBUTTON, &SettingsDialog::OnThemeChanged, this);
    m_themeDarkRadio->Bind(wxEVT_RADIOBUTTON, &SettingsDialog::OnThemeChanged, this);
    m_themeHighContrastRadio->Bind(wxEVT_RADIOBUTTON, &SettingsDialog::OnThemeChanged, this);

    m_themeBox->Add(m_themeMinimalistRadio, 0, wxALL, 6);
    m_themeBox->Add(m_themeDarkRadio, 0, wxLEFT | wxRIGHT | wxBOTTOM, 6);
    m_themeBox->Add(m_themeHighContrastRadio, 0, wxLEFT | wxRIGHT | wxBOTTOM, 6);
    bodySizer->Add(m_themeBox, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    m_body->SetSizer(bodySizer);
    rootSizer->Add(m_body, 1, wxEXPAND);

    // Bottom Action Buttons
    m_botPanel = new wxPanel(this, wxID_ANY);
    wxBoxSizer* botSizer = new wxBoxSizer(wxHORIZONTAL);

    m_cancelBtn = new wxButton(m_botPanel, wxID_CANCEL, "Cancel", wxDefaultPosition, wxSize(90, 32));
    m_saveBtn = new wxButton(m_botPanel, wxID_OK, "Save Settings", wxDefaultPosition, wxSize(120, 32));
    m_saveBtn->SetDefault();

    botSizer->AddStretchSpacer();
    botSizer->Add(m_cancelBtn, 0, wxALL, 8);
    botSizer->Add(m_saveBtn, 0, wxALL, 8);
    m_botPanel->SetSizer(botSizer);
    rootSizer->Add(m_botPanel, 0, wxEXPAND);

    SetSizer(rootSizer);
    CenterOnParent();
}

void SettingsDialog::ApplyTheme() {
    const auto& th = ThemeManager::Colors;
    SetBackgroundColour(th.windowBg);

    if (m_headerPanel) m_headerPanel->SetBackgroundColour(th.headerBg);
    if (m_title) m_title->SetForegroundColour(th.headerText);
    if (m_sub) m_sub->SetForegroundColour((ThemeManager::CurrentTheme == ThemeMode::HighContrast) ? th.textSecondary : wxColour(203, 213, 225));
    if (m_body) m_body->SetBackgroundColour(th.windowBg);

    if (m_lockBox && m_lockBox->GetStaticBox()) {
        m_lockBox->GetStaticBox()->SetForegroundColour(th.textPrimary);
        m_lockBox->GetStaticBox()->SetBackgroundColour(th.windowBg);
    }
    if (m_lockHelp) m_lockHelp->SetForegroundColour(th.textSecondary);
    if (m_autoLockChoice) {
        if (ThemeManager::CurrentTheme == ThemeMode::Dark) {
            m_autoLockChoice->SetBackgroundColour(wxColour(30, 41, 59));
            m_autoLockChoice->SetForegroundColour(wxColour(248, 250, 252));
        } else if (ThemeManager::CurrentTheme == ThemeMode::HighContrast) {
            m_autoLockChoice->SetBackgroundColour(*wxBLACK);
            m_autoLockChoice->SetForegroundColour(*wxWHITE);
        } else {
            m_autoLockChoice->SetBackgroundColour(*wxWHITE);
            m_autoLockChoice->SetForegroundColour(wxColour(15, 23, 42));
        }
    }

    if (m_backupBox && m_backupBox->GetStaticBox()) {
        m_backupBox->GetStaticBox()->SetForegroundColour(th.textPrimary);
        m_backupBox->GetStaticBox()->SetBackgroundColour(th.windowBg);
    }
    if (m_backupHelp) m_backupHelp->SetForegroundColour(th.textSecondary);
    if (m_autoBackupChoice) {
        if (ThemeManager::CurrentTheme == ThemeMode::Dark) {
            m_autoBackupChoice->SetBackgroundColour(wxColour(30, 41, 59));
            m_autoBackupChoice->SetForegroundColour(wxColour(248, 250, 252));
        } else if (ThemeManager::CurrentTheme == ThemeMode::HighContrast) {
            m_autoBackupChoice->SetBackgroundColour(*wxBLACK);
            m_autoBackupChoice->SetForegroundColour(*wxWHITE);
        } else {
            m_autoBackupChoice->SetBackgroundColour(*wxWHITE);
            m_autoBackupChoice->SetForegroundColour(wxColour(15, 23, 42));
        }
    }

    if (m_clipBox && m_clipBox->GetStaticBox()) {
        m_clipBox->GetStaticBox()->SetForegroundColour(th.textPrimary);
        m_clipBox->GetStaticBox()->SetBackgroundColour(th.windowBg);
    }
    if (m_clipboardClearCheck) {
        m_clipboardClearCheck->SetForegroundColour(th.textPrimary);
        m_clipboardClearCheck->SetBackgroundColour(th.windowBg);
    }

    if (m_keyModeBox && m_keyModeBox->GetStaticBox()) {
        m_keyModeBox->GetStaticBox()->SetForegroundColour((ThemeManager::CurrentTheme == ThemeMode::HighContrast) ? th.textSecondary : th.textPrimary);
        m_keyModeBox->GetStaticBox()->SetBackgroundColour(th.windowBg);
    }
    if (m_keyPinRadio) {
        m_keyPinRadio->SetForegroundColour(th.textPrimary);
        m_keyPinRadio->SetBackgroundColour(th.windowBg);
    }
    if (m_keyPassRadio) {
        m_keyPassRadio->SetForegroundColour(th.textPrimary);
        m_keyPassRadio->SetBackgroundColour(th.windowBg);
    }

    if (m_themeBox && m_themeBox->GetStaticBox()) {
        m_themeBox->GetStaticBox()->SetForegroundColour((ThemeManager::CurrentTheme == ThemeMode::HighContrast) ? th.textSecondary : th.textPrimary);
        m_themeBox->GetStaticBox()->SetBackgroundColour(th.windowBg);
    }
    if (m_themeMinimalistRadio) {
        m_themeMinimalistRadio->SetForegroundColour(th.textPrimary);
        m_themeMinimalistRadio->SetBackgroundColour(th.windowBg);
    }
    if (m_themeDarkRadio) {
        m_themeDarkRadio->SetForegroundColour(th.textPrimary);
        m_themeDarkRadio->SetBackgroundColour(th.windowBg);
    }
    if (m_themeHighContrastRadio) {
        m_themeHighContrastRadio->SetForegroundColour(th.textPrimary);
        m_themeHighContrastRadio->SetBackgroundColour(th.windowBg);
    }

    if (m_botPanel) m_botPanel->SetBackgroundColour(th.headerBg);
    if (m_cancelBtn) {
        m_cancelBtn->SetBackgroundColour(th.buttonBg);
        m_cancelBtn->SetForegroundColour(th.buttonText);
    }
    if (m_saveBtn) {
        if (ThemeManager::CurrentTheme == ThemeMode::HighContrast) {
            m_saveBtn->SetBackgroundColour(*wxBLACK);
            m_saveBtn->SetForegroundColour(*wxWHITE);
        } else {
            m_saveBtn->SetBackgroundColour(wxColour(37, 99, 235));
            m_saveBtn->SetForegroundColour(*wxWHITE);
        }
    }

    ThemeManager::ApplyThemeToWindow(this);
    Refresh();
    Layout();
}

void SettingsDialog::OnThemeChanged(wxCommandEvent&) {
    if (m_themeDarkRadio && m_themeDarkRadio->GetValue()) {
        ThemeManager::SetTheme(ThemeMode::Dark);
    } else if (m_themeHighContrastRadio && m_themeHighContrastRadio->GetValue()) {
        ThemeManager::SetTheme(ThemeMode::HighContrast);
    } else {
        ThemeManager::SetTheme(ThemeMode::Minimalist);
    }
    ApplyTheme();
}

void SettingsDialog::OnSave(wxCommandEvent&) {
    // Inactivity lock
    int lockSel = m_autoLockChoice->GetSelection();
    switch (lockSel) {
        case 0: m_settings.autoLockMinutes = 1; break;
        case 1: m_settings.autoLockMinutes = 2; break;
        case 2: m_settings.autoLockMinutes = 5; break;
        case 3: m_settings.autoLockMinutes = 10; break;
        case 4: m_settings.autoLockMinutes = 15; break;
        case 5: m_settings.autoLockMinutes = 30; break;
        case 6: m_settings.autoLockMinutes = 0; break; // Disabled
        default: m_settings.autoLockMinutes = 5; break;
    }

    // Auto-backup frequency
    int backupSel = m_autoBackupChoice->GetSelection();
    switch (backupSel) {
        case 0: m_settings.autoBackupDays = 1; break;
        case 1: m_settings.autoBackupDays = 3; break;
        case 2: m_settings.autoBackupDays = 7; break;
        case 3: m_settings.autoBackupDays = 30; break;
        case 4: m_settings.autoBackupDays = 0; break; // Disabled
        default: m_settings.autoBackupDays = 7; break;
    }

    // Clipboard clear
    m_settings.clipboardClearSeconds = m_clipboardClearCheck->IsChecked() ? 120 : 0;

    // Master Key Mode
    m_settings.usePassphraseMode = (m_keyPassRadio && m_keyPassRadio->GetValue());

    // Theme
    if (m_themeDarkRadio && m_themeDarkRadio->GetValue()) {
        m_settings.theme = "dark";
        ThemeManager::SetTheme(ThemeMode::Dark);
    } else if (m_themeHighContrastRadio && m_themeHighContrastRadio->GetValue()) {
        m_settings.theme = "high_contrast";
        ThemeManager::SetTheme(ThemeMode::HighContrast);
    } else {
        m_settings.theme = "minimalist";
        ThemeManager::SetTheme(ThemeMode::Minimalist);
    }

    EndModal(wxID_OK);
}

void SettingsDialog::OnCancel(wxCommandEvent&) {
    ThemeManager::SetThemeByName(m_originalTheme);
    EndModal(wxID_CANCEL);
}
