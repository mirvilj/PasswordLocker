#include "EntryDialog.h"
#include "TotpEngine.h"
#include "VaultCrypto.h"
#include "ThemeManager.h"
#include <wx/mstream.h>
#include <wx/wfstream.h>
#include <wx/dcmemory.h>
#include <random>

EntryDialog::EntryDialog(wxWindow* parent,
                         const std::vector<std::string>& availableTabs,
                         const std::string& defaultTab,
                         const VaultItem* existingItem)
    : wxDialog(parent, wxID_ANY, "", wxDefaultPosition, wxSize(680, 760), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
      m_availableTabs(availableTabs),
      m_isEditing(existingItem != nullptr),
      m_bodyScroll(nullptr),
      m_typeBox(nullptr),
      m_typeBasicRadio(nullptr),
      m_typeSecQuestionsRadio(nullptr),
      m_typeMiscRadio(nullptr),
      m_labelCtrl(nullptr),
      m_tabCombo(nullptr),
      m_newTabBtn(nullptr),
      m_usernameCtrl(nullptr),
      m_emailCtrl(nullptr),
      m_passwordCtrl(nullptr),
      m_togglePasswordBtn(nullptr),
      m_generatePasswordBtn(nullptr),
      m_websiteCtrl(nullptr),
      m_totpSecretCtrl(nullptr),
      m_totpStatusLabel(nullptr),
      m_notesCtrl(nullptr),
      m_basicCredsPanel(nullptr),
      m_secQuestionsSection(nullptr),
      m_miscCustomSection(nullptr),
      m_questionsScroll(nullptr),
      m_questionsSizer(nullptr),
      m_customFieldsScroll(nullptr),
      m_customFieldsSizer(nullptr),
      m_addQuestionBtn(nullptr),
      m_addCustomFieldBtn(nullptr),
      m_imagePanel(nullptr),
      m_imageThumbBmp(nullptr),
      m_imageNameLabel(nullptr),
      m_uploadImageBtn(nullptr),
      m_removeImageBtn(nullptr) {

    if (m_isEditing && existingItem) {
        m_resultItem = *existingItem;
        SetTitle("Password Locker - Edit Account: " + wxString::FromUTF8(existingItem->label.c_str()));
    } else {
        m_resultItem = VaultItem::CreateNew(AccountType::Basic, "", defaultTab);
        SetTitle("Password Locker - Add New Vault Entry");
    }

    SetBackgroundColour(ThemeManager::Colors.windowBg);
    SetupUI();

    if (m_isEditing && existingItem) {
        if (existingItem->type == AccountType::SecurityQuestions) {
            m_typeSecQuestionsRadio->SetValue(true);
        } else if (existingItem->type == AccountType::Miscellaneous) {
            m_typeMiscRadio->SetValue(true);
        } else {
            m_typeBasicRadio->SetValue(true);
        }
        m_labelCtrl->SetValue(wxString::FromUTF8(existingItem->label.c_str()));
        m_tabCombo->SetValue(wxString::FromUTF8(existingItem->tabName.c_str()));
        m_usernameCtrl->SetValue(wxString::FromUTF8(existingItem->username.c_str()));
        m_emailCtrl->SetValue(wxString::FromUTF8(existingItem->email.c_str()));
        m_passwordCtrl->SetValue(wxString::FromUTF8(existingItem->password.c_str()));
        m_websiteCtrl->SetValue(wxString::FromUTF8(existingItem->website.c_str()));
        m_totpSecretCtrl->SetValue(wxString::FromUTF8(existingItem->totpSecret.c_str()));
        m_notesCtrl->SetValue(wxString::FromUTF8(existingItem->notes.c_str()));

        for (const auto& sq : existingItem->securityQuestions) {
            AddQuestionRow(sq.question, sq.answer);
        }
        for (const auto& cf : existingItem->customFields) {
            AddCustomFieldRow(cf.label, cf.value);
        }
    } else {
        // Pre-add one initial empty row for questions and custom fields
        AddQuestionRow("", "");
        AddCustomFieldRow("", "");
    }

    UpdateImagePreview();
    AdaptFieldsToType(GetSelectedAccountType());
}

AccountType EntryDialog::GetSelectedAccountType() const {
    if (m_typeSecQuestionsRadio && m_typeSecQuestionsRadio->GetValue()) {
        return AccountType::SecurityQuestions;
    }
    if (m_typeMiscRadio && m_typeMiscRadio->GetValue()) {
        return AccountType::Miscellaneous;
    }
    return AccountType::Basic;
}

void EntryDialog::OnTypeRadioChanged(wxCommandEvent&) {
    AdaptFieldsToType(GetSelectedAccountType());
}

void EntryDialog::AdaptFieldsToType(AccountType type) {
    if (type == AccountType::Basic) {
        if (m_basicCredsPanel) m_basicCredsPanel->Show(true);
        if (m_secQuestionsSection) m_secQuestionsSection->Show(false);
        if (m_miscCustomSection) m_miscCustomSection->Show(false);
    } else if (type == AccountType::SecurityQuestions) {
        if (m_basicCredsPanel) m_basicCredsPanel->Show(true);
        if (m_secQuestionsSection) m_secQuestionsSection->Show(true);
        if (m_miscCustomSection) m_miscCustomSection->Show(false);
    } else { // Miscellaneous
        if (m_basicCredsPanel) m_basicCredsPanel->Show(false);
        if (m_secQuestionsSection) m_secQuestionsSection->Show(false);
        if (m_miscCustomSection) m_miscCustomSection->Show(true);
    }

    if (m_bodyScroll) {
        m_bodyScroll->FitInside();
        m_bodyScroll->Layout();
    }
    Layout();
}

void EntryDialog::SetupUI() {
    const auto& th = ThemeManager::Colors;
    wxBoxSizer* rootSizer = new wxBoxSizer(wxVERTICAL);

    // Top Header
    wxPanel* headerPanel = new wxPanel(this, wxID_ANY);
    headerPanel->SetBackgroundColour(th.headerBg);
    wxBoxSizer* headerSizer = new wxBoxSizer(wxVERTICAL);

    wxStaticText* titleTxt = new wxStaticText(headerPanel, wxID_ANY, m_isEditing ? "Edit Account Record" : "Add New Vault Record");
    titleTxt->SetForegroundColour(th.headerText);
    wxFont f = titleTxt->GetFont();
    f.SetPointSize(13);
    f.SetWeight(wxFONTWEIGHT_BOLD);
    titleTxt->SetFont(f);

    wxStaticText* descTxt = new wxStaticText(headerPanel, wxID_ANY,
        "Select account type below. Fields dynamically adapt to match your selection.");
    descTxt->SetForegroundColour((ThemeManager::CurrentTheme == ThemeMode::HighContrast) ? th.textSecondary : wxColour(148, 163, 184));

    headerSizer->Add(titleTxt, 0, wxALL, 12);
    headerSizer->Add(descTxt, 0, wxLEFT | wxRIGHT | wxBOTTOM, 12);
    headerPanel->SetSizer(headerSizer);
    rootSizer->Add(headerPanel, 0, wxEXPAND);

    // Main Scrollable Body
    m_bodyScroll = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL);
    m_bodyScroll->SetScrollRate(0, 10);
    m_bodyScroll->SetBackgroundColour(th.windowBg);
    wxBoxSizer* bodySizer = new wxBoxSizer(wxVERTICAL);

    // 1. Account Type Selector
    m_typeBox = new wxStaticBoxSizer(wxVERTICAL, m_bodyScroll, "Select Account Type:");
    m_typeBox->GetStaticBox()->SetForegroundColour((ThemeManager::CurrentTheme == ThemeMode::HighContrast) ? th.textSecondary : th.textPrimary);
    m_typeBox->GetStaticBox()->SetBackgroundColour(th.windowBg);

    m_typeBasicRadio = new wxRadioButton(m_typeBox->GetStaticBox(), wxID_ANY,
                                         "1. Basic App Password (General Purpose)",
                                         wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    m_typeSecQuestionsRadio = new wxRadioButton(m_typeBox->GetStaticBox(), wxID_ANY,
                                                "2. Account with Security Questions");
    m_typeMiscRadio = new wxRadioButton(m_typeBox->GetStaticBox(), wxID_ANY,
                                        "3. Miscellaneous Information (Custom Fields & Tabs)");

    m_typeBasicRadio->SetValue(true);
    m_typeBasicRadio->SetForegroundColour(th.textPrimary);
    m_typeBasicRadio->SetBackgroundColour(th.windowBg);
    m_typeSecQuestionsRadio->SetForegroundColour(th.textPrimary);
    m_typeSecQuestionsRadio->SetBackgroundColour(th.windowBg);
    m_typeMiscRadio->SetForegroundColour(th.textPrimary);
    m_typeMiscRadio->SetBackgroundColour(th.windowBg);

    m_typeBasicRadio->Bind(wxEVT_RADIOBUTTON, &EntryDialog::OnTypeRadioChanged, this);
    m_typeSecQuestionsRadio->Bind(wxEVT_RADIOBUTTON, &EntryDialog::OnTypeRadioChanged, this);
    m_typeMiscRadio->Bind(wxEVT_RADIOBUTTON, &EntryDialog::OnTypeRadioChanged, this);

    m_typeBox->Add(m_typeBasicRadio, 0, wxALL, 6);
    m_typeBox->Add(m_typeSecQuestionsRadio, 0, wxLEFT | wxRIGHT | wxBOTTOM, 6);
    m_typeBox->Add(m_typeMiscRadio, 0, wxLEFT | wxRIGHT | wxBOTTOM, 6);
    bodySizer->Add(m_typeBox, 0, wxEXPAND | wxALL, 12);

    // 2. Common Header Fields: Label / Name & Tab Category
    wxFlexGridSizer* topGrid = new wxFlexGridSizer(2, 2, 8, 10);
    topGrid->AddGrowableCol(0, 1);
    topGrid->AddGrowableCol(1, 1);

    wxBoxSizer* labelBox = new wxBoxSizer(wxVERTICAL);
    wxStaticText* lblLabel = new wxStaticText(m_bodyScroll, wxID_ANY, "Account / Service Label *:");
    lblLabel->SetForegroundColour(th.textPrimary);
    labelBox->Add(lblLabel, 0, wxBOTTOM, 4);
    m_labelCtrl = new wxTextCtrl(m_bodyScroll, wxID_ANY, "", wxDefaultPosition, wxSize(-1, 30));
    m_labelCtrl->SetHint("e.g. Google, GitHub, Work VPN, Bank");
    labelBox->Add(m_labelCtrl, 0, wxEXPAND);

    wxBoxSizer* tabBox = new wxBoxSizer(wxVERTICAL);
    wxStaticText* tabPrompt = new wxStaticText(m_bodyScroll, wxID_ANY, "Category / Tab Section:");
    tabPrompt->SetForegroundColour(th.textPrimary);
    tabBox->Add(tabPrompt, 0, wxBOTTOM, 4);
    wxBoxSizer* tabRow = new wxBoxSizer(wxHORIZONTAL);
    wxArrayString tabChoices;
    for (const auto& t : m_availableTabs) {
        tabChoices.Add(wxString::FromUTF8(t.c_str()));
    }
    m_tabCombo = new wxComboBox(m_bodyScroll, wxID_ANY, "General", wxDefaultPosition, wxDefaultSize, tabChoices, wxCB_DROPDOWN);
    m_newTabBtn = new wxButton(m_bodyScroll, wxID_ANY, "+ New Tab", wxDefaultPosition, wxSize(80, 30));
    m_newTabBtn->Bind(wxEVT_BUTTON, &EntryDialog::OnAddNewTabClicked, this);
    tabRow->Add(m_tabCombo, 1, wxEXPAND | wxRIGHT, 6);
    tabRow->Add(m_newTabBtn, 0);
    tabBox->Add(tabRow, 0, wxEXPAND);

    topGrid->Add(labelBox, 1, wxEXPAND);
    topGrid->Add(tabBox, 1, wxEXPAND);
    bodySizer->Add(topGrid, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 12);

    // 3. Basic Credentials Panel (Username, Email, Password, URL)
    m_basicCredsPanel = new wxPanel(m_bodyScroll, wxID_ANY);
    m_basicCredsPanel->SetBackgroundColour(th.cardBg);
    wxBoxSizer* credsSizer = new wxBoxSizer(wxVERTICAL);

    wxStaticText* credsTitle = new wxStaticText(m_basicCredsPanel, wxID_ANY, "Account Credentials");
    wxFont credsFont = credsTitle->GetFont();
    credsFont.SetWeight(wxFONTWEIGHT_BOLD);
    credsTitle->SetFont(credsFont);
    credsTitle->SetForegroundColour(th.textPrimary);
    credsSizer->Add(credsTitle, 0, wxALL, 8);

    wxFlexGridSizer* credsGrid = new wxFlexGridSizer(2, 2, 8, 10);
    credsGrid->AddGrowableCol(0, 1);
    credsGrid->AddGrowableCol(1, 1);

    // Username
    wxBoxSizer* userBox = new wxBoxSizer(wxVERTICAL);
    wxStaticText* userLabel = new wxStaticText(m_basicCredsPanel, wxID_ANY, "Username (optional):");
    userLabel->SetForegroundColour(th.textPrimary);
    userBox->Add(userLabel, 0, wxBOTTOM, 4);
    m_usernameCtrl = new wxTextCtrl(m_basicCredsPanel, wxID_ANY, "", wxDefaultPosition, wxSize(-1, 30));
    m_usernameCtrl->SetHint("e.g. johndoe");
    userBox->Add(m_usernameCtrl, 0, wxEXPAND);
    credsGrid->Add(userBox, 1, wxEXPAND);

    // Email
    wxBoxSizer* emailBox = new wxBoxSizer(wxVERTICAL);
    wxStaticText* emailLabel = new wxStaticText(m_basicCredsPanel, wxID_ANY, "Email Address (optional):");
    emailLabel->SetForegroundColour(th.textPrimary);
    emailBox->Add(emailLabel, 0, wxBOTTOM, 4);
    m_emailCtrl = new wxTextCtrl(m_basicCredsPanel, wxID_ANY, "", wxDefaultPosition, wxSize(-1, 30));
    m_emailCtrl->SetHint("e.g. john@example.com");
    emailBox->Add(m_emailCtrl, 0, wxEXPAND);
    credsGrid->Add(emailBox, 1, wxEXPAND);

    credsSizer->Add(credsGrid, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);

    // Password row with Show/Hide and Generate
    wxStaticText* passLabel = new wxStaticText(m_basicCredsPanel, wxID_ANY, "Password:");
    passLabel->SetForegroundColour(th.textPrimary);
    credsSizer->Add(passLabel, 0, wxLEFT | wxRIGHT, 8);

    wxBoxSizer* passRow = new wxBoxSizer(wxHORIZONTAL);
    m_passwordCtrl = new wxTextCtrl(m_basicCredsPanel, wxID_ANY, "", wxDefaultPosition, wxSize(-1, 30), wxTE_PASSWORD);
    m_togglePasswordBtn = new wxButton(m_basicCredsPanel, wxID_ANY, "Show", wxDefaultPosition, wxSize(55, 30));
    m_togglePasswordBtn->Bind(wxEVT_BUTTON, &EntryDialog::OnTogglePasswordClicked, this);
    m_generatePasswordBtn = new wxButton(m_basicCredsPanel, wxID_ANY, "Generate", wxDefaultPosition, wxSize(75, 30));
    m_generatePasswordBtn->Bind(wxEVT_BUTTON, &EntryDialog::OnGeneratePasswordClicked, this);

    passRow->Add(m_passwordCtrl, 1, wxEXPAND | wxRIGHT, 6);
    passRow->Add(m_togglePasswordBtn, 0, wxRIGHT, 4);
    passRow->Add(m_generatePasswordBtn, 0);
    credsSizer->Add(passRow, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);

    // Website URL
    wxStaticText* urlLabel = new wxStaticText(m_basicCredsPanel, wxID_ANY, "Website URL (optional):");
    urlLabel->SetForegroundColour(th.textPrimary);
    credsSizer->Add(urlLabel, 0, wxLEFT | wxRIGHT, 8);
    m_websiteCtrl = new wxTextCtrl(m_basicCredsPanel, wxID_ANY, "", wxDefaultPosition, wxSize(-1, 30));
    m_websiteCtrl->SetHint("e.g. https://example.com");
    credsSizer->Add(m_websiteCtrl, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);

    m_basicCredsPanel->SetSizer(credsSizer);
    bodySizer->Add(m_basicCredsPanel, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 12);

    // 4. Security Questions Section
    m_secQuestionsSection = new wxPanel(m_bodyScroll, wxID_ANY);
    m_secQuestionsSection->SetBackgroundColour(th.cardBg);
    wxBoxSizer* sqHeaderSizer = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer* sqTopRow = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* sqTitle = new wxStaticText(m_secQuestionsSection, wxID_ANY, "Security Questions & Answers");
    wxFont sqFont = sqTitle->GetFont();
    sqFont.SetWeight(wxFONTWEIGHT_BOLD);
    sqTitle->SetFont(sqFont);
    sqTitle->SetForegroundColour(th.textPrimary);

    m_addQuestionBtn = new wxButton(m_secQuestionsSection, wxID_ANY, "+ Add Question", wxDefaultPosition, wxSize(120, 28));
    m_addQuestionBtn->Bind(wxEVT_BUTTON, &EntryDialog::OnAddQuestionClicked, this);

    sqTopRow->Add(sqTitle, 1, wxALIGN_CENTER_VERTICAL);
    sqTopRow->Add(m_addQuestionBtn, 0, wxALIGN_CENTER_VERTICAL);
    sqHeaderSizer->Add(sqTopRow, 0, wxEXPAND | wxALL, 8);

    m_questionsScroll = new wxScrolledWindow(m_secQuestionsSection, wxID_ANY, wxDefaultPosition, wxSize(-1, 140), wxVSCROLL);
    m_questionsScroll->SetScrollRate(0, 10);
    m_questionsScroll->SetBackgroundColour(th.cardBg);
    m_questionsSizer = new wxBoxSizer(wxVERTICAL);
    m_questionsScroll->SetSizer(m_questionsSizer);
    sqHeaderSizer->Add(m_questionsScroll, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);

    m_secQuestionsSection->SetSizer(sqHeaderSizer);
    bodySizer->Add(m_secQuestionsSection, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 12);

    // 5. Miscellaneous Information / Custom Fields Section
    m_miscCustomSection = new wxPanel(m_bodyScroll, wxID_ANY);
    m_miscCustomSection->SetBackgroundColour(th.cardBg);
    wxBoxSizer* miscHeaderSizer = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer* miscTopRow = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* miscTitle = new wxStaticText(m_miscCustomSection, wxID_ANY, "Custom User-Defined Fields & Labels");
    wxFont miscFont = miscTitle->GetFont();
    miscFont.SetWeight(wxFONTWEIGHT_BOLD);
    miscTitle->SetFont(miscFont);
    miscTitle->SetForegroundColour(th.textPrimary);

    m_addCustomFieldBtn = new wxButton(m_miscCustomSection, wxID_ANY, "+ Add Custom Field", wxDefaultPosition, wxSize(140, 28));
    m_addCustomFieldBtn->Bind(wxEVT_BUTTON, &EntryDialog::OnAddCustomFieldClicked, this);

    miscTopRow->Add(miscTitle, 1, wxALIGN_CENTER_VERTICAL);
    miscTopRow->Add(m_addCustomFieldBtn, 0, wxALIGN_CENTER_VERTICAL);
    miscHeaderSizer->Add(miscTopRow, 0, wxEXPAND | wxALL, 8);

    m_customFieldsScroll = new wxScrolledWindow(m_miscCustomSection, wxID_ANY, wxDefaultPosition, wxSize(-1, 140), wxVSCROLL);
    m_customFieldsScroll->SetScrollRate(0, 10);
    m_customFieldsScroll->SetBackgroundColour(th.cardBg);
    m_customFieldsSizer = new wxBoxSizer(wxVERTICAL);
    m_customFieldsScroll->SetSizer(m_customFieldsSizer);
    miscHeaderSizer->Add(m_customFieldsScroll, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);

    // Attached Image / Document upload panel
    m_imagePanel = new wxPanel(m_miscCustomSection, wxID_ANY);
    m_imagePanel->SetBackgroundColour(th.cardBg);
    wxBoxSizer* imgBoxSizer = new wxBoxSizer(wxVERTICAL);

    wxStaticText* imgTitle = new wxStaticText(m_imagePanel, wxID_ANY, "Attached Document / Image (Optional):");
    wxFont imgF = imgTitle->GetFont();
    imgF.SetWeight(wxFONTWEIGHT_BOLD);
    imgTitle->SetFont(imgF);
    imgTitle->SetForegroundColour(th.textPrimary);
    imgBoxSizer->Add(imgTitle, 0, wxALL, 6);

    wxBoxSizer* imgRow = new wxBoxSizer(wxHORIZONTAL);

    // Create a safe, valid placeholder bitmap to avoid wxWidgets wxNullBitmap assertion!
    wxImage placeholderImg(90, 60, true);
    placeholderImg.SetRGB(wxRect(0, 0, 90, 60), 220, 220, 220);
    wxBitmap placeholderBmp(placeholderImg);
    m_imageThumbBmp = new wxStaticBitmap(m_imagePanel, wxID_ANY, placeholderBmp, wxDefaultPosition, wxSize(90, 60));
    m_imageThumbBmp->Show(false);

    m_imageNameLabel = new wxStaticText(m_imagePanel, wxID_ANY, "No image attached (PNG, JPG, BMP supported).");
    m_imageNameLabel->SetForegroundColour(th.textMuted);

    m_uploadImageBtn = new wxButton(m_imagePanel, wxID_ANY, "Upload Image...", wxDefaultPosition, wxSize(120, 28));
    m_uploadImageBtn->Bind(wxEVT_BUTTON, &EntryDialog::OnUploadImage, this);

    m_removeImageBtn = new wxButton(m_imagePanel, wxID_ANY, "Remove", wxDefaultPosition, wxSize(75, 28));
    m_removeImageBtn->SetForegroundColour(wxColour(220, 38, 38));
    m_removeImageBtn->Bind(wxEVT_BUTTON, &EntryDialog::OnRemoveImage, this);
    m_removeImageBtn->Show(false);

    imgRow->Add(m_imageThumbBmp, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 6);
    imgRow->Add(m_imageNameLabel, 1, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);
    imgRow->Add(m_uploadImageBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);
    imgRow->Add(m_removeImageBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);

    imgBoxSizer->Add(imgRow, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 6);
    m_imagePanel->SetSizer(imgBoxSizer);
    miscHeaderSizer->Add(m_imagePanel, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);

    m_miscCustomSection->SetSizer(miscHeaderSizer);
    bodySizer->Add(m_miscCustomSection, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 12);

    // 6. 2-Factor Authentication (TOTP Secret Key)
    wxPanel* totpPanel = new wxPanel(m_bodyScroll, wxID_ANY);
    totpPanel->SetBackgroundColour(th.cardBg);
    wxBoxSizer* totpSizer = new wxBoxSizer(wxVERTICAL);

    wxStaticText* totpTitle = new wxStaticText(totpPanel, wxID_ANY, "Two-Factor Authentication (2FA / TOTP) Secret Key:");
    wxFont tf = totpTitle->GetFont();
    tf.SetWeight(wxFONTWEIGHT_BOLD);
    totpTitle->SetFont(tf);
    totpTitle->SetForegroundColour(th.textPrimary);
    totpSizer->Add(totpTitle, 0, wxALL, 8);

    m_totpSecretCtrl = new wxTextCtrl(totpPanel, wxID_ANY, "", wxDefaultPosition, wxSize(-1, 30));
    m_totpSecretCtrl->SetHint("Paste Base32 TOTP secret key here (e.g. JBSWY3DPEHPK3PXP)");
    m_totpSecretCtrl->Bind(wxEVT_TEXT, &EntryDialog::OnTotpTextChange, this);
    totpSizer->Add(m_totpSecretCtrl, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);

    m_totpStatusLabel = new wxStaticText(totpPanel, wxID_ANY, "Leave blank if this account does not use 2FA.");
    m_totpStatusLabel->SetForegroundColour(th.textMuted);
    totpSizer->Add(m_totpStatusLabel, 0, wxLEFT | wxRIGHT | wxBOTTOM, 8);

    totpPanel->SetSizer(totpSizer);
    bodySizer->Add(totpPanel, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 12);

    // 7. General Notes
    wxStaticText* notesLabel = new wxStaticText(m_bodyScroll, wxID_ANY, "Notes / Additional Information (optional):");
    notesLabel->SetForegroundColour(th.textPrimary);
    bodySizer->Add(notesLabel, 0, wxLEFT | wxRIGHT | wxBOTTOM, 4);
    m_notesCtrl = new wxTextCtrl(m_bodyScroll, wxID_ANY, "", wxDefaultPosition, wxSize(-1, 60), wxTE_MULTILINE);
    bodySizer->Add(m_notesCtrl, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 16);

    m_bodyScroll->SetSizer(bodySizer);
    rootSizer->Add(m_bodyScroll, 1, wxEXPAND);

    // Bottom Action Buttons
    wxPanel* bottomPanel = new wxPanel(this, wxID_ANY);
    bottomPanel->SetBackgroundColour(th.headerBg);
    wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);

    wxButton* cancelBtn = new wxButton(bottomPanel, wxID_CANCEL, "Cancel", wxDefaultPosition, wxSize(90, 34));
    cancelBtn->SetBackgroundColour(th.buttonBg);
    cancelBtn->SetForegroundColour(th.buttonText);

    wxButton* saveBtn = new wxButton(bottomPanel, wxID_OK, m_isEditing ? "Save Changes" : "Submit Account", wxDefaultPosition, wxSize(130, 34));
    saveBtn->SetDefault();
    if (ThemeManager::CurrentTheme == ThemeMode::HighContrast) {
        saveBtn->SetBackgroundColour(*wxBLACK);
        saveBtn->SetForegroundColour(*wxWHITE);
    } else {
        saveBtn->SetBackgroundColour(wxColour(37, 99, 235));
        saveBtn->SetForegroundColour(*wxWHITE);
    }

    cancelBtn->Bind(wxEVT_BUTTON, &EntryDialog::OnCancelClicked, this);
    saveBtn->Bind(wxEVT_BUTTON, &EntryDialog::OnSaveClicked, this);

    bottomSizer->AddStretchSpacer();
    bottomSizer->Add(cancelBtn, 0, wxRIGHT, 8);
    bottomSizer->Add(saveBtn, 0);
    bottomPanel->SetSizer(bottomSizer);

    rootSizer->Add(bottomPanel, 0, wxEXPAND | wxALL, 10);

    SetSizer(rootSizer);
    ThemeManager::ApplyThemeToWindow(this);
}

