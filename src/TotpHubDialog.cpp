#include "TotpHubDialog.h"
#include "TotpEngine.h"
<<<<<<< HEAD
#include "ThemeManager.h"
#include "ClipboardSanitizer.h"
#include "EntryDialog.h"
=======
#include <wx/clipbrd.h>
>>>>>>> origin

const int TIMER_ID_HUB = 3001;

wxBEGIN_EVENT_TABLE(TotpHubDialog, wxDialog)
    EVT_TIMER(TIMER_ID_HUB, TotpHubDialog::OnTimer)
wxEND_EVENT_TABLE()

TotpHubDialog::TotpHubDialog(wxWindow* parent, VaultStorage& storage, const std::string& currentPin)
    : wxDialog(parent, wxID_ANY, "Password Locker - Authenticator", wxDefaultPosition, wxSize(560, 620), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
      m_storage(storage),
      m_pin(currentPin),
      m_timer(this, TIMER_ID_HUB),
      m_scrollWin(nullptr),
      m_cardsSizer(nullptr) {

<<<<<<< HEAD
    SetBackgroundColour(ThemeManager::Colors.windowBg);
=======
    SetBackgroundColour(wxColour(248, 250, 252));
>>>>>>> origin
    SetupUI();
    PopulateCards();
    m_timer.Start(1000); // 1 sec tick
}

TotpHubDialog::~TotpHubDialog() {
    if (m_timer.IsRunning()) {
        m_timer.Stop();
    }
}

void TotpHubDialog::SetupUI() {
<<<<<<< HEAD
    const auto& th = ThemeManager::Colors;
=======
>>>>>>> origin
    wxBoxSizer* rootSizer = new wxBoxSizer(wxVERTICAL);

    // Header Panel
    wxPanel* headerPanel = new wxPanel(this, wxID_ANY);
<<<<<<< HEAD
    headerPanel->SetBackgroundColour(th.headerBg);
=======
    headerPanel->SetBackgroundColour(wxColour(15, 23, 42)); // Slate 900
>>>>>>> origin
    wxBoxSizer* headerSizer = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer* topRow = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* title = new wxStaticText(headerPanel, wxID_ANY, "Password Locker Authenticator");
<<<<<<< HEAD
    title->SetForegroundColour(th.headerText);
=======
    title->SetForegroundColour(*wxWHITE);
>>>>>>> origin
    wxFont tf = title->GetFont();
    tf.SetPointSize(13);
    tf.SetWeight(wxFONTWEIGHT_BOLD);
    title->SetFont(tf);

    wxButton* addBtn = new wxButton(headerPanel, wxID_ANY, "+ Add 2FA Token", wxDefaultPosition, wxSize(130, 30));
    addBtn->Bind(wxEVT_BUTTON, &TotpHubDialog::OnAddNewTotp, this);

    topRow->Add(title, 1, wxALIGN_CENTER_VERTICAL);
    topRow->Add(addBtn, 0);
    headerSizer->Add(topRow, 0, wxEXPAND | wxALL, 12);

<<<<<<< HEAD
    wxStaticText* sub = new wxStaticText(headerPanel, wxID_ANY, "Standard RFC 6238 TOTP codes generated 100% offline. Refreshes every 30s. Auto-clears copied tokens in 2m.");
    sub->SetForegroundColour((ThemeManager::CurrentTheme == ThemeMode::HighContrast) ? th.textSecondary : wxColour(148, 163, 184));
=======
    wxStaticText* sub = new wxStaticText(headerPanel, wxID_ANY, "Standard RFC 6238 TOTP codes generated 100% offline. Refreshes every 30s.");
    sub->SetForegroundColour(wxColour(148, 163, 184));
>>>>>>> origin
    headerSizer->Add(sub, 0, wxLEFT | wxRIGHT | wxBOTTOM, 12);

    headerPanel->SetSizer(headerSizer);
    rootSizer->Add(headerPanel, 0, wxEXPAND);

    // Scrollable Cards
    m_scrollWin = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL);
    m_scrollWin->SetScrollRate(0, 10);
<<<<<<< HEAD
    m_scrollWin->SetBackgroundColour(th.windowBg);
=======
>>>>>>> origin
    m_cardsSizer = new wxBoxSizer(wxVERTICAL);
    m_scrollWin->SetSizer(m_cardsSizer);
    rootSizer->Add(m_scrollWin, 1, wxEXPAND | wxALL, 10);

    // Bottom Close Button
    wxPanel* botPanel = new wxPanel(this, wxID_ANY);
<<<<<<< HEAD
    botPanel->SetBackgroundColour(th.headerBg);
=======
    botPanel->SetBackgroundColour(wxColour(241, 245, 249));
>>>>>>> origin
    wxBoxSizer* botSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* closeBtn = new wxButton(botPanel, wxID_CANCEL, "Close", wxDefaultPosition, wxSize(100, 32));
    closeBtn->SetDefault();
    botSizer->AddStretchSpacer();
    botSizer->Add(closeBtn, 0, wxALL, 8);
    botPanel->SetSizer(botSizer);
    rootSizer->Add(botPanel, 0, wxEXPAND);

    SetSizer(rootSizer);
<<<<<<< HEAD
    ThemeManager::ApplyThemeToWindow(this);
=======
>>>>>>> origin
    CenterOnParent();
}

