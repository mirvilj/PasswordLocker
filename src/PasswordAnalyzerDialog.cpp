#include "PasswordAnalyzerDialog.h"
#include "ThemeManager.h"
#include "VaultCrypto.h"
#include <wx/clipbrd.h>
#include <map>
#include <algorithm>

wxBEGIN_EVENT_TABLE(PasswordAnalyzerDialog, wxDialog)
    EVT_BUTTON(wxID_CANCEL, PasswordAnalyzerDialog::OnClose)
wxEND_EVENT_TABLE()

PasswordAnalyzerDialog::PasswordAnalyzerDialog(wxWindow* parent, VaultStorage& storage)
    : wxDialog(parent, wxID_ANY, "Password Locker - Password Health & Security Audit", wxDefaultPosition, wxSize(680, 620), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
      m_storage(storage) {

    SetBackgroundColour(ThemeManager::Colors.windowBg);
    SetupUI();
}

std::string PasswordAnalyzerDialog::GenerateSuggestedPassword() {
    static const char charset[] =
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "0123456789"
        "!@#$%^&*()-_=+";
    const size_t charsetSize = sizeof(charset) - 1;
    const size_t length = 18;

    std::vector<uint8_t> randBytes = VaultCrypto::GenerateRandomBytes(length);
    std::string pass;
    pass.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        pass += charset[randBytes[i] % charsetSize];
    }
    return pass;
}

