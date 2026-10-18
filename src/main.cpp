#include <wx/wx.h>
#include <wx/image.h>
#include "MainWindow.h"

class VaultApp : public wxApp {
public:
    virtual bool OnInit() override {
        if (!wxApp::OnInit()) {
            return false;
        }

        wxInitAllImageHandlers();

        // Initialize and display main window
        MainWindow* mainFrame = new MainWindow("Password Locker");
        mainFrame->Show(true);
        SetTopWindow(mainFrame);

        return true;
    }
};

wxIMPLEMENT_APP(VaultApp);