void TotpHubDialog::PopulateCards() {
<<<<<<< HEAD
    const auto& th = ThemeManager::Colors;
=======
>>>>>>> origin
    m_cards.clear();
    m_cardsSizer->Clear(true);

    int count = 0;
    for (const auto& item : m_storage.GetAllItems()) {
        if (item.totpSecret.empty()) continue;

        count++;
        wxPanel* card = new wxPanel(m_scrollWin, wxID_ANY);
<<<<<<< HEAD
        card->SetBackgroundColour(th.cardBg);
=======
        card->SetBackgroundColour(*wxWHITE);
>>>>>>> origin
        wxBoxSizer* cardSizer = new wxBoxSizer(wxVERTICAL);

        // Account title + section
        wxBoxSizer* titleRow = new wxBoxSizer(wxHORIZONTAL);
        wxStaticText* nameLbl = new wxStaticText(card, wxID_ANY, wxString::FromUTF8(item.label.c_str()));
        wxFont nf = nameLbl->GetFont();
        nf.SetWeight(wxFONTWEIGHT_BOLD);
        nf.SetPointSize(11);
        nameLbl->SetFont(nf);
<<<<<<< HEAD
        nameLbl->SetForegroundColour(th.textPrimary);

        wxStaticText* secLbl = new wxStaticText(card, wxID_ANY, " " + wxString::FromUTF8(item.tabName.c_str()) + " ");
        secLbl->SetBackgroundColour(th.tagBg);
        secLbl->SetForegroundColour(th.tagText);

        titleRow->Add(nameLbl, 1, wxALIGN_CENTER_VERTICAL | wxALL, 8);
        titleRow->Add(secLbl, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);
        cardSizer->Add(titleRow, 0, wxEXPAND);

        // Code and Copy button
        wxBoxSizer* codeRow = new wxBoxSizer(wxHORIZONTAL);
        std::string currentCode = TotpEngine::GenerateCode(item.totpSecret);
        wxStaticText* codeLbl = new wxStaticText(card, wxID_ANY, TotpEngine::FormatCode(currentCode));
=======

        wxStaticText* tabBadge = new wxStaticText(card, wxID_ANY, " Section: " + wxString::FromUTF8(item.tabName.c_str()) + " ");
        tabBadge->SetForegroundColour(wxColour(100, 116, 139));

        titleRow->Add(nameLbl, 0, wxALIGN_CENTER_VERTICAL | wxALL, 8);
        titleRow->Add(tabBadge, 0, wxALIGN_CENTER_VERTICAL | wxALL, 8);
        cardSizer->Add(titleRow, 0, wxEXPAND);

        // Code row
        wxBoxSizer* codeRow = new wxBoxSizer(wxHORIZONTAL);
        wxStaticText* codeLbl = new wxStaticText(card, wxID_ANY, "--- ---");
>>>>>>> origin
        wxFont cf = codeLbl->GetFont();
        cf.SetPointSize(18);
        cf.SetWeight(wxFONTWEIGHT_BOLD);
        codeLbl->SetFont(cf);
<<<<<<< HEAD
        codeLbl->SetForegroundColour(th.accent);
=======
        codeLbl->SetForegroundColour(wxColour(30, 27, 75));
>>>>>>> origin

        wxButton* copyBtn = new wxButton(card, wxID_ANY, "Copy", wxDefaultPosition, wxSize(80, 30));
        std::string secret = item.totpSecret;
        std::string itmId = item.id;
        copyBtn->Bind(wxEVT_BUTTON, [this, secret, itmId, copyBtn](wxCommandEvent&) {
            std::string code = TotpEngine::GenerateCode(secret);
<<<<<<< HEAD
            ClipboardSanitizer::CopyAndScheduleClear(wxString::FromUTF8(code.c_str()), 120);
            copyBtn->SetLabel("Copied! (2m)");
=======
            if (wxTheClipboard->Open()) {
                wxTheClipboard->SetData(new wxTextDataObject(wxString::FromUTF8(code.c_str())));
                wxTheClipboard->Close();
            }
            copyBtn->SetLabel("Copied!");
>>>>>>> origin
            for (auto& w : m_cards) {
                if (w.itemId == itmId) {
                    w.copiedResetCountdown = 2;
                    break;
                }
            }
        });

        codeRow->Add(codeLbl, 1, wxALIGN_CENTER_VERTICAL | wxLEFT, 12);
        codeRow->Add(copyBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 12);
        cardSizer->Add(codeRow, 0, wxEXPAND | wxBOTTOM, 6);

        // Progress bar
        wxBoxSizer* prgRow = new wxBoxSizer(wxHORIZONTAL);
        wxGauge* gauge = new wxGauge(card, wxID_ANY, 30, wxDefaultPosition, wxSize(-1, 6));
        wxStaticText* cdLbl = new wxStaticText(card, wxID_ANY, "30s");
<<<<<<< HEAD
        cdLbl->SetForegroundColour(th.textMuted);
=======
        cdLbl->SetForegroundColour(wxColour(100, 116, 139));
>>>>>>> origin

        prgRow->Add(gauge, 1, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 12);
        prgRow->Add(cdLbl, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 12);
        cardSizer->Add(prgRow, 0, wxEXPAND | wxBOTTOM, 8);

        card->SetSizer(cardSizer);
<<<<<<< HEAD
        ThemeManager::ApplyThemeToWindow(card);
=======
>>>>>>> origin
        m_cardsSizer->Add(card, 0, wxEXPAND | wxBOTTOM, 8);

        TotpCardWidgets w;
        w.itemId = item.id;
        w.secret = item.totpSecret;
        w.codeLabel = codeLbl;
        w.countdownLabel = cdLbl;
        w.progressGauge = gauge;
        w.copyBtn = copyBtn;
        w.copiedResetCountdown = 0;
        m_cards.push_back(w);
    }

    if (count == 0) {
        wxStaticText* emptyLbl = new wxStaticText(m_scrollWin, wxID_ANY,
            "No accounts have 2-Factor Authentication keys configured.\n\nAdd or edit an account and provide a Base32 2FA secret key to view live tokens here.");
<<<<<<< HEAD
        emptyLbl->SetForegroundColour(th.textMuted);
=======
        emptyLbl->SetForegroundColour(wxColour(100, 116, 139));
>>>>>>> origin
        m_cardsSizer->Add(emptyLbl, 0, wxALL, 20);
    }

    m_scrollWin->FitInside();
    m_scrollWin->Layout();
<<<<<<< HEAD
}