void EntryDialog::AddQuestionRow(const std::string& q, const std::string& a) {
    wxPanel* rowPanel = new wxPanel(m_questionsScroll, wxID_ANY);
    rowPanel->SetBackgroundColour(ThemeManager::Colors.cardBg);
    wxBoxSizer* rowSizer = new wxBoxSizer(wxHORIZONTAL);

    wxTextCtrl* qCtrl = new wxTextCtrl(rowPanel, wxID_ANY, wxString::FromUTF8(q.c_str()), wxDefaultPosition, wxSize(200, 28));
    qCtrl->SetHint("Security Question (e.g. First pet?)");

    wxTextCtrl* aCtrl = new wxTextCtrl(rowPanel, wxID_ANY, wxString::FromUTF8(a.c_str()), wxDefaultPosition, wxSize(160, 28));
    aCtrl->SetHint("Answer");

    wxButton* rmBtn = new wxButton(rowPanel, wxID_ANY, "Del", wxDefaultPosition, wxSize(36, 28));
    rmBtn->SetToolTip("Remove this security question");

    rowSizer->Add(qCtrl, 1, wxEXPAND | wxRIGHT, 6);
    rowSizer->Add(aCtrl, 1, wxEXPAND | wxRIGHT, 6);
    rowSizer->Add(rmBtn, 0);

    rowPanel->SetSizer(rowSizer);
    m_questionsSizer->Add(rowPanel, 0, wxEXPAND | wxBOTTOM, 6);

    QuestionRow qr;
    qr.panel = rowPanel;
    qr.questionCtrl = qCtrl;
    qr.answerCtrl = aCtrl;
    qr.removeBtn = rmBtn;
    m_questionRows.push_back(qr);

    rmBtn->Bind(wxEVT_BUTTON, [this, rowPanel](wxCommandEvent&) {
        RemoveQuestionRow(rowPanel);
    });

    m_questionsScroll->FitInside();
    m_questionsScroll->Layout();
}

