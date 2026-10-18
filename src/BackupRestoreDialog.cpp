#include "BackupRestoreDialog.h"
#include "ThemeManager.h"
#include <ctime>

wxBEGIN_EVENT_TABLE(BackupRestoreDialog, wxDialog)
    EVT_BUTTON(wxID_OK, BackupRestoreDialog::OnExecute)
    EVT_BUTTON(wxID_CANCEL, BackupRestoreDialog::OnCancel)
wxEND_EVENT_TABLE()

BackupRestoreDialog::BackupRestoreDialog(wxWindow* parent, VaultStorage& storage, BackupDialogMode mode)
    : wxDialog(parent, wxID_ANY, "", wxDefaultPosition, wxSize(520, 380), wxDEFAULT_DIALOG_STYLE),
      m_storage(storage),
      m_mode(mode),
      m_wasRestored(false),
      m_filePicker(nullptr),
      m_keyCtrl(nullptr),
      m_confirmKeyCtrl(nullptr),
      m_errorText(nullptr) {

    SetBackgroundColour(ThemeManager::Colors.cardBg);
    SetupUI();
}

void BackupRestoreDialog::SetupUI() {
    const auto& th = ThemeManager::Colors;
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Header Panel
    wxPanel* headerPanel = new wxPanel(this, wxID_ANY);
    headerPanel->SetBackgroundColour(th.headerBg);
    wxBoxSizer* headerSizer = new wxBoxSizer(wxVERTICAL);

    wxString titleText = (m_mode == BackupDialogMode::Create) ? "Create Encrypted Vault Backup" : "Restore Vault from Backup";
    wxString subText = (m_mode == BackupDialogMode::Create)
        ? "Compile accounts, tabs, attachments, and settings into an encrypted, unreadable container (.ecvault)."
        : "Restore all accounts, tabs, and preferences from an encrypted .ecvault backup file.";

    SetTitle("Password Locker - " + titleText);

    wxStaticText* title = new wxStaticText(headerPanel, wxID_ANY, titleText);
    title->SetForegroundColour(th.headerText);
    wxFont tf = title->GetFont();
    tf.SetPointSize(13);
    tf.SetWeight(wxFONTWEIGHT_BOLD);
    title->SetFont(tf);

    wxStaticText* sub = new wxStaticText(headerPanel, wxID_ANY, subText);
    sub->SetForegroundColour((ThemeManager::CurrentTheme == ThemeMode::HighContrast) ? th.textSecondary : wxColour(203, 213, 225));

    headerSizer->Add(title, 0, wxALL, 12);
    headerSizer->Add(sub, 0, wxLEFT | wxRIGHT | wxBOTTOM, 12);
    headerPanel->SetSizer(headerSizer);
    mainSizer->Add(headerPanel, 0, wxEXPAND);

    // Form fields
    wxBoxSizer* formSizer = new wxBoxSizer(wxVERTICAL);

    wxStaticText* fileLbl = new wxStaticText(this, wxID_ANY, (m_mode == BackupDialogMode::Create) ? "Backup File Location:" : "Select .ecvault Backup File:");
    fileLbl->SetForegroundColour(th.textPrimary);
    formSizer->Add(fileLbl, 0, wxBOTTOM, 4);

    wxString defaultFile;
    if (m_mode == BackupDialogMode::Create) {
        char timeBuf[64];
        std::time_t tNow = std::time(nullptr);
        std::tm* tmInfo = std::localtime(&tNow);
        std::strftime(timeBuf, sizeof(timeBuf), "%Y%m%d_%H%M%S", tmInfo);
        defaultFile = "data/backup_" + wxString(timeBuf) + ".ecvault";
    }

    long pickerStyle = (m_mode == BackupDialogMode::Create) ? (wxFLP_SAVE | wxFLP_OVERWRITE_PROMPT | wxFLP_USE_TEXTCTRL)
                                                           : (wxFLP_OPEN | wxFLP_FILE_MUST_EXIST | wxFLP_USE_TEXTCTRL);

    m_filePicker = new wxFilePickerCtrl(this, wxID_ANY, defaultFile, "Choose Backup File", "Encrypted Vault (*.ecvault)|*.ecvault|All Files (*.*)|*.*", wxDefaultPosition, wxDefaultSize, pickerStyle);
    formSizer->Add(m_filePicker, 0, wxEXPAND | wxBOTTOM, 12);

    wxStaticText* keyLbl = new wxStaticText(this, wxID_ANY, (m_mode == BackupDialogMode::Create) ? "Encryption Passphrase (Do not share with anyone):" : "Backup Decryption Passphrase:");
    keyLbl->SetForegroundColour(th.textPrimary);
    m_keyCtrl = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(-1, 30), wxTE_PASSWORD);
    formSizer->Add(keyLbl, 0, wxBOTTOM, 4);
    formSizer->Add(m_keyCtrl, 0, wxEXPAND | wxBOTTOM, 10);

    if (m_mode == BackupDialogMode::Create) {
        wxStaticText* confKeyLbl = new wxStaticText(this, wxID_ANY, "Confirm Encryption Passphrase:");
        confKeyLbl->SetForegroundColour(th.textPrimary);
        m_confirmKeyCtrl = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(-1, 30), wxTE_PASSWORD);
        formSizer->Add(confKeyLbl, 0, wxBOTTOM, 4);
        formSizer->Add(m_confirmKeyCtrl, 0, wxEXPAND | wxBOTTOM, 10);
    }

    m_errorText = new wxStaticText(this, wxID_ANY, "");
    m_errorText->SetForegroundColour(th.danger);
    formSizer->Add(m_errorText, 0, wxBOTTOM, 8);

    // Buttons
    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* cancelBtn = new wxButton(this, wxID_CANCEL, "Cancel", wxDefaultPosition, wxSize(90, 32));
    wxButton* execBtn = new wxButton(this, wxID_OK, (m_mode == BackupDialogMode::Create) ? "Create Backup" : "Restore Vault", wxDefaultPosition, wxSize(120, 32));
    execBtn->SetDefault();

    btnSizer->AddStretchSpacer();
    btnSizer->Add(cancelBtn, 0, wxRIGHT, 8);
    btnSizer->Add(execBtn, 0);

    formSizer->Add(btnSizer, 0, wxEXPAND);
    mainSizer->Add(formSizer, 1, wxEXPAND | wxALL, 16);

    SetSizerAndFit(mainSizer);
    ThemeManager::ApplyThemeToWindow(this);
    CenterOnParent();
}

