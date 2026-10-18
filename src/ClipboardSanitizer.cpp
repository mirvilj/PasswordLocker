#include "ClipboardSanitizer.h"

wxString ClipboardSanitizer::s_copiedText = "";
std::time_t ClipboardSanitizer::s_expireTime = 0;
bool ClipboardSanitizer::s_active = false;

void ClipboardSanitizer::CopyAndScheduleClear(const wxString& text, int clearSeconds) {
    if (wxTheClipboard->Open()) {
        wxTheClipboard->SetData(new wxTextDataObject(text));
        wxTheClipboard->Close();
    }
    s_copiedText = text;
    s_expireTime = std::time(nullptr) + clearSeconds;
    s_active = (clearSeconds > 0);
}

void ClipboardSanitizer::CheckAndClearExpired() {
    if (!s_active) return;
    if (std::time(nullptr) >= s_expireTime) {
        if (wxTheClipboard->Open()) {
            if (wxTheClipboard->IsSupported(wxDF_TEXT)) {
                wxTextDataObject data;
                if (wxTheClipboard->GetData(data)) {
                    if (data.GetText() == s_copiedText) {
                        wxTheClipboard->Clear();
                    }
                }
            }
            wxTheClipboard->Close();
        }
        s_active = false;
        s_copiedText.clear();
    }
}

bool ClipboardSanitizer::HasActiveSanitizer() {
    return s_active;
}

int ClipboardSanitizer::GetRemainingSeconds() {
    if (!s_active) return 0;
    int rem = static_cast<int>(s_expireTime - std::time(nullptr));
    return rem > 0 ? rem : 0;
}