void EntryDialog::RemoveQuestionRow(wxPanel* panel) {
    for (auto it = m_questionRows.begin(); it != m_questionRows.end(); ++it) {
        if (it->panel == panel) {
            panel->Destroy();
            m_questionRows.erase(it);
            break;
        }
    }
    m_questionsScroll->FitInside();
    m_questionsScroll->Layout();
}

void EntryDialog::AddCustomFieldRow(const std::string& lbl, const std::string& val) {
    wxPanel* rowPanel = new wxPanel(m_customFieldsScroll, wxID_ANY);
    rowPanel->SetBackgroundColour(ThemeManager::Colors.cardBg);
    wxBoxSizer* rowSizer = new wxBoxSizer(wxHORIZONTAL);

    wxTextCtrl* lCtrl = new wxTextCtrl(rowPanel, wxID_ANY, wxString::FromUTF8(lbl.c_str()), wxDefaultPosition, wxSize(160, 28));
    lCtrl->SetHint("Label (e.g. Pin, PUK, Port)");

    wxTextCtrl* vCtrl = new wxTextCtrl(rowPanel, wxID_ANY, wxString::FromUTF8(val.c_str()), wxDefaultPosition, wxSize(200, 28));
    vCtrl->SetHint("Value");

    wxButton* rmBtn = new wxButton(rowPanel, wxID_ANY, "Del", wxDefaultPosition, wxSize(36, 28));
    rmBtn->SetToolTip("Remove this custom field");

    rowSizer->Add(lCtrl, 1, wxEXPAND | wxRIGHT, 6);
    rowSizer->Add(vCtrl, 1, wxEXPAND | wxRIGHT, 6);
    rowSizer->Add(rmBtn, 0);

    rowPanel->SetSizer(rowSizer);
    m_customFieldsSizer->Add(rowPanel, 0, wxEXPAND | wxBOTTOM, 6);

    CustomFieldRow cfr;
    cfr.panel = rowPanel;
    cfr.labelCtrl = lCtrl;
    cfr.valueCtrl = vCtrl;
    cfr.removeBtn = rmBtn;
    m_customFieldRows.push_back(cfr);

    rmBtn->Bind(wxEVT_BUTTON, [this, rowPanel](wxCommandEvent&) {
        RemoveCustomFieldRow(rowPanel);
    });

    m_customFieldsScroll->FitInside();
    m_customFieldsScroll->Layout();
}

