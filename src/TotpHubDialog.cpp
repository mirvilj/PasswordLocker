#include "TotpHubDialog.h"
#include "TotpEngine.h"
#include "ThemeManager.h"
#include "ClipboardSanitizer.h"
#include "EntryDialog.h"

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

    SetBackgroundColour(ThemeManager::Colors.windowBg);
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
    const auto& th = ThemeManager::Colors;
    wxBoxSizer* rootSizer = new wxBoxSizer(wxVERTICAL);

    // Header Panel
    wxPanel* headerPanel = new wxPanel(this, wxID_ANY);
    headerPanel->SetBackgroundColour(th.headerBg);
    wxBoxSizer* headerSizer = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer* topRow = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* title = new wxStaticText(headerPanel, wxID_ANY, "Password Locker Authenticator");
    title->SetForegroundColour(th.headerText);
    wxFont tf = title->GetFont();
    tf.SetPointSize(13);
    tf.SetWeight(wxFONTWEIGHT_BOLD);
    title->SetFont(tf);

    wxButton* addBtn = new wxButton(headerPanel, wxID_ANY, "+ Add 2FA Token", wxDefaultPosition, wxSize(130, 30));
    addBtn->Bind(wxEVT_BUTTON, &TotpHubDialog::OnAddNewTotp, this);

    topRow->Add(title, 1, wxALIGN_CENTER_VERTICAL);
    topRow->Add(addBtn, 0);
    headerSizer->Add(topRow, 0, wxEXPAND | wxALL, 12);

    wxStaticText* sub = new wxStaticText(headerPanel, wxID_ANY, "Standard RFC 6238 TOTP codes generated 100% offline. Refreshes every 30s. Auto-clears copied tokens in 2m.");
    sub->SetForegroundColour((ThemeManager::CurrentTheme == ThemeMode::HighContrast) ? th.textSecondary : wxColour(148, 163, 184));
    headerSizer->Add(sub, 0, wxLEFT | wxRIGHT | wxBOTTOM, 12);

    headerPanel->SetSizer(headerSizer);
    rootSizer->Add(headerPanel, 0, wxEXPAND);

    // Scrollable Cards
    m_scrollWin = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL);
    m_scrollWin->SetScrollRate(0, 10);
    m_scrollWin->SetBackgroundColour(th.windowBg);
    m_cardsSizer = new wxBoxSizer(wxVERTICAL);
    m_scrollWin->SetSizer(m_cardsSizer);
    rootSizer->Add(m_scrollWin, 1, wxEXPAND | wxALL, 10);

    // Bottom Close Button
    wxPanel* botPanel = new wxPanel(this, wxID_ANY);
    botPanel->SetBackgroundColour(th.headerBg);
    wxBoxSizer* botSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* closeBtn = new wxButton(botPanel, wxID_CANCEL, "Close", wxDefaultPosition, wxSize(100, 32));
    closeBtn->SetDefault();
    botSizer->AddStretchSpacer();
    botSizer->Add(closeBtn, 0, wxALL, 8);
    botPanel->SetSizer(botSizer);
    rootSizer->Add(botPanel, 0, wxEXPAND);

    SetSizer(rootSizer);
    ThemeManager::ApplyThemeToWindow(this);
    CenterOnParent();
}

void TotpHubDialog::PopulateCards() {
    const auto& th = ThemeManager::Colors;
    m_cards.clear();
    m_cardsSizer->Clear(true);

    int count = 0;
    for (const auto& item : m_storage.GetAllItems()) {
        if (item.totpSecret.empty()) continue;

        count++;
        wxPanel* card = new wxPanel(m_scrollWin, wxID_ANY);
        card->SetBackgroundColour(th.cardBg);
        wxBoxSizer* cardSizer = new wxBoxSizer(wxVERTICAL);

        // Account title + section
        wxBoxSizer* titleRow = new wxBoxSizer(wxHORIZONTAL);
        wxStaticText* nameLbl = new wxStaticText(card, wxID_ANY, wxString::FromUTF8(item.label.c_str()));
        wxFont nf = nameLbl->GetFont();
        nf.SetWeight(wxFONTWEIGHT_BOLD);
        nf.SetPointSize(11);
        nameLbl->SetFont(nf);
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
        wxFont cf = codeLbl->GetFont();
        cf.SetPointSize(18);
        cf.SetWeight(wxFONTWEIGHT_BOLD);
        codeLbl->SetFont(cf);
        codeLbl->SetForegroundColour(th.accent);

        wxButton* copyBtn = new wxButton(card, wxID_ANY, "Copy", wxDefaultPosition, wxSize(80, 30));
        std::string secret = item.totpSecret;
        std::string itmId = item.id;
        copyBtn->Bind(wxEVT_BUTTON, [this, secret, itmId, copyBtn](wxCommandEvent&) {
            std::string code = TotpEngine::GenerateCode(secret);
            ClipboardSanitizer::CopyAndScheduleClear(wxString::FromUTF8(code.c_str()), 120);
            copyBtn->SetLabel("Copied! (2m)");
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
        cdLbl->SetForegroundColour(th.textMuted);

        prgRow->Add(gauge, 1, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 12);
        prgRow->Add(cdLbl, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 12);
        cardSizer->Add(prgRow, 0, wxEXPAND | wxBOTTOM, 8);

        card->SetSizer(cardSizer);
        ThemeManager::ApplyThemeToWindow(card);
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
        emptyLbl->SetForegroundColour(th.textMuted);
        m_cardsSizer->Add(emptyLbl, 0, wxALL, 20);
    }

    m_scrollWin->FitInside();
    m_scrollWin->Layout();
}

void TotpHubDialog::OnTimer(wxTimerEvent&) {
    int rem = TotpEngine::GetRemainingSeconds();
    const auto& th = ThemeManager::Colors;

    for (auto& w : m_cards) {
        std::string code = TotpEngine::GenerateCode(w.secret);
        w.codeLabel->SetLabel(TotpEngine::FormatCode(code));
        w.progressGauge->SetValue(rem);
        w.countdownLabel->SetLabel(wxString::Format("%ds", rem));

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
                w.copyBtn->SetLabel("Copy");
            }
        }
    }
}

void TotpHubDialog::OnAddNewTotp(wxCommandEvent&) {
    EntryDialog dlg(this, m_storage.GetCustomTabs(), "General");
    if (dlg.ShowModal() == wxID_OK) {
        VaultItem newItem = dlg.GetVaultItem();
        m_storage.AddItem(newItem);
        m_storage.Save(m_pin);
        PopulateCards();
    }
}
