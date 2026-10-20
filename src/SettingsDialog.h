#ifndef SETTINGS_DIALOG_H
#define SETTINGS_DIALOG_H

#include <wx/wx.h>
#include "VaultStorage.h"

class SettingsDialog : public wxDialog {
public:
    SettingsDialog(wxWindow* parent, const VaultSettings& currentSettings);

    VaultSettings GetSettings() const { return m_settings; }

private:
    VaultSettings m_settings;
    std::string m_originalTheme;

    wxPanel* m_headerPanel;
    wxStaticText* m_title;
    wxStaticText* m_sub;
    wxScrolledWindow* m_body;

    wxStaticBoxSizer* m_lockBox;
    wxStaticText* m_lockHelp;
    wxChoice* m_autoLockChoice;

    wxStaticBoxSizer* m_backupBox;
    wxStaticText* m_backupHelp;
    wxChoice* m_autoBackupChoice;

    wxStaticBoxSizer* m_clipBox;
    wxCheckBox* m_clipboardClearCheck;

    wxStaticBoxSizer* m_keyModeBox;
    wxRadioButton* m_keyPinRadio;
    wxRadioButton* m_keyPassRadio;

    wxStaticBoxSizer* m_themeBox;
    wxRadioButton* m_themeMinimalistRadio;
    wxRadioButton* m_themeDarkRadio;
    wxRadioButton* m_themeHighContrastRadio;

    wxPanel* m_botPanel;
    wxButton* m_cancelBtn;
    wxButton* m_saveBtn;

    void SetupUI();
    void ApplyTheme();
    void OnThemeChanged(wxCommandEvent& evt);
    void OnSave(wxCommandEvent& evt);
    void OnCancel(wxCommandEvent& evt);

    wxDECLARE_EVENT_TABLE();
};

#endif // SETTINGS_DIALOG_H