void EntryDialog::RemoveCustomFieldRow(wxPanel* panel) {
    for (auto it = m_customFieldRows.begin(); it != m_customFieldRows.end(); ++it) {
        if (it->panel == panel) {
            panel->Destroy();
            m_customFieldRows.erase(it);
            break;
        }
    }
    m_customFieldsScroll->FitInside();
    m_customFieldsScroll->Layout();
}

void EntryDialog::OnAddQuestionClicked(wxCommandEvent&) {
    AddQuestionRow("", "");
}

void EntryDialog::OnAddCustomFieldClicked(wxCommandEvent&) {
    AddCustomFieldRow("", "");
}

void EntryDialog::OnTogglePasswordClicked(wxCommandEvent&) {
    wxString currentVal = m_passwordCtrl->GetValue();
    long style = m_passwordCtrl->GetWindowStyle();
    bool isPassword = (style & wxTE_PASSWORD) != 0;

    wxWindow* parent = m_passwordCtrl->GetParent();
    wxSizer* sizer = m_passwordCtrl->GetContainingSizer();

    int idx = 0;
    for (size_t i = 0; i < sizer->GetChildren().size(); ++i) {
        if (sizer->GetItem(i)->GetWindow() == m_passwordCtrl) {
            idx = static_cast<int>(i);
            break;
        }
    }

    m_passwordCtrl->Destroy();

    if (isPassword) {
        m_passwordCtrl = new wxTextCtrl(parent, wxID_ANY, currentVal, wxDefaultPosition, wxSize(-1, 30));
        m_togglePasswordBtn->SetLabel("Hide");
    } else {
        m_passwordCtrl = new wxTextCtrl(parent, wxID_ANY, currentVal, wxDefaultPosition, wxSize(-1, 30), wxTE_PASSWORD);
        m_togglePasswordBtn->SetLabel("Show");
    }

    sizer->Insert(idx, m_passwordCtrl, 1, wxEXPAND | wxRIGHT, 6);
    parent->Layout();
}

