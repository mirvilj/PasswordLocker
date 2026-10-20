#include "HelpDialog.h"
#include "ThemeManager.h"

wxBEGIN_EVENT_TABLE(HelpDialog, wxDialog)
    EVT_LISTBOX(wxID_ANY, HelpDialog::OnChapterSelected)
    EVT_BUTTON(wxID_CANCEL, HelpDialog::OnClose)
wxEND_EVENT_TABLE()

HelpDialog::HelpDialog(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, "Password Locker - User Guide & FAQ", wxDefaultPosition, wxSize(800, 620), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
      m_chapterList(nullptr),
      m_contentScroll(nullptr),
      m_contentSizer(nullptr) {

    SetBackgroundColour(ThemeManager::Colors.windowBg);
    InitChapters();
    SetupUI();
    DisplayChapter(0);
}

void HelpDialog::InitChapters() {
    m_chapters = {
        {
            "1. Overview & 100% Offline Vault",
            "Zero-trust, completely offline password storage and two-factor authenticator.",
            {
                "Password Locker is engineered to operate 100% offline with zero cloud or internet dependency. Your data never leaves your computer.",
                "All vault credentials, documents, and 2FA secrets are encrypted on disk (data/vault.dat) using industry-standard AES-256-CBC with keys derived through PBKDF2-SHA256 (25,000 iterations).",
                "Crash Protection: Atomic writes protect your database against power loss or crashes by staging writes in a temporary file and atomically committing them."
            }
        },
        {
            "2. Account Types & Adaptive Forms",
            "Adding accounts with dynamic input forms tailored to each credential type.",
            {
                "Type 1: Basic App Password (General Purpose)\n"
                "- Suitable for everyday web accounts, services, and software licenses.\n"
                "- Fields: Account Label, Username, Email, Password, Website URL, Notes, and optional 2FA Secret Key.\n"
                "- Built-in Strong Password Generator and Show/Hide password toggle.",

                "Type 2: Accounts with Security Questions\n"
                "- Designed for financial portals, banking, and sensitive accounts requiring challenge responses.\n"
                "- Features a dynamic [+ Add Question] tool to record unlimited question-and-answer pairs with individual removal controls.",

                "Type 3: Miscellaneous & Documents\n"
                "- Flexible key-value storage where you define custom labels (e.g., API Keys, Server IPs, Recovery Seeds).\n"
                "- Features dynamic [+ Add Custom Field] controls to track custom parameters.\n"
                "- Supports encrypted photo and document attachments (PNG, JPG, JPEG, BMP)."
            }
        },
        {
            "3. Documents & Fullscreen Viewer",
            "Encrypted image attachments with pan, zoom, and fullscreen capabilities.",
            {
                "Interactive Image Previews: Unlocked accounts display crisp thumbnail previews of attached documents.",
                "Full Screen Mode: Click the thumbnail or 'View Full Screen' to open a dedicated canvas that auto-zooms the image to fit your display while preserving aspect ratio.",
                "Interactive Zoom: Zoom smoothly between 20% and 500% using mouse scroll or on-screen [+] / [-] buttons.",
                "Pan & Drag: Click and drag anywhere on the canvas to navigate high-resolution documents.",
                "Minimize & Exit: Press Escape (Esc) or click 'Minimize' to return immediately to the vault."
            }
        },
        {
            "4. Master PIN & Passphrase Security",
            "Configuring 4-digit PINs vs alphanumeric Master Passphrases.",
            {
                "Default Master PIN: 1234 (change this immediately under Configure -> Settings or PIN Settings).",
                "Authentication Modes: You can choose between a convenient 4-digit PIN (4-8 digits) or a strong Master Passphrase (between 6 and 63 characters).\n"
                "To switch modes, open Configure -> Settings or click PIN Settings in the toolbar.",
                "Auto-Lock on PIN Change: Whenever you update your PIN or passphrase, the vault automatically locks to verify your new key.",
                "Bi-Weekly PIN Reminders: Every 2 weeks, Password Locker offers a polite reminder to update your master credentials for optimal hygiene. You are never locked out if you choose to postpone."
            }
        },
        {
            "5. Inactivity Auto-Lock & Clipboard Clear",
            "Automated protection against unauthorized physical access.",
            {
                "Inactivity Auto-Lock: Set the vault to lock automatically after 1, 2, 5, 10, 15, or 30 minutes of inactivity.\n"
                "Any mouse movement, click, or keystroke automatically resets the inactivity counter.",
                "Clipboard Sanitization (2 Minutes): When you copy a password, username, security answer, or 2FA token, Password Locker automatically clears the clipboard after 2 minutes to prevent other programs or accidental pastes from leaking sensitive data."
            }
        },
        {
            "6. Integrated 2FA Authenticator (TOTP)",
            "Generate rotating two-factor authentication tokens 100% offline.",
            {
                "RFC 6238 Standard: Fully compatible with Google Authenticator, Authy, and standard Base32 TOTP secret keys.",
                "Live Countdown & Gauge: Unlocked cards and the Authenticator Hub display 6-digit tokens alongside live 30-second countdown bars.",
                "Authenticator Hub: Click 'Authenticator' in the toolbar or Window -> Authenticator Hub to view all active 2FA accounts simultaneously in a dedicated dashboard."
            }
        },
        {
            "7. Tabs, Sections & Drag-and-Drop",
            "Organize accounts into custom categories and custom visual order.",
            {
                "Custom Tabs: Click '+ New Tab' in the header to create custom sections (e.g. Work, Personal, Finances, Crypto).\n"
                "Move items between tabs via right-click context menu.",
                "Drag-and-Drop Reordering: Click and drag any card in the list to reposition it manually.\n"
                "Dedicated 'Up' / 'Down' buttons are also provided on each card for quick manual sorting.",
                "Sorting: View -> Sort Accounts allows instant sorting by Creation Time (Newest/Oldest), Alphabetical (A-Z/Z-A), or Grouped by Type."
            }
        },
        {
            "8. Encrypted Backups & Restoration",
            "Compiling all data into encrypted, text-unreadable .ecvault containers.",
            {
                "Tools -> Backup Vault: Compiles all accounts, saved passwords, security questions, custom fields, document attachments, tabs, and preferences into a single encrypted file (.ecvault).",
                "Text-Editor Resistance: The backup container is encrypted with AES-256 and PBKDF2 (25,000 iterations) with a binary magic header. Opening it with Notepad, VS Code, or text editors displays non-readable scrambled binary data.",
                "Restoring Backups: Go to Tools -> Restore Vault..., select your .ecvault file, and input the encryption key used when creating the backup. The vault validates integrity before restoring."
            }
        },
        {
            "9. Password Health & Reused Passwords",
            "Auditing stored passwords and generating unique suggestions.",
            {
                "Tools -> Password Health & Analyzer scans all stored accounts.\n"
                "- Reused Password Detection: Identifies identical passwords used across multiple accounts and warns of credential stuffing risks.\n"
                "- Weak Password Alerts: Highlights accounts with passwords shorter than 8 characters.\n"
                "- Unique Password Suggestions: Provides a 1-click generator to produce high-entropy 18-character replacement passwords."
            }
        },
        {
            "10. Frequently Asked Questions (FAQ)",
            "Common answers and troubleshooting tips.",
            {
                "Q: Does this application send any data over the internet?\n"
                "A: No. Password Locker has zero network code and does not connect to any server or cloud service. It is 100% local and offline.",

                "Q: What if I forget my Master PIN or Passphrase?\n"
                "A: Because the vault uses zero-knowledge AES-256 encryption, there is no backdoor or master recovery server. Keep a secure backup of your PIN/passphrase in a safe physical location.",

                "Q: How do I undo an accidental edit or deletion?\n"
                "A: Click Edit -> Undo (Ctrl+Z) in the top menu to undo your last action. Edit -> Redo (Ctrl+Y) is also supported.",

                "Q: Can I use Dark Mode or High Contrast?\n"
                "A: Yes! Open Configure -> Settings or View -> Appearance / Theme to switch between Modern Light, Dark Mode, and High-Contrast Mode."
            }
        }
    };
}

void HelpDialog::SetupUI() {
    const auto& th = ThemeManager::Colors;
    wxBoxSizer* rootSizer = new wxBoxSizer(wxVERTICAL);

    // Header Panel
    wxPanel* headerPanel = new wxPanel(this, wxID_ANY);
    headerPanel->SetBackgroundColour(th.headerBg);
    wxBoxSizer* headerSizer = new wxBoxSizer(wxVERTICAL);

    wxStaticText* title = new wxStaticText(headerPanel, wxID_ANY, "Password Locker Documentation & FAQs");
    title->SetForegroundColour(th.headerText);
    wxFont tf = title->GetFont();
    tf.SetPointSize(13);
    tf.SetWeight(wxFONTWEIGHT_BOLD);
    title->SetFont(tf);

    wxStaticText* sub = new wxStaticText(headerPanel, wxID_ANY, "Click any chapter on the left to learn how to use offline features, 2FA, backups, and security settings.");
    sub->SetForegroundColour((ThemeManager::CurrentTheme == ThemeMode::HighContrast) ? th.textSecondary : wxColour(203, 213, 225));

    headerSizer->Add(title, 0, wxALL, 12);
    headerSizer->Add(sub, 0, wxLEFT | wxRIGHT | wxBOTTOM, 12);
    headerPanel->SetSizer(headerSizer);
    rootSizer->Add(headerPanel, 0, wxEXPAND);

    // Splitter row (Chapters on left, reader on right)
    wxBoxSizer* bodyRow = new wxBoxSizer(wxHORIZONTAL);

    // Left List
    wxArrayString chapterTitles;
    for (const auto& ch : m_chapters) {
        chapterTitles.Add(wxString::FromUTF8(ch.title.c_str()));
    }

    m_chapterList = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxSize(250, -1), chapterTitles, wxLB_SINGLE);
    m_chapterList->SetSelection(0);
    bodyRow->Add(m_chapterList, 0, wxEXPAND | wxALL, 10);

    // Right Content Scroller
    m_contentScroll = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL);
    m_contentScroll->SetScrollRate(0, 10);
    m_contentSizer = new wxBoxSizer(wxVERTICAL);
    m_contentScroll->SetSizer(m_contentSizer);

    bodyRow->Add(m_contentScroll, 1, wxEXPAND | wxTOP | wxBOTTOM | wxRIGHT, 10);
    rootSizer->Add(bodyRow, 1, wxEXPAND);

    // Bottom Close Panel
    wxPanel* botPanel = new wxPanel(this, wxID_ANY);
    botPanel->SetBackgroundColour(th.headerBg);
    wxBoxSizer* botSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* closeBtn = new wxButton(botPanel, wxID_CANCEL, "Close Guide", wxDefaultPosition, wxSize(110, 32));
    closeBtn->SetDefault();
    botSizer->AddStretchSpacer();
    botSizer->Add(closeBtn, 0, wxALL, 8);
    botPanel->SetSizer(botSizer);
    rootSizer->Add(botPanel, 0, wxEXPAND);

    SetSizer(rootSizer);
    ThemeManager::ApplyThemeToWindow(this);
    CenterOnParent();
}

