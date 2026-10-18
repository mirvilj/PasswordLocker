#ifndef ENTRY_DIALOG_H
#define ENTRY_DIALOG_H

#include <wx/wx.h>
#include <wx/scrolwin.h>
#include "VaultItem.h"
#include <vector>
#include <string>

struct QuestionRow {
    wxPanel* panel;
    wxTextCtrl* questionCtrl;
    wxTextCtrl* answerCtrl;
    wxButton* removeBtn;
};

struct CustomFieldRow {
    wxPanel* panel;
    wxTextCtrl* labelCtrl;
    wxTextCtrl* valueCtrl;
    wxButton* removeBtn;
};

class EntryDialog : public wxDialog {
public:
    EntryDialog(wxWindow* parent,
                const std::vector<std::string>& availableTabs,
                const std::string& defaultTab = "General",
                const VaultItem* existingItem = nullptr);

    VaultItem GetVaultItem() const;

private:
    std::vector<std::string> m_availableTabs;
    bool m_isEditing;
    VaultItem m_resultItem;

    // Account Type selector
    wxStaticBoxSizer* m_typeBox;
    wxRadioButton* m_typeBasicRadio;
    wxRadioButton* m_typeSecQuestionsRadio;
    wxRadioButton* m_typeMiscRadio;
    AccountType GetSelectedAccountType() const;

    // General fields
    wxTextCtrl* m_labelCtrl;
    wxComboBox* m_tabCombo;
    wxButton* m_newTabBtn;
    wxTextCtrl* m_usernameCtrl;
    wxTextCtrl* m_emailCtrl;
    wxTextCtrl* m_passwordCtrl;
    wxButton* m_togglePasswordBtn;
    wxButton* m_generatePasswordBtn;
    wxTextCtrl* m_websiteCtrl;
    wxTextCtrl* m_totpSecretCtrl;
    wxStaticText* m_totpStatusLabel;
    wxTextCtrl* m_notesCtrl;

    // Panels for dynamic switching
    wxPanel* m_basicCredsPanel;
    wxPanel* m_secQuestionsSection;
    wxPanel* m_miscCustomSection;

    // Containers for dynamic rows
    wxScrolledWindow* m_questionsScroll;
    wxBoxSizer* m_questionsSizer;
    std::vector<QuestionRow> m_questionRows;

    wxScrolledWindow* m_customFieldsScroll;
    wxBoxSizer* m_customFieldsSizer;
    std::vector<CustomFieldRow> m_customFieldRows;

    wxButton* m_addQuestionBtn;
    wxButton* m_addCustomFieldBtn;

    // Image attachment in Misc section
    wxPanel* m_imagePanel;
    wxStaticBitmap* m_imageThumbBmp;
    wxStaticText* m_imageNameLabel;
    wxButton* m_uploadImageBtn;
    wxButton* m_removeImageBtn;
    void UpdateImagePreview();
    void OnUploadImage(wxCommandEvent& evt);
    void OnRemoveImage(wxCommandEvent& evt);

    // UI construction
    void SetupUI();
    void AdaptFieldsToType(AccountType type);

    // Dynamic row management
    void AddQuestionRow(const std::string& q = "", const std::string& a = "");
    void RemoveQuestionRow(wxPanel* panel);

    void AddCustomFieldRow(const std::string& lbl = "", const std::string& val = "");
    void RemoveCustomFieldRow(wxPanel* panel);

    // Event Handlers
    void OnTypeRadioChanged(wxCommandEvent& evt);
    void OnAddQuestionClicked(wxCommandEvent& evt);
    void OnAddCustomFieldClicked(wxCommandEvent& evt);
    void OnGeneratePasswordClicked(wxCommandEvent& evt);
    void OnTogglePasswordClicked(wxCommandEvent& evt);
    void OnAddNewTabClicked(wxCommandEvent& evt);
    void OnTotpTextChange(wxCommandEvent& evt);
    void OnSaveClicked(wxCommandEvent& evt);
    void OnCancelClicked(wxCommandEvent& evt);
};

#endif // ENTRY_DIALOG_H