void EntryDialog::OnGeneratePasswordClicked(wxCommandEvent&) {
    static const char chars[] = "ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz23456789!@#$%^&*()-_=+";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(0, sizeof(chars) - 2);

    std::string pass;
    for (int i = 0; i < 18; ++i) {
        pass.push_back(chars[dis(gen)]);
    }
    m_passwordCtrl->SetValue(pass);
}

void EntryDialog::OnAddNewTabClicked(wxCommandEvent&) {
    wxTextEntryDialog dlg(this, "Enter a new tag name for this tab:", "Create a New tab");
    if (dlg.ShowModal() == wxID_OK) {
        wxString newTab = dlg.GetValue().Trim();
        if (!newTab.empty()) {
            m_tabCombo->Append(newTab);
            m_tabCombo->SetValue(newTab);
        }
    }
}

void EntryDialog::OnTotpTextChange(wxCommandEvent&) {
    std::string secret = m_totpSecretCtrl->GetValue().ToStdString();
    const auto& th = ThemeManager::Colors;
    if (secret.empty()) {
        m_totpStatusLabel->SetLabel("Leave blank if this account does not use 2FA.");
        m_totpStatusLabel->SetForegroundColour(th.textMuted);
    } else if (TotpEngine::IsValidBase32(secret)) {
        std::string sample = TotpEngine::GenerateCode(secret);
        m_totpStatusLabel->SetLabel("Valid 2FA Base32 Secret! Current Token: " + sample);
        m_totpStatusLabel->SetForegroundColour(th.success);
    } else {
        m_totpStatusLabel->SetLabel("Invalid Base32 secret key. Allowed characters: A-Z, 2-7.");
        m_totpStatusLabel->SetForegroundColour(th.danger);
    }
}

