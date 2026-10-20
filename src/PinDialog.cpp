#include "PinDialog.h"
#include "ThemeManager.h"
#include <cctype>

wxBEGIN_EVENT_TABLE(PinDialog, wxDialog)
    EVT_BUTTON(wxID_OK, PinDialog::OnOk)
    EVT_BUTTON(wxID_CANCEL, PinDialog::OnCancel)
wxEND_EVENT_TABLE()

PinDialog::PinDialog(wxWindow* parent, PinDialogMode mode, const std::string& itemLabel, bool usePassphraseMode)
    : wxDialog(parent, wxID_ANY, "", wxDefaultPosition, wxSize(460, 360), wxDEFAULT_DIALOG_STYLE),
      m_mode(mode),
      m_usePassphraseMode(usePassphraseMode),
      m_pinRadio(nullptr),
      m_passRadio(nullptr),
      m_currentPinCtrl(nullptr),
      m_newPinCtrl(nullptr),
      m_confirmPinCtrl(nullptr),
      m_errorText(nullptr),
      m_toggleShowCurrentBtn(nullptr),
      m_toggleShowNewBtn(nullptr),
      m_showingCurrent(false),
      m_showingNew(false) {

    SetBackgroundColour(ThemeManager::Colors.windowBg);
    SetupUI(itemLabel);
}

