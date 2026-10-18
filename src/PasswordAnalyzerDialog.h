#ifndef PASSWORD_ANALYZER_DIALOG_H
#define PASSWORD_ANALYZER_DIALOG_H

#include <wx/wx.h>
#include "VaultStorage.h"
#include <map>
#include <vector>

class PasswordAnalyzerDialog : public wxDialog {
public:
    PasswordAnalyzerDialog(wxWindow* parent, VaultStorage& storage);

private:
    VaultStorage& m_storage;

    void SetupUI();
    std::string GenerateSuggestedPassword();

    void OnCopyGenerated(const wxString& pass);
    void OnClose(wxCommandEvent& evt);

    wxDECLARE_EVENT_TABLE();
};

#endif // PASSWORD_ANALYZER_DIALOG_H
