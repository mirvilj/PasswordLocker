#ifndef DETAIL_DIALOG_H
#define DETAIL_DIALOG_H

#include <wx/wx.h>
#include <wx/timer.h>
#include <wx/gauge.h>
#include "VaultItem.h"

enum DetailDialogAction {
    DetailAction_Close,
    DetailAction_Edit,
    DetailAction_Delete
};

class DetailDialog : public wxDialog {
public:
    DetailDialog(wxWindow* parent, const VaultItem& item);
    virtual ~DetailDialog();

    DetailDialogAction GetSelectedAction() const { return m_action; }
    bool WasItemModified() const { return m_itemModified; }
    const VaultItem& GetItem() const { return m_item; }

private:
    VaultItem m_item;
    DetailDialogAction m_action;
    bool m_itemModified;

    // Attached Image panel
    wxPanel* m_imageCardPanel;
    void OnViewImageFullscreen(wxCommandEvent& evt);

    // 2FA TOTP controls
    wxTimer m_totpTimer;
    wxStaticText* m_totpCodeLabel;
    wxStaticText* m_totpCountdownLabel;
    wxGauge* m_totpProgressGauge;
    wxButton* m_copyTotpBtn;

    // Password show/hide
    wxTextCtrl* m_passwordCtrl;
    wxButton* m_showHidePassBtn;
    bool m_passwordVisible;
    int m_copiedResetCountdown;

    void SetupUI();
    void UpdateTotpDisplay();
    void CopyToClipboard(const wxString& text, const wxString& labelName);

    void OnTimer(wxTimerEvent& evt);
    void OnCopyTotp(wxCommandEvent& evt);
    void OnTogglePassword(wxCommandEvent& evt);
    void OnEdit(wxCommandEvent& evt);
    void OnDelete(wxCommandEvent& evt);
    void OnCloseBtn(wxCommandEvent& evt);

    wxDECLARE_EVENT_TABLE();
};

#endif // DETAIL_DIALOG_H