void BackupRestoreDialog::OnExecute(wxCommandEvent&) {
    wxString filePath = m_filePicker->GetPath().Trim();
    if (filePath.empty()) {
        m_errorText->SetLabel("Please select a target file path.");
        Layout();
        return;
    }

    wxString key = m_keyCtrl->GetValue().Trim();
    if (key.empty()) {
        m_errorText->SetLabel("Please enter an encryption/decryption key.");
        Layout();
        return;
    }

    if (m_mode == BackupDialogMode::Create) {
        wxString conf = m_confirmKeyCtrl->GetValue().Trim();
        if (key != conf) {
            m_errorText->SetLabel("Encryption keys do not match.");
            Layout();
            return;
        }

        std::string err;
        if (m_storage.CreateBackup(filePath.ToStdString(), key.ToStdString(), err)) {
            wxMessageBox("Encrypted backup successfully created!\n\nLocation: " + filePath + "\n\nAll accounts, tabs, and preferences are secured with your encryption key.", "Backup Successful", wxOK | wxICON_INFORMATION, this);
            EndModal(wxID_OK);
        } else {
            m_errorText->SetLabel("Backup failed: " + wxString::FromUTF8(err.c_str()));
            Layout();
        }
    } else { // Restore
        int res = wxMessageBox("Restoring from this backup will replace current vault items with the backup contents.\n\nDo you wish to continue?", "Confirm Restore", wxYES_NO | wxNO_DEFAULT | wxICON_WARNING, this);
        if (res != wxYES) return;

        std::string err;
        int restoredItems = 0;
        int restoredTabs = 0;
        if (m_storage.RestoreBackup(filePath.ToStdString(), key.ToStdString(), err, restoredItems, restoredTabs)) {
            m_wasRestored = true;
            wxMessageBox(wxString::Format("Vault restored successfully!\n\nRestored %d accounts across %d section tabs.", restoredItems, restoredTabs), "Restore Successful", wxOK | wxICON_INFORMATION, this);
            EndModal(wxID_OK);
        } else {
            m_errorText->SetLabel("Restore failed: " + wxString::FromUTF8(err.c_str()));
            Layout();
        }
    }
}

void BackupRestoreDialog::OnCancel(wxCommandEvent&) {
    EndModal(wxID_CANCEL);
}
