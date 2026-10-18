#include "DetailDialog.h"
#include "ImageViewerDialog.h"
#include "TotpEngine.h"
#include "VaultCrypto.h"
#include "ThemeManager.h"
#include "ClipboardSanitizer.h"
#include <wx/mstream.h>

const int TIMER_ID_TOTP = 2001;

wxBEGIN_EVENT_TABLE(DetailDialog, wxDialog)
    EVT_TIMER(TIMER_ID_TOTP, DetailDialog::OnTimer)
wxEND_EVENT_TABLE()

DetailDialog::DetailDialog(wxWindow* parent, const VaultItem& item)
    : wxDialog(parent, wxID_ANY, "", wxDefaultPosition, wxSize(660, 700), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
      m_item(item),
      m_action(DetailAction_Close),
      m_itemModified(false),
      m_imageCardPanel(nullptr),
      m_totpTimer(this, TIMER_ID_TOTP),
      m_totpCodeLabel(nullptr),
      m_totpCountdownLabel(nullptr),
      m_totpProgressGauge(nullptr),
      m_copyTotpBtn(nullptr),
      m_passwordCtrl(nullptr),
      m_showHidePassBtn(nullptr),
      m_passwordVisible(false),
      m_copiedResetCountdown(0) {

    SetTitle("Password Locker - " + wxString::FromUTF8(item.label.c_str()));
    SetBackgroundColour(ThemeManager::Colors.windowBg);

    SetupUI();

    if (!m_item.totpSecret.empty()) {
        UpdateTotpDisplay();
        m_totpTimer.Start(1000); // Tick every 1 second
    }
}

DetailDialog::~DetailDialog() {
    if (m_totpTimer.IsRunning()) {
        m_totpTimer.Stop();
    }
}

void DetailDialog::SetupUI() {
    const auto& th = ThemeManager::Colors;
    wxBoxSizer* rootSizer = new wxBoxSizer(wxVERTICAL);

    // Header Panel
    wxPanel* headerPanel = new wxPanel(this, wxID_ANY);
    headerPanel->SetBackgroundColour(th.headerBg);
    wxBoxSizer* headerSizer = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer* topHeaderRow = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* titleLbl = new wxStaticText(headerPanel, wxID_ANY, wxString::FromUTF8(m_item.label.c_str()));
    titleLbl->SetForegroundColour(th.headerText);
    wxFont tf = titleLbl->GetFont();
    tf.SetPointSize(14);
    tf.SetWeight(wxFONTWEIGHT_BOLD);
    titleLbl->SetFont(tf);

    wxString typeBadgeStr = wxString::FromUTF8(AccountTypeToString(m_item.type).c_str());
    wxStaticText* badge = new wxStaticText(headerPanel, wxID_ANY, " " + typeBadgeStr + " ");
    badge->SetForegroundColour(th.accent);
    wxFont bf = badge->GetFont();
    bf.SetWeight(wxFONTWEIGHT_BOLD);
    badge->SetFont(bf);

    topHeaderRow->Add(titleLbl, 1, wxALIGN_CENTER_VERTICAL);
    topHeaderRow->Add(badge, 0, wxALIGN_CENTER_VERTICAL);
    headerSizer->Add(topHeaderRow, 0, wxEXPAND | wxALL, 12);

    // Bold Submission Timestamp Line
    wxBoxSizer* timeRow = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* timePrompt = new wxStaticText(headerPanel, wxID_ANY, "Submitted: ");
    timePrompt->SetForegroundColour(th.textMuted);

    wxStaticText* boldTimeTxt = new wxStaticText(headerPanel, wxID_ANY, wxString::FromUTF8(m_item.submittedTimeFormatted.c_str()));
    boldTimeTxt->SetForegroundColour(th.warning); // Amber / Yellow bold
    wxFont timeFont = boldTimeTxt->GetFont();
    timeFont.SetPointSize(10);
    timeFont.SetWeight(wxFONTWEIGHT_BOLD);
    boldTimeTxt->SetFont(timeFont);

    timeRow->Add(timePrompt, 0, wxALIGN_CENTER_VERTICAL);
    timeRow->Add(boldTimeTxt, 0, wxALIGN_CENTER_VERTICAL);

    wxStaticText* tabTag = new wxStaticText(headerPanel, wxID_ANY, "   Section: " + wxString::FromUTF8(m_item.tabName.c_str()));
    tabTag->SetForegroundColour(th.textSecondary);
    timeRow->Add(tabTag, 0, wxALIGN_CENTER_VERTICAL);

    headerSizer->Add(timeRow, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 12);
    headerPanel->SetSizer(headerSizer);
    rootSizer->Add(headerPanel, 0, wxEXPAND);

    // Scrollable Content
    wxScrolledWindow* bodyScroll = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL);
    bodyScroll->SetScrollRate(0, 10);
    wxBoxSizer* bodySizer = new wxBoxSizer(wxVERTICAL);

    // 2FA Authenticator Live Card (if configured)
    if (!m_item.totpSecret.empty()) {
        wxPanel* totpCard = new wxPanel(bodyScroll, wxID_ANY);
        totpCard->SetBackgroundColour(th.cardBg);
        wxBoxSizer* cardSizer = new wxBoxSizer(wxVERTICAL);

        wxStaticText* totpTitle = new wxStaticText(totpCard, wxID_ANY, "Two-Factor Authenticator (2FA)");
        wxFont ttf = totpTitle->GetFont();
        ttf.SetWeight(wxFONTWEIGHT_BOLD);
        totpTitle->SetFont(ttf);
        totpTitle->SetForegroundColour(th.accent);
        cardSizer->Add(totpTitle, 0, wxALL, 8);

        wxBoxSizer* codeRow = new wxBoxSizer(wxHORIZONTAL);
        m_totpCodeLabel = new wxStaticText(totpCard, wxID_ANY, "--- ---");
        wxFont codeFont = m_totpCodeLabel->GetFont();
        codeFont.SetPointSize(20);
        codeFont.SetWeight(wxFONTWEIGHT_BOLD);
        m_totpCodeLabel->SetFont(codeFont);
        m_totpCodeLabel->SetForegroundColour(th.textPrimary);

        m_copyTotpBtn = new wxButton(totpCard, wxID_ANY, "Copy 2FA Code", wxDefaultPosition, wxSize(130, 34));
        m_copyTotpBtn->Bind(wxEVT_BUTTON, &DetailDialog::OnCopyTotp, this);

        codeRow->Add(m_totpCodeLabel, 1, wxALIGN_CENTER_VERTICAL | wxLEFT, 8);
        codeRow->Add(m_copyTotpBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);
        cardSizer->Add(codeRow, 0, wxEXPAND | wxBOTTOM, 6);

        // Progress countdown
        wxBoxSizer* progressRow = new wxBoxSizer(wxHORIZONTAL);
        m_totpProgressGauge = new wxGauge(totpCard, wxID_ANY, 30, wxDefaultPosition, wxSize(-1, 8));
        m_totpProgressGauge->SetValue(30);

        m_totpCountdownLabel = new wxStaticText(totpCard, wxID_ANY, "30s remaining");
        m_totpCountdownLabel->SetForegroundColour(th.accent);

        progressRow->Add(m_totpProgressGauge, 1, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 8);
        progressRow->Add(m_totpCountdownLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);
        cardSizer->Add(progressRow, 0, wxEXPAND | wxBOTTOM, 8);

        totpCard->SetSizer(cardSizer);
        bodySizer->Add(totpCard, 0, wxEXPAND | wxALL, 10);
    }

    // Credentials Card
    if (m_item.type == AccountType::Basic || m_item.type == AccountType::SecurityQuestions) {
        wxPanel* credPanel = new wxPanel(bodyScroll, wxID_ANY);
        credPanel->SetBackgroundColour(th.cardBg);
        wxBoxSizer* credSizer = new wxBoxSizer(wxVERTICAL);

        // Username
        if (!m_item.username.empty()) {
            wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);
            wxStaticText* lbl = new wxStaticText(credPanel, wxID_ANY, "Username:", wxDefaultPosition, wxSize(90, -1));
            lbl->SetForegroundColour(th.textMuted);
            wxTextCtrl* val = new wxTextCtrl(credPanel, wxID_ANY, wxString::FromUTF8(m_item.username.c_str()), wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
            wxButton* copyBtn = new wxButton(credPanel, wxID_ANY, "Copy", wxDefaultPosition, wxSize(60, 26));
            copyBtn->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
                CopyToClipboard(wxString::FromUTF8(m_item.username.c_str()), "Username");
            });

            row->Add(lbl, 0, wxALIGN_CENTER_VERTICAL | wxALL, 6);
            row->Add(val, 1, wxEXPAND | wxALL, 6);
            row->Add(copyBtn, 0, wxALIGN_CENTER_VERTICAL | wxALL, 6);
            credSizer->Add(row, 0, wxEXPAND);
        }

        // Email
        if (!m_item.email.empty()) {
            wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);
            wxStaticText* lbl = new wxStaticText(credPanel, wxID_ANY, "Email:", wxDefaultPosition, wxSize(90, -1));
            lbl->SetForegroundColour(th.textMuted);
            wxTextCtrl* val = new wxTextCtrl(credPanel, wxID_ANY, wxString::FromUTF8(m_item.email.c_str()), wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
            wxButton* copyBtn = new wxButton(credPanel, wxID_ANY, "Copy", wxDefaultPosition, wxSize(60, 26));
            copyBtn->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
                CopyToClipboard(wxString::FromUTF8(m_item.email.c_str()), "Email");
            });

            row->Add(lbl, 0, wxALIGN_CENTER_VERTICAL | wxALL, 6);
            row->Add(val, 1, wxEXPAND | wxALL, 6);
            row->Add(copyBtn, 0, wxALIGN_CENTER_VERTICAL | wxALL, 6);
            credSizer->Add(row, 0, wxEXPAND);
        }

        // Password
        if (!m_item.password.empty()) {
            wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);
            wxStaticText* lbl = new wxStaticText(credPanel, wxID_ANY, "Password:", wxDefaultPosition, wxSize(90, -1));
            lbl->SetForegroundColour(th.textMuted);

            m_passwordCtrl = new wxTextCtrl(credPanel, wxID_ANY, wxString::FromUTF8(m_item.password.c_str()), wxDefaultPosition, wxDefaultSize, wxTE_READONLY | wxTE_PASSWORD);
            m_showHidePassBtn = new wxButton(credPanel, wxID_ANY, "Show", wxDefaultPosition, wxSize(60, 26));
            m_showHidePassBtn->Bind(wxEVT_BUTTON, &DetailDialog::OnTogglePassword, this);

            wxButton* copyBtn = new wxButton(credPanel, wxID_ANY, "Copy", wxDefaultPosition, wxSize(60, 26));
            copyBtn->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
                CopyToClipboard(wxString::FromUTF8(m_item.password.c_str()), "Password");
            });

            row->Add(lbl, 0, wxALIGN_CENTER_VERTICAL | wxALL, 6);
            row->Add(m_passwordCtrl, 1, wxEXPAND | wxALL, 6);
            row->Add(m_showHidePassBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);
            row->Add(copyBtn, 0, wxALIGN_CENTER_VERTICAL | wxALL, 6);
            credSizer->Add(row, 0, wxEXPAND);
        }

        // Website
        if (!m_item.website.empty()) {
            wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);
            wxStaticText* lbl = new wxStaticText(credPanel, wxID_ANY, "Website:", wxDefaultPosition, wxSize(90, -1));
            lbl->SetForegroundColour(th.textMuted);
            wxTextCtrl* val = new wxTextCtrl(credPanel, wxID_ANY, wxString::FromUTF8(m_item.website.c_str()), wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
            wxButton* copyBtn = new wxButton(credPanel, wxID_ANY, "Copy", wxDefaultPosition, wxSize(60, 26));
            copyBtn->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
                CopyToClipboard(wxString::FromUTF8(m_item.website.c_str()), "Website URL");
            });

            row->Add(lbl, 0, wxALIGN_CENTER_VERTICAL | wxALL, 6);
            row->Add(val, 1, wxEXPAND | wxALL, 6);
            row->Add(copyBtn, 0, wxALIGN_CENTER_VERTICAL | wxALL, 6);
            credSizer->Add(row, 0, wxEXPAND);
        }

        credPanel->SetSizer(credSizer);
        bodySizer->Add(credPanel, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);
    }

    // Security Questions Section
    if (!m_item.securityQuestions.empty()) {
        wxPanel* sqPanel = new wxPanel(bodyScroll, wxID_ANY);
        sqPanel->SetBackgroundColour(th.cardBg);
        wxBoxSizer* sqSizer = new wxBoxSizer(wxVERTICAL);

        wxStaticText* sqTitle = new wxStaticText(sqPanel, wxID_ANY, "Security Questions & Answers");
        wxFont sf = sqTitle->GetFont();
        sf.SetWeight(wxFONTWEIGHT_BOLD);
        sqTitle->SetFont(sf);
        sqTitle->SetForegroundColour(th.textPrimary);
        sqSizer->Add(sqTitle, 0, wxALL, 8);

        for (size_t i = 0; i < m_item.securityQuestions.size(); ++i) {
            const auto& sq = m_item.securityQuestions[i];
            wxBoxSizer* row = new wxBoxSizer(wxVERTICAL);

            wxStaticText* qLbl = new wxStaticText(sqPanel, wxID_ANY, wxString::Format("Q%d: %s", static_cast<int>(i + 1), wxString::FromUTF8(sq.question.c_str())));
            wxFont qf = qLbl->GetFont();
            qf.SetWeight(wxFONTWEIGHT_BOLD);
            qLbl->SetFont(qf);
            qLbl->SetForegroundColour(th.textPrimary);

            wxBoxSizer* ansRow = new wxBoxSizer(wxHORIZONTAL);
            wxTextCtrl* aCtrl = new wxTextCtrl(sqPanel, wxID_ANY, wxString::FromUTF8(sq.answer.c_str()), wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
            wxButton* copyBtn = new wxButton(sqPanel, wxID_ANY, "Copy", wxDefaultPosition, wxSize(60, 26));
            copyBtn->Bind(wxEVT_BUTTON, [this, sq](wxCommandEvent&) {
                CopyToClipboard(wxString::FromUTF8(sq.answer.c_str()), "Security Answer");
            });

            ansRow->Add(aCtrl, 1, wxEXPAND | wxRIGHT, 6);
            ansRow->Add(copyBtn, 0);

            row->Add(qLbl, 0, wxLEFT | wxRIGHT | wxTOP, 6);
            row->Add(ansRow, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 6);
            sqSizer->Add(row, 0, wxEXPAND);
        }

        sqPanel->SetSizer(sqSizer);
        bodySizer->Add(sqPanel, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);
    }

    // Miscellaneous Custom Fields Section
    if (!m_item.customFields.empty()) {
        wxPanel* cfPanel = new wxPanel(bodyScroll, wxID_ANY);
        cfPanel->SetBackgroundColour(th.cardBg);
        wxBoxSizer* cfSizer = new wxBoxSizer(wxVERTICAL);

        wxStaticText* cfTitle = new wxStaticText(cfPanel, wxID_ANY, "User-Defined Custom Fields");
        wxFont cfF = cfTitle->GetFont();
        cfF.SetWeight(wxFONTWEIGHT_BOLD);
        cfTitle->SetFont(cfF);
        cfTitle->SetForegroundColour(th.textPrimary);
        cfSizer->Add(cfTitle, 0, wxALL, 8);

        for (const auto& cf : m_item.customFields) {
            wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);
            wxStaticText* lbl = new wxStaticText(cfPanel, wxID_ANY, wxString::FromUTF8(cf.label.c_str()) + ":", wxDefaultPosition, wxSize(130, -1));
            wxFont lf = lbl->GetFont();
            lf.SetWeight(wxFONTWEIGHT_BOLD);
            lbl->SetFont(lf);
            lbl->SetForegroundColour(th.textPrimary);

            wxTextCtrl* val = new wxTextCtrl(cfPanel, wxID_ANY, wxString::FromUTF8(cf.value.c_str()), wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
            wxButton* copyBtn = new wxButton(cfPanel, wxID_ANY, "Copy", wxDefaultPosition, wxSize(60, 26));
            copyBtn->Bind(wxEVT_BUTTON, [this, cf](wxCommandEvent&) {
                CopyToClipboard(wxString::FromUTF8(cf.value.c_str()), wxString::FromUTF8(cf.label.c_str()));
            });

            row->Add(lbl, 0, wxALIGN_CENTER_VERTICAL | wxALL, 6);
            row->Add(val, 1, wxEXPAND | wxALL, 6);
            row->Add(copyBtn, 0, wxALIGN_CENTER_VERTICAL | wxALL, 6);
            cfSizer->Add(row, 0, wxEXPAND);
        }

        cfPanel->SetSizer(cfSizer);
        bodySizer->Add(cfPanel, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);
    }

    // Notes Section
    if (!m_item.notes.empty()) {
        wxPanel* notesPanel = new wxPanel(bodyScroll, wxID_ANY);
        notesPanel->SetBackgroundColour(th.cardBg);
        wxBoxSizer* notesSizer = new wxBoxSizer(wxVERTICAL);

        wxStaticText* nTitle = new wxStaticText(notesPanel, wxID_ANY, "Notes / Documentation");
        wxFont nf = nTitle->GetFont();
        nf.SetWeight(wxFONTWEIGHT_BOLD);
        nTitle->SetFont(nf);
        nTitle->SetForegroundColour(th.textPrimary);
        notesSizer->Add(nTitle, 0, wxALL, 8);

        wxTextCtrl* nText = new wxTextCtrl(notesPanel, wxID_ANY, wxString::FromUTF8(m_item.notes.c_str()), wxDefaultPosition, wxSize(-1, 80), wxTE_MULTILINE | wxTE_READONLY);
        notesSizer->Add(nText, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);

        notesPanel->SetSizer(notesSizer);
        bodySizer->Add(notesPanel, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);
    }

    // Attached Image / Document Section
    if (!m_item.attachedImageBase64.empty()) {
        m_imageCardPanel = new wxPanel(bodyScroll, wxID_ANY);
        m_imageCardPanel->SetBackgroundColour(th.cardBg);
        wxBoxSizer* imgCardSizer = new wxBoxSizer(wxVERTICAL);

        wxStaticText* docTitle = new wxStaticText(m_imageCardPanel, wxID_ANY, "Attached Image / Document");
        wxFont df = docTitle->GetFont();
        df.SetWeight(wxFONTWEIGHT_BOLD);
        docTitle->SetFont(df);
        docTitle->SetForegroundColour(th.textPrimary);
        imgCardSizer->Add(docTitle, 0, wxALL, 8);

        wxBoxSizer* imgRow = new wxBoxSizer(wxHORIZONTAL);

        std::vector<uint8_t> raw = VaultCrypto::Base64Decode(m_item.attachedImageBase64);
        wxBitmap thumbBmp;
        if (!raw.empty()) {
            wxMemoryInputStream memStream(raw.data(), raw.size());
            wxImage img(memStream);
            if (img.IsOk()) {
                wxImage thumb = img.Scale(120, 80, wxIMAGE_QUALITY_HIGH);
                thumbBmp = wxBitmap(thumb);
            }
        }

        wxStaticBitmap* thumbCtrl = new wxStaticBitmap(m_imageCardPanel, wxID_ANY, thumbBmp, wxDefaultPosition, wxSize(120, 80));
        thumbCtrl->SetCursor(wxCURSOR_HAND);
        thumbCtrl->SetToolTip("Click to view Full Screen (fits to page, Esc to exit)");

        thumbCtrl->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent&) {
            wxCommandEvent dummy;
            OnViewImageFullscreen(dummy);
        });

        wxBoxSizer* infoSizer = new wxBoxSizer(wxVERTICAL);
        wxString docName = m_item.attachedImageName.empty() ? "Attached Document" : wxString::FromUTF8(m_item.attachedImageName.c_str());
        wxStaticText* nameTxt = new wxStaticText(m_imageCardPanel, wxID_ANY, docName);
        wxFont nf2 = nameTxt->GetFont();
        nf2.SetWeight(wxFONTWEIGHT_BOLD);
        nameTxt->SetFont(nf2);
        nameTxt->SetForegroundColour(th.textPrimary);

        wxStaticText* hintTxt = new wxStaticText(m_imageCardPanel, wxID_ANY, "Click preview to view Full Screen.\nAuto-fits to screen. Press Esc to exit.");
        hintTxt->SetForegroundColour(th.textMuted);

        wxButton* viewFullBtn = new wxButton(m_imageCardPanel, wxID_ANY, "View Full Screen", wxDefaultPosition, wxSize(140, 30));
        viewFullBtn->Bind(wxEVT_BUTTON, &DetailDialog::OnViewImageFullscreen, this);

        infoSizer->Add(nameTxt, 0, wxBOTTOM, 4);
        infoSizer->Add(hintTxt, 0, wxBOTTOM, 8);
        infoSizer->Add(viewFullBtn, 0);

        imgRow->Add(thumbCtrl, 0, wxALIGN_CENTER_VERTICAL | wxALL, 8);
        imgRow->Add(infoSizer, 1, wxALIGN_CENTER_VERTICAL | wxALL, 8);
        imgCardSizer->Add(imgRow, 0, wxEXPAND);

        m_imageCardPanel->SetSizer(imgCardSizer);
        bodySizer->Add(m_imageCardPanel, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);
    }

    bodyScroll->SetSizer(bodySizer);
    rootSizer->Add(bodyScroll, 1, wxEXPAND);

    // Bottom Action Buttons (Edit, Delete, Close)
    wxPanel* bottomPanel = new wxPanel(this, wxID_ANY);
    bottomPanel->SetBackgroundColour(th.headerBg);
    wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);

    wxButton* deleteBtn = new wxButton(bottomPanel, wxID_ANY, "Delete Account", wxDefaultPosition, wxSize(120, 32));
    deleteBtn->SetForegroundColour(th.danger);
    deleteBtn->Bind(wxEVT_BUTTON, &DetailDialog::OnDelete, this);

    wxButton* editBtn = new wxButton(bottomPanel, wxID_ANY, "Edit Details", wxDefaultPosition, wxSize(110, 32));
    editBtn->Bind(wxEVT_BUTTON, &DetailDialog::OnEdit, this);

    wxButton* closeBtn = new wxButton(bottomPanel, wxID_CANCEL, "Close", wxDefaultPosition, wxSize(90, 32));
    closeBtn->SetDefault();
    closeBtn->Bind(wxEVT_BUTTON, &DetailDialog::OnCloseBtn, this);

    bottomSizer->Add(deleteBtn, 0, wxALL, 8);
    bottomSizer->AddStretchSpacer();
    bottomSizer->Add(editBtn, 0, wxALL, 8);
    bottomSizer->Add(closeBtn, 0, wxALL, 8);
    bottomPanel->SetSizer(bottomSizer);

    rootSizer->Add(bottomPanel, 0, wxEXPAND);

    SetSizer(rootSizer);
    ThemeManager::ApplyThemeToWindow(this);
    CenterOnParent();
}

