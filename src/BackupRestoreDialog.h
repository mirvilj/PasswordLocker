#ifndef BACKUP_RESTORE_DIALOG_H
#define BACKUP_RESTORE_DIALOG_H

#include <wx/wx.h>
#include <wx/filepicker.h>
#include "VaultStorage.h"

enum class BackupDialogMode {
    Create,
    Restore
};

class BackupRestoreDialog : public wxDialog {
public:
    BackupRestoreDialog(wxWindow* parent, VaultStorage& storage, BackupDialogMode mode);

    bool WasRestored() const { return m_wasRestored; }

private:
    VaultStorage& m_storage;
    BackupDialogMode m_mode;
    bool m_wasRestored;

    wxFilePickerCtrl* m_filePicker;
    wxTextCtrl* m_keyCtrl;
    wxTextCtrl* m_confirmKeyCtrl;
    wxStaticText* m_errorText;

    void SetupUI();
    void OnExecute(wxCommandEvent& evt);
    void OnCancel(wxCommandEvent& evt);

    wxDECLARE_EVENT_TABLE();
};

#endif // BACKUP_RESTORE_DIALOG_H