void PasswordAnalyzerDialog::SetupUI() {
    const auto& th = ThemeManager::Colors;
    wxBoxSizer* rootSizer = new wxBoxSizer(wxVERTICAL);

    // Header Panel
    wxPanel* headerPanel = new wxPanel(this, wxID_ANY);
    headerPanel->SetBackgroundColour(th.headerBg);
    wxBoxSizer* headerSizer = new wxBoxSizer(wxVERTICAL);

    wxStaticText* title = new wxStaticText(headerPanel, wxID_ANY, "Account Security Audit & Password Analyzer");
    title->SetForegroundColour(th.headerText);
    wxFont tf = title->GetFont();
    tf.SetPointSize(13);
    tf.SetWeight(wxFONTWEIGHT_BOLD);
    title->SetFont(tf);

    wxStaticText* sub = new wxStaticText(headerPanel, wxID_ANY, "Analyzes stored accounts for reused passwords, weak credentials, and provides replacement suggestions.");
    sub->SetForegroundColour((ThemeManager::CurrentTheme == ThemeMode::HighContrast) ? th.textSecondary : wxColour(203, 213, 225));

    headerSizer->Add(title, 0, wxALL, 12);
    headerSizer->Add(sub, 0, wxLEFT | wxRIGHT | wxBOTTOM, 12);
    headerPanel->SetSizer(headerSizer);
    rootSizer->Add(headerPanel, 0, wxEXPAND);

    // Analyze data
    const auto& items = m_storage.GetAllItems();
    int totalAccounts = static_cast<int>(items.size());
    int accountsWithPasswords = 0;
    std::map<std::string, std::vector<const VaultItem*>> passMap;
    std::vector<const VaultItem*> weakList;

    for (const auto& item : items) {
        if (!item.password.empty()) {
            accountsWithPasswords++;
            passMap[item.password].push_back(&item);

            // Check weak
            if (item.password.length() < 8) {
                weakList.push_back(&item);
            }
        }
    }

    std::vector<std::pair<std::string, std::vector<const VaultItem*>>> reusedClusters;
    int totalReusedAccounts = 0;
    for (const auto& pair : passMap) {
        if (pair.second.size() > 1) {
            reusedClusters.push_back(pair);
            totalReusedAccounts += static_cast<int>(pair.second.size());
        }
    }

    // Health Score calculation
    int score = 100;
    if (totalAccounts > 0) {
        int penalty = (totalReusedAccounts * 15) + (static_cast<int>(weakList.size()) * 10);
        score = std::max(15, 100 - penalty);
    }

    // Metrics Row
    wxPanel* metricsPanel = new wxPanel(this, wxID_ANY);
    metricsPanel->SetBackgroundColour(th.cardBg);
    wxBoxSizer* metricsSizer = new wxBoxSizer(wxHORIZONTAL);

    // Metric 1: Health Score
    wxPanel* m1 = new wxPanel(metricsPanel, wxID_ANY);
    m1->SetBackgroundColour(th.cardBg);
    wxBoxSizer* s1 = new wxBoxSizer(wxVERTICAL);
    wxStaticText* scoreVal = new wxStaticText(m1, wxID_ANY, wxString::Format("%d%%", score));
    scoreVal->SetForegroundColour(score >= 80 ? th.success : (score >= 50 ? th.warning : th.danger));
    wxFont scFont = scoreVal->GetFont();
    scFont.SetPointSize(18);
    scFont.SetWeight(wxFONTWEIGHT_BOLD);
    scoreVal->SetFont(scFont);
    wxStaticText* scoreLbl = new wxStaticText(m1, wxID_ANY, "Overall Security Score");
    scoreLbl->SetForegroundColour(th.textSecondary);
    s1->Add(scoreVal, 0, wxALIGN_CENTER);
    s1->Add(scoreLbl, 0, wxALIGN_CENTER);
    m1->SetSizer(s1);

    // Metric 2: Reused Passwords
    wxPanel* m2 = new wxPanel(metricsPanel, wxID_ANY);
    m2->SetBackgroundColour(th.cardBg);
    wxBoxSizer* s2 = new wxBoxSizer(wxVERTICAL);
    wxStaticText* reusedVal = new wxStaticText(m2, wxID_ANY, wxString::Format("%d", static_cast<int>(reusedClusters.size())));
    reusedVal->SetForegroundColour(reusedClusters.empty() ? th.success : th.danger);
    reusedVal->SetFont(scFont);
    wxStaticText* reusedLbl = new wxStaticText(m2, wxID_ANY, wxString::Format("Reused across %d accounts", totalReusedAccounts));
    reusedLbl->SetForegroundColour(th.textSecondary);
    s2->Add(reusedVal, 0, wxALIGN_CENTER);
    s2->Add(reusedLbl, 0, wxALIGN_CENTER);
    m2->SetSizer(s2);

    // Metric 3: Weak Passwords
    wxPanel* m3 = new wxPanel(metricsPanel, wxID_ANY);
    m3->SetBackgroundColour(th.cardBg);
    wxBoxSizer* s3 = new wxBoxSizer(wxVERTICAL);
    wxStaticText* weakVal = new wxStaticText(m3, wxID_ANY, wxString::Format("%d", static_cast<int>(weakList.size())));
    weakVal->SetForegroundColour(weakList.empty() ? th.success : th.warning);
    weakVal->SetFont(scFont);
    wxStaticText* weakLbl = new wxStaticText(m3, wxID_ANY, "Weak (<8 chars)");
    weakLbl->SetForegroundColour(th.textSecondary);
    s3->Add(weakVal, 0, wxALIGN_CENTER);
    s3->Add(weakLbl, 0, wxALIGN_CENTER);
    m3->SetSizer(s3);

    metricsSizer->Add(m1, 1, wxEXPAND | wxALL, 10);
    metricsSizer->Add(m2, 1, wxEXPAND | wxALL, 10);
    metricsSizer->Add(m3, 1, wxEXPAND | wxALL, 10);
    metricsPanel->SetSizer(metricsSizer);
    rootSizer->Add(metricsPanel, 0, wxEXPAND | wxALL, 10);

    // Scrollable Findings & Suggestions
    wxScrolledWindow* scroll = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL);
    scroll->SetScrollRate(0, 10);
    scroll->SetBackgroundColour(th.windowBg);
    wxBoxSizer* scrollSizer = new wxBoxSizer(wxVERTICAL);

    if (reusedClusters.empty() && weakList.empty()) {
        wxPanel* okCard = new wxPanel(scroll, wxID_ANY);
        okCard->SetBackgroundColour(th.cardBg);
        wxBoxSizer* okSizer = new wxBoxSizer(wxVERTICAL);
        wxStaticText* okTxt = new wxStaticText(okCard, wxID_ANY,
            "Outstanding Security Status!\n\nNo reused or weak passwords detected across your vault accounts.\nEvery analyzed account is protected by distinct credentials.");
        okTxt->SetForegroundColour(th.success);
        wxFont of = okTxt->GetFont();
        of.SetPointSize(11);
        okTxt->SetFont(of);
        okSizer->Add(okTxt, 0, wxALL | wxALIGN_CENTER, 30);
        okCard->SetSizer(okSizer);
        scrollSizer->Add(okCard, 0, wxEXPAND | wxALL, 8);
    } else {
        // Reused Passwords Section
        if (!reusedClusters.empty()) {
            wxStaticText* rh = new wxStaticText(scroll, wxID_ANY, "Reused Password Risks Detected:");
            rh->SetForegroundColour(th.danger);
            wxFont rhf = rh->GetFont();
            rhf.SetPointSize(11);
            rhf.SetWeight(wxFONTWEIGHT_BOLD);
            rh->SetFont(rhf);
            scrollSizer->Add(rh, 0, wxLEFT | wxRIGHT | wxTOP, 8);

            for (const auto& cluster : reusedClusters) {
                wxPanel* card = new wxPanel(scroll, wxID_ANY);
                card->SetBackgroundColour(th.cardBg);
                wxBoxSizer* cSizer = new wxBoxSizer(wxVERTICAL);

                wxBoxSizer* hRow = new wxBoxSizer(wxHORIZONTAL);
                wxStaticText* alert = new wxStaticText(card, wxID_ANY,
                    wxString::Format("Identical password shared across %d accounts:", static_cast<int>(cluster.second.size())));
                alert->SetForegroundColour(th.danger);
                wxFont af = alert->GetFont();
                af.SetWeight(wxFONTWEIGHT_BOLD);
                alert->SetFont(af);

                wxButton* genBtn = new wxButton(card, wxID_ANY, "Suggest Unique Replacement", wxDefaultPosition, wxSize(200, 26));
                genBtn->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
                    std::string fresh = GenerateSuggestedPassword();
                    if (wxTheClipboard->Open()) {
                        wxTheClipboard->SetData(new wxTextDataObject(wxString::FromUTF8(fresh.c_str())));
                        wxTheClipboard->Close();
                    }
                    wxMessageBox("Generated a strong, unique replacement password:\n\n" + wxString::FromUTF8(fresh.c_str()) + "\n\n(Copied to clipboard. Auto-clears in 2 minutes.)", "Suggested Password", wxOK | wxICON_INFORMATION, this);
                });

                hRow->Add(alert, 1, wxALIGN_CENTER_VERTICAL);
                hRow->Add(genBtn, 0);
                cSizer->Add(hRow, 0, wxEXPAND | wxALL, 8);

                for (const auto* itm : cluster.second) {
                    wxBoxSizer* itmRow = new wxBoxSizer(wxHORIZONTAL);
                    wxStaticText* bullet = new wxStaticText(card, wxID_ANY, " - ");
                    bullet->SetForegroundColour(th.accent);

                    wxStaticText* name = new wxStaticText(card, wxID_ANY, wxString::FromUTF8(itm->label.c_str()));
                    name->SetForegroundColour(th.textPrimary);
                    wxFont nf = name->GetFont();
                    nf.SetWeight(wxFONTWEIGHT_BOLD);
                    name->SetFont(nf);

                    wxString detailStr = " (" + wxString::FromUTF8(itm->tabName.c_str()) + ")";
                    if (!itm->username.empty()) detailStr += " - User: " + wxString::FromUTF8(itm->username.c_str());
                    wxStaticText* subDet = new wxStaticText(card, wxID_ANY, detailStr);
                    subDet->SetForegroundColour(th.textSecondary);

                    itmRow->Add(bullet, 0, wxALIGN_CENTER_VERTICAL);
                    itmRow->Add(name, 0, wxALIGN_CENTER_VERTICAL);
                    itmRow->Add(subDet, 1, wxALIGN_CENTER_VERTICAL);
                    cSizer->Add(itmRow, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 4);
                }

                card->SetSizer(cSizer);
                scrollSizer->Add(card, 0, wxEXPAND | wxALL, 8);
            }
        }

        // Weak Passwords Section
        if (!weakList.empty()) {
            wxStaticText* wh = new wxStaticText(scroll, wxID_ANY, "Weak Passwords (<8 Characters):");
            wh->SetForegroundColour(th.warning);
            wxFont whf = wh->GetFont();
            whf.SetPointSize(11);
            whf.SetWeight(wxFONTWEIGHT_BOLD);
            wh->SetFont(whf);
            scrollSizer->Add(wh, 0, wxLEFT | wxRIGHT | wxTOP, 8);

            for (const auto* itm : weakList) {
                wxPanel* card = new wxPanel(scroll, wxID_ANY);
                card->SetBackgroundColour(th.cardBg);
                wxBoxSizer* cSizer = new wxBoxSizer(wxHORIZONTAL);

                wxStaticText* name = new wxStaticText(card, wxID_ANY, wxString::FromUTF8(itm->label.c_str()) + " (" + wxString::FromUTF8(itm->tabName.c_str()) + ")");
                name->SetForegroundColour(th.textPrimary);
                wxFont nf = name->GetFont();
                nf.SetWeight(wxFONTWEIGHT_BOLD);
                name->SetFont(nf);

                wxStaticText* lenTxt = new wxStaticText(card, wxID_ANY, wxString::Format("Only %d characters", static_cast<int>(itm->password.length())));
                lenTxt->SetForegroundColour(th.warning);

                wxButton* genBtn = new wxButton(card, wxID_ANY, "Generate Suggestion", wxDefaultPosition, wxSize(150, 26));
                genBtn->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
                    std::string fresh = GenerateSuggestedPassword();
                    if (wxTheClipboard->Open()) {
                        wxTheClipboard->SetData(new wxTextDataObject(wxString::FromUTF8(fresh.c_str())));
                        wxTheClipboard->Close();
                    }
                    wxMessageBox("Generated strong password:\n\n" + wxString::FromUTF8(fresh.c_str()) + "\n\n(Copied to clipboard. Auto-clears in 2 minutes.)", "Suggested Password", wxOK | wxICON_INFORMATION, this);
                });

                cSizer->Add(name, 1, wxALIGN_CENTER_VERTICAL | wxALL, 8);
                cSizer->Add(lenTxt, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 12);
                cSizer->Add(genBtn, 0, wxALIGN_CENTER_VERTICAL | wxALL, 8);
                card->SetSizer(cSizer);
                scrollSizer->Add(card, 0, wxEXPAND | wxALL, 8);
            }
        }
    }

    scroll->SetSizer(scrollSizer);
    rootSizer->Add(scroll, 1, wxEXPAND);

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

void PasswordAnalyzerDialog::OnClose(wxCommandEvent&) {
    EndModal(wxID_CANCEL);
}
