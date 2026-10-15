#ifndef TOTP_HUB_DIALOG_H
#define TOTP_HUB_DIALOG_H

#include <wx/wx.h>
#include <wx/timer.h>
#include <wx/scrolwin.h>
#include <wx/gauge.h>
#include "VaultStorage.h"
#include <vector>

struct TotpCardWidgets {
    std::string itemId;
    std::string secret;
    wxStaticText* codeLabel;
    wxStaticText* countdownLabel;
    wxGauge* progressGauge;
    wxButton* copyBtn;
    int copiedResetCountdown = 0;
};

class TotpHubDialog : public wxDialog {
public:
    TotpHubDialog(wxWindow* parent, VaultStorage& storage, const std::string& currentPin);
    virtual ~TotpHubDialog();

private:
    VaultStorage& m_storage;
    std::string m_pin;
    wxTimer m_timer;
    wxScrolledWindow* m_scrollWin;
    wxBoxSizer* m_cardsSizer;
    std::vector<TotpCardWidgets> m_cards;

    void SetupUI();
    void PopulateCards();
    void UpdateAllCodes();

    void OnTimer(wxTimerEvent& evt);
    void OnAddNewTotp(wxCommandEvent& evt);

    wxDECLARE_EVENT_TABLE();
};

#endif // TOTP_HUB_DIALOG_H
