#include "PinDialog.h"

wxBEGIN_EVENT_TABLE(PinDialog, wxDialog)
    EVT_BUTTON(wxID_OK, PinDialog::OnOk)
    EVT_BUTTON(wxID_CANCEL, PinDialog::OnCancel)
wxEND_EVENT_TABLE()

PinDialog::PinDialog(wxWindow* parent, PinDialogMode mode, const std::string& itemLabel)
    : wxDialog(parent, wxID_ANY, "", wxDefaultPosition, wxSize(420, 280), wxDEFAULT_DIALOG_STYLE),
      m_mode(mode),
      m_currentPinCtrl(nullptr),
      m_newPinCtrl(nullptr),
      m_confirmPinCtrl(nullptr),
      m_errorText(nullptr) {

    SetBackgroundColour(wxColour(248, 250, 252));

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    wxPanel* headerPanel = new wxPanel(this, wxID_ANY);
    headerPanel->SetBackgroundColour(wxColour(15, 23, 42)); // Dark Slate
    wxBoxSizer* headerSizer = new wxBoxSizer(wxVERTICAL);

    wxString titleText;
    wxString subText;

    if (m_mode == PinDialogMode::UnlockItem) {
        SetTitle("Password Locker - PIN Verification");
        titleText = "PIN Verification Required";
        subText = itemLabel.empty() ? "Enter your master PIN to view stored account details."
                                    : "Enter master PIN to view: " + wxString::FromUTF8(itemLabel.c_str());
    } else if (m_mode == PinDialogMode::SetupInitialPin) {
        SetTitle("Password Locker - Setup Master PIN");
        titleText = "Set Your Vault Master PIN";
        subText = "Choose a secure PIN (4-12 digits) to protect your offline vault.";
    } else {
        SetTitle("Password Locker - Change Master PIN");
        titleText = "Change Master PIN";
        subText = "Enter your current PIN and choose a new PIN.";
    }

    wxStaticText* headerTitle = new wxStaticText(headerPanel, wxID_ANY, titleText);
    headerTitle->SetForegroundColour(*wxWHITE);
    wxFont titleFont = headerTitle->GetFont();
    titleFont.SetPointSize(12);
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    headerTitle->SetFont(titleFont);

    wxStaticText* headerSub = new wxStaticText(headerPanel, wxID_ANY, subText);
    headerSub->SetForegroundColour(wxColour(203, 213, 225));

    headerSizer->Add(headerTitle, 0, wxALL, 12);
    headerSizer->Add(headerSub, 0, wxLEFT | wxRIGHT | wxBOTTOM, 12);
    headerPanel->SetSizer(headerSizer);

    mainSizer->Add(headerPanel, 0, wxEXPAND);

    // Form fields
    wxBoxSizer* formSizer = new wxBoxSizer(wxVERTICAL);

    if (m_mode == PinDialogMode::UnlockItem) {
        wxStaticText* pinLbl = new wxStaticText(this, wxID_ANY, "Enter Master PIN (Default: 1234):");
        m_currentPinCtrl = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(-1, 32), wxTE_PASSWORD | wxTE_PROCESS_ENTER);
        m_currentPinCtrl->Bind(wxEVT_TEXT_ENTER, [this](wxCommandEvent& evt) {
            wxCommandEvent okEvt(wxEVT_BUTTON, wxID_OK);
            ProcessEvent(okEvt);
        });

        formSizer->Add(pinLbl, 0, wxBOTTOM, 6);
        formSizer->Add(m_currentPinCtrl, 0, wxEXPAND | wxBOTTOM, 10);
    } else if (m_mode == PinDialogMode::SetupInitialPin) {
        wxStaticText* pinLbl = new wxStaticText(this, wxID_ANY, "Create New PIN:");
        m_newPinCtrl = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(-1, 32), wxTE_PASSWORD);

        wxStaticText* confirmLbl = new wxStaticText(this, wxID_ANY, "Confirm PIN:");
        m_confirmPinCtrl = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(-1, 32), wxTE_PASSWORD);

        formSizer->Add(pinLbl, 0, wxBOTTOM, 4);
        formSizer->Add(m_newPinCtrl, 0, wxEXPAND | wxBOTTOM, 8);
        formSizer->Add(confirmLbl, 0, wxBOTTOM, 4);
        formSizer->Add(m_confirmPinCtrl, 0, wxEXPAND | wxBOTTOM, 10);
    } else { // ChangePin
        wxStaticText* currLbl = new wxStaticText(this, wxID_ANY, "Current PIN:");
        m_currentPinCtrl = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(-1, 30), wxTE_PASSWORD);

        wxStaticText* newLbl = new wxStaticText(this, wxID_ANY, "New PIN:");
        m_newPinCtrl = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(-1, 30), wxTE_PASSWORD);

        wxStaticText* confLbl = new wxStaticText(this, wxID_ANY, "Confirm New PIN:");
        m_confirmPinCtrl = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(-1, 30), wxTE_PASSWORD);

        formSizer->Add(currLbl, 0, wxBOTTOM, 3);
        formSizer->Add(m_currentPinCtrl, 0, wxEXPAND | wxBOTTOM, 6);
        formSizer->Add(newLbl, 0, wxBOTTOM, 3);
        formSizer->Add(m_newPinCtrl, 0, wxEXPAND | wxBOTTOM, 6);
        formSizer->Add(confLbl, 0, wxBOTTOM, 3);
        formSizer->Add(m_confirmPinCtrl, 0, wxEXPAND | wxBOTTOM, 8);
    }

    m_errorText = new wxStaticText(this, wxID_ANY, "");
    m_errorText->SetForegroundColour(wxColour(220, 38, 38)); // Red error
    formSizer->Add(m_errorText, 0, wxBOTTOM, 8);

    // Buttons
    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* cancelBtn = new wxButton(this, wxID_CANCEL, "Cancel", wxDefaultPosition, wxSize(90, 32));
    wxButton* okBtn = new wxButton(this, wxID_OK, m_mode == PinDialogMode::UnlockItem ? "Unlock" : "Save PIN", wxDefaultPosition, wxSize(100, 32));
    okBtn->SetDefault();

    btnSizer->AddStretchSpacer();
    btnSizer->Add(cancelBtn, 0, wxRIGHT, 8);
    btnSizer->Add(okBtn, 0);

    formSizer->Add(btnSizer, 0, wxEXPAND);

    mainSizer->Add(formSizer, 1, wxEXPAND | wxALL, 16);

    SetSizerAndFit(mainSizer);
    CenterOnParent();

    if (m_currentPinCtrl) m_currentPinCtrl->SetFocus();
    else if (m_newPinCtrl) m_newPinCtrl->SetFocus();
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

