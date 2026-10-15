#ifndef PIN_DIALOG_H
#define PIN_DIALOG_H

#include <wx/wx.h>
#include <string>

enum class PinDialogMode {
    UnlockItem,
    SetupInitialPin,
    ChangePin
};

class PinDialog : public wxDialog {
public:
    PinDialog(wxWindow* parent, PinDialogMode mode = PinDialogMode::UnlockItem, const std::string& itemLabel = "");

    std::string GetEnteredPin() const;
    std::string GetNewPin() const;

private:
    PinDialogMode m_mode;
    wxTextCtrl* m_currentPinCtrl;
    wxTextCtrl* m_newPinCtrl;
    wxTextCtrl* m_confirmPinCtrl;
    wxStaticText* m_errorText;

    void OnOk(wxCommandEvent& evt);
    void OnCancel(wxCommandEvent& evt);

    wxDECLARE_EVENT_TABLE();
};

#endif // PIN_DIALOG_H