void PinDialog::SetupUI(const std::string& itemLabel) {
    const auto& th = ThemeManager::Colors;
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    wxPanel* headerPanel = new wxPanel(this, wxID_ANY);
    headerPanel->SetBackgroundColour(th.headerBg);
    wxBoxSizer* headerSizer = new wxBoxSizer(wxVERTICAL);

    wxString titleText;
    wxString subText;

    if (m_mode == PinDialogMode::UnlockItem) {
        SetTitle("Password Locker - Master Key Verification");
        titleText = "Authentication Required";
        subText = itemLabel.empty() ? "Enter your Master PIN or Passphrase to unlock stored account details."
                                    : "Enter Master Key to unlock: " + wxString::FromUTF8(itemLabel.c_str());
    } else if (m_mode == PinDialogMode::SetupInitialPin) {
        SetTitle("Password Locker - Setup Master Key");
        titleText = "Setup Vault Master Key";
        subText = "Choose a 4-digit PIN or a strong Passphrase (6-63 characters).";
    } else {
        SetTitle("Password Locker - Change Master Key");
        titleText = "Change Master PIN / Passphrase";
        subText = "Enter your current key and select your new PIN or Passphrase.";
    }

    wxStaticText* headerTitle = new wxStaticText(headerPanel, wxID_ANY, titleText);
    headerTitle->SetForegroundColour(th.headerText);
    wxFont titleFont = headerTitle->GetFont();
    titleFont.SetPointSize(12);
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    headerTitle->SetFont(titleFont);

    wxStaticText* headerSub = new wxStaticText(headerPanel, wxID_ANY, subText);
    headerSub->SetForegroundColour((ThemeManager::CurrentTheme == ThemeMode::HighContrast) ? th.textSecondary : wxColour(203, 213, 225));

    headerSizer->Add(headerTitle, 0, wxALL, 12);
    headerSizer->Add(headerSub, 0, wxLEFT | wxRIGHT | wxBOTTOM, 12);
    headerPanel->SetSizer(headerSizer);

    mainSizer->Add(headerPanel, 0, wxEXPAND);

    // Form fields
    wxBoxSizer* formSizer = new wxBoxSizer(wxVERTICAL);

    // Option to choose between PIN and Passphrase for setup or change
    if (m_mode != PinDialogMode::UnlockItem) {
        wxBoxSizer* radioRow = new wxBoxSizer(wxHORIZONTAL);
        m_pinRadio = new wxRadioButton(this, wxID_ANY, "4-Digit PIN", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
        m_passRadio = new wxRadioButton(this, wxID_ANY, "Passphrase / Password (6-63 chars)");

        m_pinRadio->SetForegroundColour(th.textPrimary);
        m_passRadio->SetForegroundColour(th.textPrimary);

        if (m_usePassphraseMode) {
            m_passRadio->SetValue(true);
        } else {
            m_pinRadio->SetValue(true);
        }

        m_pinRadio->Bind(wxEVT_RADIOBUTTON, &PinDialog::OnModeRadioChanged, this);
        m_passRadio->Bind(wxEVT_RADIOBUTTON, &PinDialog::OnModeRadioChanged, this);

        radioRow->Add(m_pinRadio, 0, wxRIGHT, 14);
        radioRow->Add(m_passRadio, 0);
        formSizer->Add(radioRow, 0, wxBOTTOM, 10);
    }

    if (m_mode == PinDialogMode::UnlockItem) {
        wxStaticText* pinLbl = new wxStaticText(this, wxID_ANY, "Master PIN or Passphrase (Default: 1234):");
        pinLbl->SetForegroundColour(th.textPrimary);

        wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);
        m_currentPinCtrl = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(-1, 30), wxTE_PASSWORD | wxTE_PROCESS_ENTER);
        m_currentPinCtrl->Bind(wxEVT_TEXT_ENTER, [this](wxCommandEvent&) {
            wxCommandEvent okEvt(wxEVT_BUTTON, wxID_OK);
            ProcessEvent(okEvt);
        });

        m_toggleShowCurrentBtn = new wxButton(this, wxID_ANY, "Show", wxDefaultPosition, wxSize(55, 30));
        m_toggleShowCurrentBtn->Bind(wxEVT_BUTTON, &PinDialog::OnToggleShowCurrent, this);

        row->Add(m_currentPinCtrl, 1, wxEXPAND | wxRIGHT, 6);
        row->Add(m_toggleShowCurrentBtn, 0);

        formSizer->Add(pinLbl, 0, wxBOTTOM, 6);
        formSizer->Add(row, 0, wxEXPAND | wxBOTTOM, 10);
    } else if (m_mode == PinDialogMode::SetupInitialPin) {
        wxStaticText* pinLbl = new wxStaticText(this, wxID_ANY, "Create New Key:");
        pinLbl->SetForegroundColour(th.textPrimary);

        wxBoxSizer* r1 = new wxBoxSizer(wxHORIZONTAL);
        m_newPinCtrl = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(-1, 30), wxTE_PASSWORD);
        m_toggleShowNewBtn = new wxButton(this, wxID_ANY, "Show", wxDefaultPosition, wxSize(55, 30));
        m_toggleShowNewBtn->Bind(wxEVT_BUTTON, &PinDialog::OnToggleShowNew, this);
        r1->Add(m_newPinCtrl, 1, wxEXPAND | wxRIGHT, 6);
        r1->Add(m_toggleShowNewBtn, 0);

        wxStaticText* confirmLbl = new wxStaticText(this, wxID_ANY, "Confirm Key:");
        confirmLbl->SetForegroundColour(th.textPrimary);
        m_confirmPinCtrl = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(-1, 30), wxTE_PASSWORD);

        formSizer->Add(pinLbl, 0, wxBOTTOM, 4);
        formSizer->Add(r1, 0, wxEXPAND | wxBOTTOM, 8);
        formSizer->Add(confirmLbl, 0, wxBOTTOM, 4);
        formSizer->Add(m_confirmPinCtrl, 0, wxEXPAND | wxBOTTOM, 10);
    } else { // ChangePin
        wxStaticText* currLbl = new wxStaticText(this, wxID_ANY, "Current PIN / Passphrase:");
        currLbl->SetForegroundColour(th.textPrimary);

        wxBoxSizer* r0 = new wxBoxSizer(wxHORIZONTAL);
        m_currentPinCtrl = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(-1, 30), wxTE_PASSWORD);
        m_toggleShowCurrentBtn = new wxButton(this, wxID_ANY, "Show", wxDefaultPosition, wxSize(55, 30));
        m_toggleShowCurrentBtn->Bind(wxEVT_BUTTON, &PinDialog::OnToggleShowCurrent, this);
        r0->Add(m_currentPinCtrl, 1, wxEXPAND | wxRIGHT, 6);
        r0->Add(m_toggleShowCurrentBtn, 0);

        wxStaticText* newLbl = new wxStaticText(this, wxID_ANY, "New PIN / Passphrase:");
        newLbl->SetForegroundColour(th.textPrimary);

        wxBoxSizer* r1 = new wxBoxSizer(wxHORIZONTAL);
        m_newPinCtrl = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(-1, 30), wxTE_PASSWORD);
        m_toggleShowNewBtn = new wxButton(this, wxID_ANY, "Show", wxDefaultPosition, wxSize(55, 30));
        m_toggleShowNewBtn->Bind(wxEVT_BUTTON, &PinDialog::OnToggleShowNew, this);
        r1->Add(m_newPinCtrl, 1, wxEXPAND | wxRIGHT, 6);
        r1->Add(m_toggleShowNewBtn, 0);

        wxStaticText* confLbl = new wxStaticText(this, wxID_ANY, "Confirm New Key:");
        confLbl->SetForegroundColour(th.textPrimary);
        m_confirmPinCtrl = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(-1, 30), wxTE_PASSWORD);

        formSizer->Add(currLbl, 0, wxBOTTOM, 3);
        formSizer->Add(r0, 0, wxEXPAND | wxBOTTOM, 6);
        formSizer->Add(newLbl, 0, wxBOTTOM, 3);
        formSizer->Add(r1, 0, wxEXPAND | wxBOTTOM, 6);
        formSizer->Add(confLbl, 0, wxBOTTOM, 3);
        formSizer->Add(m_confirmPinCtrl, 0, wxEXPAND | wxBOTTOM, 8);
    }

    m_errorText = new wxStaticText(this, wxID_ANY, "");
    m_errorText->SetForegroundColour(th.danger);
    formSizer->Add(m_errorText, 0, wxBOTTOM, 8);

    // Action buttons
    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* cancelBtn = new wxButton(this, wxID_CANCEL, "Cancel", wxDefaultPosition, wxSize(90, 32));
    wxButton* okBtn = new wxButton(this, wxID_OK, m_mode == PinDialogMode::UnlockItem ? "Unlock Vault" : "Save Master Key", wxDefaultPosition, wxSize(120, 32));
    okBtn->SetDefault();

    btnSizer->AddStretchSpacer();
    btnSizer->Add(cancelBtn, 0, wxRIGHT, 8);
    btnSizer->Add(okBtn, 0);

    formSizer->Add(btnSizer, 0, wxEXPAND);
    mainSizer->Add(formSizer, 1, wxEXPAND | wxALL, 16);

    SetSizerAndFit(mainSizer);
    ThemeManager::ApplyThemeToWindow(this);
    CenterOnParent();

    if (m_currentPinCtrl) m_currentPinCtrl->SetFocus();
    else if (m_newPinCtrl) m_newPinCtrl->SetFocus();
}