void EntryDialog::OnSaveClicked(wxCommandEvent&) {
    wxString label = m_labelCtrl->GetValue().Trim();
    if (label.empty()) {
        wxMessageBox("Please enter an Account / Service Label.", "Validation Error", wxOK | wxICON_WARNING, this);
        m_labelCtrl->SetFocus();
        return;
    }

    AccountType type = GetSelectedAccountType();
    m_resultItem.type = type;
    m_resultItem.label = label.ToStdString();
    m_resultItem.tabName = m_tabCombo->GetValue().Trim().ToStdString();
    if (m_resultItem.tabName.empty()) m_resultItem.tabName = "General";

    m_resultItem.username = m_usernameCtrl->GetValue().Trim().ToStdString();
    m_resultItem.email = m_emailCtrl->GetValue().Trim().ToStdString();
    m_resultItem.password = m_passwordCtrl->GetValue().ToStdString();
    m_resultItem.website = m_websiteCtrl->GetValue().Trim().ToStdString();
    m_resultItem.totpSecret = m_totpSecretCtrl->GetValue().Trim().ToStdString();
    m_resultItem.notes = m_notesCtrl->GetValue().ToStdString();

    // Collect security questions
    m_resultItem.securityQuestions.clear();
    if (type == AccountType::SecurityQuestions) {
        for (const auto& row : m_questionRows) {
            std::string q = row.questionCtrl->GetValue().Trim().ToStdString();
            std::string a = row.answerCtrl->GetValue().Trim().ToStdString();
            if (!q.empty() || !a.empty()) {
                m_resultItem.securityQuestions.push_back({ q, a });
            }
        }
    }

    // Collect custom fields
    m_resultItem.customFields.clear();
    if (type == AccountType::Miscellaneous) {
        for (const auto& row : m_customFieldRows) {
            std::string l = row.labelCtrl->GetValue().Trim().ToStdString();
            std::string v = row.valueCtrl->GetValue().Trim().ToStdString();
            if (!l.empty() || !v.empty()) {
                m_resultItem.customFields.push_back({ l, v });
            }
        }
    }

    EndModal(wxID_OK);
}

