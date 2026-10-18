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
<<<<<<< HEAD
    PinDialog(wxWindow* parent,
              PinDialogMode mode = PinDialogMode::UnlockItem,
              const std::string& itemLabel = "",
              bool usePassphraseMode = false);

    std::string GetEnteredPin() const;
    std::string GetNewPin() const;
    bool IsPassphraseMode() const { return m_usePassphraseMode; }

private:
    PinDialogMode m_mode;
    bool m_usePassphraseMode;

    wxRadioButton* m_pinRadio;
    wxRadioButton* m_passRadio;

=======
    PinDialog(wxWindow* parent, PinDialogMode mode = PinDialogMode::UnlockItem, const std::string& itemLabel = "");

    std::string GetEnteredPin() const;
    std::string GetNewPin() const;

private:
    PinDialogMode m_mode;
>>>>>>> origin
    wxTextCtrl* m_currentPinCtrl;
    wxTextCtrl* m_newPinCtrl;
    wxTextCtrl* m_confirmPinCtrl;
    wxStaticText* m_errorText;

<<<<<<< HEAD
    wxButton* m_toggleShowCurrentBtn;
    wxButton* m_toggleShowNewBtn;
    bool m_showingCurrent;
    bool m_showingNew;

    void SetupUI(const std::string& itemLabel);
    void OnModeRadioChanged(wxCommandEvent& evt);
    void OnToggleShowCurrent(wxCommandEvent& evt);
    void OnToggleShowNew(wxCommandEvent& evt);
=======
>>>>>>> origin
    void OnOk(wxCommandEvent& evt);
    void OnCancel(wxCommandEvent& evt);

    wxDECLARE_EVENT_TABLE();
};

#endif // PIN_DIALOG_H