void PinDialog::OnModeRadioChanged(wxCommandEvent&) {
    if (m_passRadio && m_passRadio->GetValue()) {
        m_usePassphraseMode = true;
    } else {
        m_usePassphraseMode = false;
    }
}

void PinDialog::OnToggleShowCurrent(wxCommandEvent&) {
    if (!m_currentPinCtrl) return;
    m_showingCurrent = !m_showingCurrent;

    wxString curVal = m_currentPinCtrl->GetValue();
    wxWindow* parent = m_currentPinCtrl->GetParent();
    wxSizer* sizer = m_currentPinCtrl->GetContainingSizer();

    int idx = 0;
    for (size_t i = 0; i < sizer->GetChildren().size(); ++i) {
        if (sizer->GetItem(i)->GetWindow() == m_currentPinCtrl) {
            idx = static_cast<int>(i);
            break;
        }
    }

    m_currentPinCtrl->Destroy();
    long style = m_showingCurrent ? wxTE_PROCESS_ENTER : (wxTE_PASSWORD | wxTE_PROCESS_ENTER);
    m_currentPinCtrl = new wxTextCtrl(parent, wxID_ANY, curVal, wxDefaultPosition, wxSize(-1, 30), style);
    ThemeManager::ApplyThemeToWindow(m_currentPinCtrl);
    m_currentPinCtrl->Bind(wxEVT_TEXT_ENTER, [this](wxCommandEvent&) {
        wxCommandEvent okEvt(wxEVT_BUTTON, wxID_OK);
        ProcessEvent(okEvt);
    });

    sizer->Insert(idx, m_currentPinCtrl, 1, wxEXPAND | wxRIGHT, 6);
    m_toggleShowCurrentBtn->SetLabel(m_showingCurrent ? "Hide" : "Show");
    parent->Layout();
    m_currentPinCtrl->SetFocus();
}

