#ifndef CLIPBOARD_SANITIZER_H
#define CLIPBOARD_SANITIZER_H

#include <wx/wx.h>
#include <wx/clipbrd.h>
#include <ctime>

class ClipboardSanitizer {
public:
    static void CopyAndScheduleClear(const wxString& text, int clearSeconds = 120);
    static void CheckAndClearExpired();
    static bool HasActiveSanitizer();
    static int GetRemainingSeconds();

private:
    static wxString s_copiedText;
    static std::time_t s_expireTime;
    static bool s_active;
};

#endif // CLIPBOARD_SANITIZER_H