void EntryDialog::OnCancelClicked(wxCommandEvent&) {
    EndModal(wxID_CANCEL);
}

VaultItem EntryDialog::GetVaultItem() const {
    return m_resultItem;
}

void EntryDialog::UpdateImagePreview() {
    if (!m_imagePanel || !m_imageThumbBmp || !m_imageNameLabel || !m_removeImageBtn) return;

    if (m_resultItem.attachedImageBase64.empty()) {
        m_imageThumbBmp->Show(false);
        m_imageNameLabel->SetLabel("No image attached (PNG, JPG, BMP supported).");
        m_removeImageBtn->Show(false);
    } else {
        std::vector<uint8_t> raw = VaultCrypto::Base64Decode(m_resultItem.attachedImageBase64);
        if (!raw.empty()) {
            wxMemoryInputStream memStream(raw.data(), raw.size());
            wxImage img(memStream);
            if (img.IsOk()) {
                wxImage thumb = img.Scale(90, 60, wxIMAGE_QUALITY_HIGH);
                m_imageThumbBmp->SetBitmap(wxBitmap(thumb));
                m_imageThumbBmp->Show(true);
            }
        }
        wxString nameStr = m_resultItem.attachedImageName.empty() ? "Attached Image" : wxString::FromUTF8(m_resultItem.attachedImageName.c_str());
        m_imageNameLabel->SetLabel(nameStr);
        m_removeImageBtn->Show(true);
    }
    m_imagePanel->Layout();
}