void HelpDialog::DisplayChapter(size_t index) {
    if (index >= m_chapters.size()) return;

    const auto& th = ThemeManager::Colors;
    const auto& ch = m_chapters[index];

    m_contentSizer->Clear(true);

    wxPanel* card = new wxPanel(m_contentScroll, wxID_ANY);
    card->SetBackgroundColour(th.cardBg);
    wxBoxSizer* cardSizer = new wxBoxSizer(wxVERTICAL);

    // Chapter Title
    wxStaticText* hTitle = new wxStaticText(card, wxID_ANY, wxString::FromUTF8(ch.title.c_str()));
    hTitle->SetForegroundColour(th.accent);
    wxFont hf = hTitle->GetFont();
    hf.SetPointSize(14);
    hf.SetWeight(wxFONTWEIGHT_BOLD);
    hTitle->SetFont(hf);
    cardSizer->Add(hTitle, 0, wxALL, 12);

    // Summary callout
    wxPanel* callout = new wxPanel(card, wxID_ANY);
    callout->SetBackgroundColour(th.tagBg);
    wxBoxSizer* calloutSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* sumTxt = new wxStaticText(callout, wxID_ANY, wxString::FromUTF8(ch.summary.c_str()));
    sumTxt->SetForegroundColour(th.tagText);
    wxFont sf = sumTxt->GetFont();
    sf.SetWeight(wxFONTWEIGHT_BOLD);
    sumTxt->SetFont(sf);
    calloutSizer->Add(sumTxt, 1, wxALL, 8);
    callout->SetSizer(calloutSizer);
    cardSizer->Add(callout, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 12);

    // Sections
    for (const auto& sec : ch.sections) {
        wxStaticText* secTxt = new wxStaticText(card, wxID_ANY, wxString::FromUTF8(sec.c_str()));
        secTxt->SetForegroundColour(th.textPrimary);
        wxFont bFont = secTxt->GetFont();
        bFont.SetPointSize(10);
        secTxt->SetFont(bFont);
        secTxt->Wrap(480);
        cardSizer->Add(secTxt, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);
    }

    card->SetSizer(cardSizer);
    ThemeManager::ApplyThemeToWindow(card);
    m_contentSizer->Add(card, 0, wxEXPAND | wxALL, 4);

    m_contentScroll->FitInside();
    m_contentScroll->Layout();
    m_contentScroll->Scroll(0, 0);
}

void HelpDialog::OnChapterSelected(wxCommandEvent&) {
    int sel = m_chapterList->GetSelection();
    if (sel >= 0) {
        DisplayChapter(static_cast<size_t>(sel));
    }
}

void HelpDialog::OnClose(wxCommandEvent&) {
    EndModal(wxID_CANCEL);
}
