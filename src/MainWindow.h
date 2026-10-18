#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/scrolwin.h>
#include <wx/timer.h>
#include "VaultStorage.h"
#include <string>
#include <vector>
#include <ctime>

class ActivityEventFilter : public wxEventFilter {
public:
    ActivityEventFilter(class MainWindow* win);
    virtual int FilterEvent(wxEvent& event) override;

private:
    class MainWindow* m_win;
};

class MainWindow : public wxFrame {
public:
    MainWindow(const wxString& title);
    virtual ~MainWindow();

    void ResetInactivityTimer();

private:
    VaultStorage m_storage;
    std::string m_currentPin;
    bool m_isUnlockedSession;
    enum class SortDisplayMode { None, Type, Tag };
    SortDisplayMode m_sortDisplayMode;
    bool m_pinReminderShown;

    // Inactivity & Sanitizer Timer (ticks every 1s)
    wxTimer m_tickerTimer;
    std::time_t m_lastActivityTime;
    ActivityEventFilter* m_eventFilter;

    // Menu Bar & Menus
    wxMenuBar* m_menuBar;
    wxMenu* m_fileMenu;
    wxMenu* m_editMenu;
    wxMenu* m_viewMenu;
    wxMenu* m_tabsSubMenu;
    wxMenu* m_toolsMenu;
    wxMenu* m_configMenu;
    wxMenu* m_windowMenu;
    wxMenu* m_helpMenu;

    // Controls
    wxPanel* m_headerPanel;
    wxNotebook* m_notebook;
    wxTextCtrl* m_searchCtrl;
    wxStaticText* m_appTitle;
    wxStaticText* m_statusLabel;
    wxStaticText* m_itemCountLabel;
    wxStaticText* m_clipboardTimerLabel;
    wxStaticText* m_searchPrompt;
    wxStaticText* m_zoomPrompt;
    wxButton* m_newEntryBtn;
    wxButton* m_newTabBtn;
    wxButton* m_totpHubBtn;
    wxButton* m_sortBtn;
    wxButton* m_lockBtn;
    wxButton* m_settingsBtn;
    wxButton* m_pinBtn;
    wxButton* m_zoomOutBtn;
    wxButton* m_zoomInBtn;
    wxButton* m_zoomResetBtn;

    // Containers per tab
    struct TabPageInfo {
        std::string tabName;
        wxPanel* pagePanel;
        wxScrolledWindow* scrollWin;
        wxBoxSizer* cardsSizer;
    };
    std::vector<TabPageInfo> m_tabPages;

    // UI Setup & Theming
    void SetupMenuBar();
    void SetupUI();
    void SetupNotebookTabs();
    void RefreshCurrentTabList();
    void RefreshAllTabs();
    void ApplyTheme();
    void UpdateEditMenuState();

    bool PromptAndVerifyPin(const std::string& purposeLabel = "");
    void LockSession(bool byInactivity = false);
    void CheckPinReminder();

    // Zoom controls
    double m_zoomLevel;
    wxStaticText* m_zoomText;
    void SetZoom(double zoom);
    void OnZoomIn(wxCommandEvent& evt);
    void OnZoomOut(wxCommandEvent& evt);
    void OnZoomReset(wxCommandEvent& evt);
    void OnMouseWheel(wxMouseEvent& evt);

    // Menu & Toolbar Handlers
    void OnNewEntry(wxCommandEvent& evt);
    void OnExit(wxCommandEvent& evt);
    void OnAddNewTab(wxCommandEvent& evt);
    void OnOpenTotpHub(wxCommandEvent& evt);
    void OnLockVault(wxCommandEvent& evt);
    void OnChangePin(wxCommandEvent& evt);
    void OnSearchUpdated(wxCommandEvent& evt);
    void OnNotebookPageChanged(wxBookCtrlEvent& evt);

    // Edit actions (Undo / Redo)
    void OnUndo(wxCommandEvent& evt);
    void OnRedo(wxCommandEvent& evt);

    // View actions (Sorting & Themes)
    void OnSortByTimeNewest(wxCommandEvent& evt);
    void OnSortByTimeOldest(wxCommandEvent& evt);
    void OnSortAlphabeticalAsc(wxCommandEvent& evt);
    void OnSortAlphabeticalDesc(wxCommandEvent& evt);
    void OnSortByType(wxCommandEvent& evt);
    void OnSortByTagsDefault(wxCommandEvent& evt);
    void OnSortByTagsAndItems(wxCommandEvent& evt);
    void OnSortBtnClicked(wxCommandEvent& evt);
    void OnSwitchTabFromMenu(wxCommandEvent& evt);
    void OnSetThemeLight(wxCommandEvent& evt);
    void OnSetThemeDark(wxCommandEvent& evt);
    void OnSetThemeHighContrast(wxCommandEvent& evt);

    // Tools actions
    void OnBackupVault(wxCommandEvent& evt);
    void OnRestoreVault(wxCommandEvent& evt);
    void OnPasswordAnalyzer(wxCommandEvent& evt);

    // Configure actions
    void OnOpenSettings(wxCommandEvent& evt);

    // Window actions
    void OnWindowHome(wxCommandEvent& evt);

    // Help actions
    void OnOpenHelp(wxCommandEvent& evt);
    void OnAbout(wxCommandEvent& evt);

    // Timer Tick
    void OnTickerTimer(wxTimerEvent& evt);

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