void EntryDialog::OnUploadImage(wxCommandEvent&) {
    wxFileDialog openFileDialog(this, "Select Image or Document to Attach", "", "",
        "Image Files (*.png;*.jpg;*.jpeg;*.bmp)|*.png;*.jpg;*.jpeg;*.bmp|All Files (*.*)|*.*",
        wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (openFileDialog.ShowModal() == wxID_CANCEL) return;

    wxString path = openFileDialog.GetPath();
    wxFileInputStream input(path);
    if (!input.IsOk()) {
        wxMessageBox("Could not open the selected file.", "Error", wxOK | wxICON_ERROR, this);
        return;
    }

    size_t size = input.GetSize();
    if (size > 15 * 1024 * 1024) { // 15MB limit
        wxMessageBox("File is too large. Please select an image under 15MB.", "File Too Large", wxOK | wxICON_WARNING, this);
        return;
    }

    std::vector<uint8_t> buf(size);
    input.Read(buf.data(), size);

    m_resultItem.attachedImageBase64 = VaultCrypto::Base64Encode(buf);
    m_resultItem.attachedImageName = openFileDialog.GetFilename().ToStdString();

    UpdateImagePreview();
}

void EntryDialog::OnRemoveImage(wxCommandEvent&) {
    m_resultItem.attachedImageBase64.clear();
    m_resultItem.attachedImageName.clear();
    wxImage placeholderImg(90, 60, true);
    placeholderImg.SetRGB(wxRect(0, 0, 90, 60), 220, 220, 220);
    if (m_imageThumbBmp) {
        m_imageThumbBmp->SetBitmap(wxBitmap(placeholderImg));
    }
    UpdateImagePreview();
}
