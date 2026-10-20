#ifndef HELP_DIALOG_H
#define HELP_DIALOG_H

#include <wx/wx.h>
#include <vector>
#include <string>

struct HelpChapter {
    std::string title;
    std::string summary;
    std::vector<std::string> sections;
};

class HelpDialog : public wxDialog {
public:
    HelpDialog(wxWindow* parent);

private:
    std::vector<HelpChapter> m_chapters;
    wxListBox* m_chapterList;
    wxScrolledWindow* m_contentScroll;
    wxBoxSizer* m_contentSizer;

    void InitChapters();
    void SetupUI();
    void DisplayChapter(size_t index);
    void OnChapterSelected(wxCommandEvent& evt);
    void OnClose(wxCommandEvent& evt);

    wxDECLARE_EVENT_TABLE();
};

#endif // HELP_DIALOG_H