void TotpHubDialog::OnTimer(wxTimerEvent&) {
    int rem = TotpEngine::GetRemainingSeconds();
    const auto& th = ThemeManager::Colors;

=======
    UpdateAllCodes();
}

void TotpHubDialog::UpdateAllCodes() {
    int rem = TotpEngine::GetRemainingSeconds();
>>>>>>> origin
    for (auto& w : m_cards) {
        std::string code = TotpEngine::GenerateCode(w.secret);
        w.codeLabel->SetLabel(TotpEngine::FormatCode(code));
        w.progressGauge->SetValue(rem);
        w.countdownLabel->SetLabel(wxString::Format("%ds", rem));
<<<<<<< HEAD

        if (rem <= 5) {
            w.countdownLabel->SetForegroundColour(th.danger);
        } else if (rem <= 15) {
            w.countdownLabel->SetForegroundColour(th.warning);
        } else {
            w.countdownLabel->SetForegroundColour(th.textMuted);
        }

        if (w.copiedResetCountdown > 0) {
            w.copiedResetCountdown--;
            if (w.copiedResetCountdown == 0) {
=======
        if (rem <= 5) {
            w.countdownLabel->SetForegroundColour(wxColour(220, 38, 38));
        } else if (rem <= 15) {
            w.countdownLabel->SetForegroundColour(wxColour(217, 119, 6));
        } else {
            w.countdownLabel->SetForegroundColour(wxColour(79, 70, 229));
        }
        if (w.copiedResetCountdown > 0) {
            w.copiedResetCountdown--;
            if (w.copiedResetCountdown == 0 && w.copyBtn) {
>>>>>>> origin
                w.copyBtn->SetLabel("Copy");
            }
        }
    }
}

<<<<<<< HEAD
void TotpHubDialog::OnAddNewTotp(wxCommandEvent&) {
    EntryDialog dlg(this, m_storage.GetCustomTabs(), "General");
    if (dlg.ShowModal() == wxID_OK) {
        VaultItem newItem = dlg.GetVaultItem();
        m_storage.AddItem(newItem);
        m_storage.Save(m_pin);
        PopulateCards();
    }
=======
void TotpHubDialog::OnTimer(wxTimerEvent& evt) {
    UpdateAllCodes();
}

void TotpHubDialog::OnAddNewTotp(wxCommandEvent& evt) {
    wxTextEntryDialog nameDlg(this, "Enter Account Name (e.g. GitHub):", "Add 2FA Token");
    if (nameDlg.ShowModal() != wxID_OK) return;
    wxString name = nameDlg.GetValue().Trim();
    if (name.empty()) return;

    wxTextEntryDialog keyDlg(this, "Enter Base32 2FA Secret Key (or leave blank to auto-generate):", "Secret Key");
    if (keyDlg.ShowModal() != wxID_OK) return;
    wxString secret = keyDlg.GetValue().Trim();
    if (secret.empty()) {
        secret = TotpEngine::GenerateRandomSecret(16);
    }

    VaultItem item = VaultItem::CreateNew(AccountType::Basic, name.ToStdString(), "General");
    item.totpSecret = secret.ToStdString();
    m_storage.AddItem(item);
    m_storage.Save(m_pin);

    PopulateCards();
>>>>>>> origin
}