void PinDialog::OnToggleShowNew(wxCommandEvent&) {
    if (!m_newPinCtrl) return;
    m_showingNew = !m_showingNew;

    wxString curVal = m_newPinCtrl->GetValue();
    wxWindow* parent = m_newPinCtrl->GetParent();
    wxSizer* sizer = m_newPinCtrl->GetContainingSizer();

    int idx = 0;
    for (size_t i = 0; i < sizer->GetChildren().size(); ++i) {
        if (sizer->GetItem(i)->GetWindow() == m_newPinCtrl) {
            idx = static_cast<int>(i);
            break;
        }
    }

    m_newPinCtrl->Destroy();
    long style = m_showingNew ? 0 : wxTE_PASSWORD;
    m_newPinCtrl = new wxTextCtrl(parent, wxID_ANY, curVal, wxDefaultPosition, wxSize(-1, 30), style);
    ThemeManager::ApplyThemeToWindow(m_newPinCtrl);

    sizer->Insert(idx, m_newPinCtrl, 1, wxEXPAND | wxRIGHT, 6);
    m_toggleShowNewBtn->SetLabel(m_showingNew ? "Hide" : "Show");
    parent->Layout();
    m_newPinCtrl->SetFocus();
}

void PinDialog::OnOk(wxCommandEvent&) {
    if (m_mode == PinDialogMode::UnlockItem) {
        if (m_currentPinCtrl->GetValue().Trim().empty()) {
            m_errorText->SetLabel("Please enter your PIN or Passphrase.");
            Layout();
            return;
        }
    } else if (m_mode == PinDialogMode::SetupInitialPin) {
        wxString p1 = m_newPinCtrl->GetValue().Trim();
        wxString p2 = m_confirmPinCtrl->GetValue().Trim();
        if (m_usePassphraseMode) {
            if (p1.length() < 6 || p1.length() > 63) {
                m_errorText->SetLabel("Passphrase must be between 6 and 63 characters.");
                Layout();
                return;
            }
        } else {
            if (p1.length() < 4 || p1.length() > 8) {
                m_errorText->SetLabel("PIN must be between 4 and 8 digits.");
                Layout();
                return;
            }
            for (wxUniChar c : p1) {
                if (!wxIsdigit(c)) {
                    m_errorText->SetLabel("PIN must contain digits only.");
                    Layout();
                    return;
                }
            }
        }
        if (p1 != p2) {
            m_errorText->SetLabel("PINs/Passphrases do not match. Please re-enter.");
            Layout();
            return;
        }
    } else if (m_mode == PinDialogMode::ChangePin) {
        wxString p0 = m_currentPinCtrl->GetValue().Trim();
        wxString p1 = m_newPinCtrl->GetValue().Trim();
        wxString p2 = m_confirmPinCtrl->GetValue().Trim();
        if (p0.empty()) {
            m_errorText->SetLabel("Enter your current PIN or Passphrase.");
            Layout();
            return;
        }
        if (m_usePassphraseMode) {
            if (p1.length() < 6 || p1.length() > 63) {
                m_errorText->SetLabel("New passphrase must be between 6 and 63 characters.");
                Layout();
                return;
            }
        } else {
            if (p1.length() < 4 || p1.length() > 8) {
                m_errorText->SetLabel("New PIN must be between 4 and 8 digits.");
                Layout();
                return;
            }
            for (wxUniChar c : p1) {
                if (!wxIsdigit(c)) {
                    m_errorText->SetLabel("New PIN must contain digits only.");
                    Layout();
                    return;
                }
            }
        }
        if (p1 != p2) {
            m_errorText->SetLabel("New PINs/Passphrases do not match. Please re-enter.");
            Layout();
            return;
        }
    }

    EndModal(wxID_OK);
}

void PinDialog::OnCancel(wxCommandEvent&) {
    EndModal(wxID_CANCEL);
}

std::string PinDialog::GetEnteredPin() const {
    if (m_currentPinCtrl) {
        return m_currentPinCtrl->GetValue().ToStdString();
    }
    return "";
}

std::string PinDialog::GetNewPin() const {
    if (m_newPinCtrl) {
        return m_newPinCtrl->GetValue().ToStdString();
    }
    return "";
}
