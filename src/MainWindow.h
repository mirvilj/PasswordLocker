#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/scrolwin.h>
#include "VaultStorage.h"
#include <string>
#include <vector>

class MainWindow : public wxFrame {
public:
    MainWindow(const wxString& title);
    virtual ~MainWindow();

private:
    VaultStorage m_storage;
    std::string m_currentPin;
    bool m_isUnlockedSession;

    // Controls
    wxNotebook* m_notebook;
    wxTextCtrl* m_searchCtrl;
    wxStaticText* m_statusLabel;
    wxStaticText* m_itemCountLabel;

    // Containers per tab
    struct TabPageInfo {
        std::string tabName;
        wxScrolledWindow* scrollWin;
        wxBoxSizer* cardsSizer;
    };
    std::vector<TabPageInfo> m_tabPages;

    void SetupUI();
    void SetupNotebookTabs();
    void RefreshCurrentTabList();
    void RefreshAllTabs();

    bool PromptAndVerifyPin(const std::string& purposeLabel = "");

    // Zoom controls
    double m_zoomLevel;
    wxStaticText* m_zoomText;
    void SetZoom(double zoom);
    void OnZoomIn(wxCommandEvent& evt);
    void OnZoomOut(wxCommandEvent& evt);
    void OnZoomReset(wxCommandEvent& evt);
    void OnMouseWheel(wxMouseEvent& evt);

    // Actions
    void OnNewEntry(wxCommandEvent& evt);
    void OnAddNewTab(wxCommandEvent& evt);
    void OnOpenTotpHub(wxCommandEvent& evt);
    void OnSortByTime(wxCommandEvent& evt);
    void OnLockVault(wxCommandEvent& evt);
    void OnChangePin(wxCommandEvent& evt);
    void OnSearchUpdated(wxCommandEvent& evt);
    void OnNotebookPageChanged(wxBookCtrlEvent& evt);

    // Item Card callbacks
    void OnUnlockItem(const std::string& itemId);
    void OnDeleteItem(const std::string& itemId);
    void OnMoveItemRelative(const std::string& itemId, int direction);
    void OnDropReorder(const std::string& draggedId, const std::string& targetId);
    void OnMoveItemToTab(const std::string& itemId, const std::string& targetTab);

    std::string GetCurrentTabName() const;

    wxDECLARE_EVENT_TABLE();
};

#endif // MAIN_WINDOW_H