void DetailDialog::OnViewImageFullscreen(wxCommandEvent&) {
    if (m_item.attachedImageBase64.empty()) return;

    std::vector<uint8_t> raw = VaultCrypto::Base64Decode(m_item.attachedImageBase64);
    if (raw.empty()) return;

    wxMemoryInputStream memStream(raw.data(), raw.size());
    wxImage img(memStream);
    if (!img.IsOk()) {
        wxMessageBox("Failed to decode image data.", "Error", wxOK | wxICON_ERROR, this);
        return;
    }

    ImageViewerDialog viewer(this, img, m_item.attachedImageName, true);
    viewer.ShowModal();

    if (viewer.WasImageRemoved()) {
        m_item.attachedImageBase64.clear();
        m_item.attachedImageName.clear();
        m_itemModified = true;
        if (m_imageCardPanel) {
            m_imageCardPanel->Show(false);
            Layout();
        }
        wxMessageBox("Attached image removed from account.", "Image Removed", wxOK | wxICON_INFORMATION, this);
    }
}

void DetailDialog::UpdateTotpDisplay() {
    if (m_item.totpSecret.empty() || !m_totpCodeLabel) return;

    std::string code = TotpEngine::GenerateCode(m_item.totpSecret);
    int remaining = TotpEngine::GetRemainingSeconds();

    m_totpCodeLabel->SetLabel(TotpEngine::FormatCode(code));
    if (m_totpProgressGauge) {
        m_totpProgressGauge->SetValue(remaining);
    }
    if (m_totpCountdownLabel) {
        m_totpCountdownLabel->SetLabel(wxString::Format("%ds remaining", remaining));
        const auto& th = ThemeManager::Colors;
        if (remaining <= 5) {
            m_totpCountdownLabel->SetForegroundColour(th.danger);
        } else if (remaining <= 15) {
            m_totpCountdownLabel->SetForegroundColour(th.warning);
        } else {
            m_totpCountdownLabel->SetForegroundColour(th.accent);
        }
    }
}