void PinDialog::OnOk(wxCommandEvent& evt) {
    if (m_mode == PinDialogMode::UnlockItem) {
        if (m_currentPinCtrl->GetValue().Trim().empty()) {
            m_errorText->SetLabel("Please enter your PIN.");
            Layout();
            return;
        }
    } else if (m_mode == PinDialogMode::SetupInitialPin) {
        wxString p1 = m_newPinCtrl->GetValue().Trim();
        wxString p2 = m_confirmPinCtrl->GetValue().Trim();
        if (p1.length() < 4) {
            m_errorText->SetLabel("PIN must be at least 4 digits/characters.");
            Layout();
            return;
        }
        if (p1 != p2) {
            m_errorText->SetLabel("PINs do not match. Please re-enter.");
            Layout();
            return;
        }
    } else if (m_mode == PinDialogMode::ChangePin) {
        wxString p0 = m_currentPinCtrl->GetValue().Trim();
        wxString p1 = m_newPinCtrl->GetValue().Trim();
        wxString p2 = m_confirmPinCtrl->GetValue().Trim();
        if (p0.empty()) {
            m_errorText->SetLabel("Enter your current PIN.");
            Layout();
            return;
        }
        if (p1.length() < 4) {
            m_errorText->SetLabel("New PIN must be at least 4 characters.");
            Layout();
            return;
        }
        if (p1 != p2) {
            m_errorText->SetLabel("New PIN confirmation does not match.");
            Layout();
            return;
        }
    }

    EndModal(wxID_OK);
}

void PinDialog::OnCancel(wxCommandEvent& evt) {
    EndModal(wxID_CANCEL);
}