void DetailDialog::OnTimer(wxTimerEvent&) {
    UpdateTotpDisplay();
    if (m_copiedResetCountdown > 0) {
        m_copiedResetCountdown--;
        if (m_copiedResetCountdown == 0 && m_copyTotpBtn) {
            m_copyTotpBtn->SetLabel("Copy 2FA Code");
        }
    }
}

void DetailDialog::OnCopyTotp(wxCommandEvent&) {
    if (m_item.totpSecret.empty()) return;
    std::string code = TotpEngine::GenerateCode(m_item.totpSecret);
    CopyToClipboard(wxString::FromUTF8(code.c_str()), "2FA Code");
    if (m_copyTotpBtn) {
        m_copyTotpBtn->SetLabel("Copied! (2m timer)");
        m_copiedResetCountdown = 2; // Reset after 2 seconds
    }
}

void DetailDialog::OnTogglePassword(wxCommandEvent&) {
    if (!m_passwordCtrl) return;
    m_passwordVisible = !m_passwordVisible;

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

    if (m_passwordVisible) {
        m_passwordCtrl = new wxTextCtrl(parent, wxID_ANY, wxString::FromUTF8(m_item.password.c_str()), wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
        m_showHidePassBtn->SetLabel("Hide");
    } else {
        m_passwordCtrl = new wxTextCtrl(parent, wxID_ANY, wxString::FromUTF8(m_item.password.c_str()), wxDefaultPosition, wxDefaultSize, wxTE_READONLY | wxTE_PASSWORD);
        m_showHidePassBtn->SetLabel("Show");
    }

    sizer->Insert(idx, m_passwordCtrl, 1, wxEXPAND | wxALL, 6);
    parent->Layout();
}

void DetailDialog::CopyToClipboard(const wxString& text, const wxString&) {
    // Copies and schedules automatic sanitization / clear after 2 minutes
    ClipboardSanitizer::CopyAndScheduleClear(text, 120);
}

void DetailDialog::OnEdit(wxCommandEvent&) {
    m_action = DetailAction_Edit;
    EndModal(wxID_OK);
}

void DetailDialog::OnDelete(wxCommandEvent&) {
    int res = wxMessageBox(
        "Are you sure you want to permanently delete '" + wxString::FromUTF8(m_item.label.c_str()) + "'?",
        "Confirm Delete",
        wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION,
        this
    );
    if (res == wxYES) {
        m_action = DetailAction_Delete;
        EndModal(wxID_OK);
    }
}

void DetailDialog::OnCloseBtn(wxCommandEvent&) {
    m_action = DetailAction_Close;
    EndModal(wxID_CANCEL);
}
