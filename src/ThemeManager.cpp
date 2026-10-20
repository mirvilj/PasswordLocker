#include "ThemeManager.h"

ThemeMode ThemeManager::CurrentTheme = ThemeMode::Minimalist;
ThemeColors ThemeManager::Colors = ThemeManager::GetColors(ThemeMode::Minimalist);

ThemeColors ThemeManager::GetColors(ThemeMode mode) {
    ThemeColors c;
    if (mode == ThemeMode::Dark) {
        c.windowBg = wxColour(15, 23, 42);          // Slate 900
        c.headerBg = wxColour(2, 6, 23);            // Slate 950
        c.headerText = wxColour(248, 250, 252);
        c.cardBg = wxColour(30, 41, 59);            // Slate 800
        c.cardBorder = wxColour(51, 65, 85);        // Slate 700
        c.textPrimary = wxColour(248, 250, 252);    // Crisp white/slate 50
        c.textSecondary = wxColour(226, 232, 240);  // Slate 200
        c.textMuted = wxColour(148, 163, 184);      // Slate 400
        c.accent = wxColour(59, 130, 246);          // Blue 500
        c.accentText = *wxWHITE;
        c.buttonBg = wxColour(51, 65, 85);
        c.buttonText = wxColour(248, 250, 252);
        c.success = wxColour(34, 197, 94);          // Green 500
        c.warning = wxColour(251, 191, 36);         // Amber 400
        c.danger = wxColour(239, 68, 68);           // Red 500
        c.tagBg = wxColour(30, 58, 138);            // Dark Blue
        c.tagText = wxColour(191, 219, 254);
        c.sectionHeaderBg = wxColour(30, 41, 59);
        c.sectionHeaderText = wxColour(56, 189, 248);
    } else if (mode == ThemeMode::HighContrast) {
        c.windowBg = *wxBLACK;                      // Pure Black
        c.headerBg = *wxBLACK;
        c.headerText = wxColour(255, 255, 0);       // High-contrast Yellow
        c.cardBg = *wxBLACK;
        c.cardBorder = *wxWHITE;                    // Solid Pure White
        c.textPrimary = *wxWHITE;
        c.textSecondary = wxColour(255, 255, 0);    // Bright Yellow
        c.textMuted = wxColour(0, 255, 255);        // Cyan
        c.accent = wxColour(0, 255, 255);           // Cyan
        c.accentText = *wxBLACK;
        c.buttonBg = *wxBLACK;
        c.buttonText = *wxWHITE;
        c.success = wxColour(0, 255, 0);            // Lime
        c.warning = wxColour(255, 255, 0);          // Yellow
        c.danger = wxColour(255, 50, 50);           // Red
        c.tagBg = *wxBLACK;
        c.tagText = wxColour(255, 255, 0);
        c.sectionHeaderBg = *wxBLACK;
        c.sectionHeaderText = wxColour(0, 255, 255);
    } else { // Minimalist (Default)
        c.windowBg = wxColour(248, 250, 252);       // Slate 50
        c.headerBg = wxColour(15, 23, 42);          // Slate 900
        c.headerText = *wxWHITE;
        c.cardBg = *wxWHITE;
        c.cardBorder = wxColour(226, 232, 240);     // Slate 200
        c.textPrimary = wxColour(15, 23, 42);       // Slate 900
        c.textSecondary = wxColour(71, 85, 105);    // Slate 600
        c.textMuted = wxColour(100, 116, 139);      // Slate 500
        c.accent = wxColour(37, 99, 235);           // Blue 600
        c.accentText = *wxWHITE;
        c.buttonBg = wxColour(241, 245, 249);
        c.buttonText = wxColour(15, 23, 42);
        c.success = wxColour(22, 163, 74);          // Green 600
        c.warning = wxColour(217, 119, 6);          // Amber 600
        c.danger = wxColour(220, 38, 38);           // Red 600
        c.tagBg = wxColour(239, 246, 255);          // Light Blue
        c.tagText = wxColour(29, 78, 216);
        c.sectionHeaderBg = wxColour(241, 245, 249);
        c.sectionHeaderText = wxColour(30, 41, 59);
    }
    return c;
}

void ThemeManager::SetTheme(ThemeMode mode) {
    CurrentTheme = mode;
    Colors = GetColors(mode);
}

void ThemeManager::SetThemeByName(const std::string& name) {
    if (name == "dark") {
        SetTheme(ThemeMode::Dark);
    } else if (name == "high_contrast") {
        SetTheme(ThemeMode::HighContrast);
    } else {
        SetTheme(ThemeMode::Minimalist);
    }
}

std::string ThemeManager::GetThemeName() {
    switch (CurrentTheme) {
        case ThemeMode::Dark: return "dark";
        case ThemeMode::HighContrast: return "high_contrast";
        default: return "minimalist";
    }
}

void ThemeManager::ApplyThemeToWindow(wxWindow* win) {
    if (!win) return;
    const auto& th = ThemeManager::Colors;

    if (wxStaticText* st = dynamic_cast<wxStaticText*>(win)) {
        wxColour curFg = st->GetForegroundColour();
        if (CurrentTheme == ThemeMode::Dark) {
            bool needsLightening = !curFg.IsOk() ||
                curFg == *wxBLACK || curFg == wxColour(0, 0, 0) ||
                curFg == wxColour(15, 23, 42) || curFg == wxColour(30, 41, 59) ||
                curFg == wxColour(71, 85, 105) || curFg == wxColour(100, 116, 139) ||
                curFg == wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);

            if (!needsLightening && curFg.IsOk() &&
                curFg != th.danger && curFg != th.warning && curFg != th.success && curFg != th.accent &&
                curFg != wxColour(239, 68, 68) && curFg != wxColour(220, 38, 38) &&
                curFg != wxColour(34, 197, 94) && curFg != wxColour(22, 163, 74) &&
                curFg != wxColour(251, 191, 36) && curFg != wxColour(217, 119, 6)) {
                int lum = (curFg.Red() * 299 + curFg.Green() * 587 + curFg.Blue() * 114) / 1000;
                if (lum < 130) {
                    needsLightening = true;
                }
            }

            if (needsLightening) {
                st->SetForegroundColour(th.textPrimary);
            }
        } else if (CurrentTheme == ThemeMode::HighContrast) {
            bool isHighContrastColor = curFg.IsOk() && (
                curFg == *wxWHITE || curFg == wxColour(255, 255, 0) ||
                curFg == wxColour(0, 255, 255) || curFg == wxColour(0, 255, 0) ||
                curFg == wxColour(255, 50, 50)
            );
            if (!isHighContrastColor) {
                st->SetForegroundColour(th.textPrimary);
            }
        } else {
            wxWindow* parent = st->GetParent();
            wxColour parentBg = (parent && parent->GetBackgroundColour().IsOk()) ? parent->GetBackgroundColour() : wxNullColour;
            bool parentIsDark = parentBg.IsOk() && (
                parentBg == th.headerBg || parentBg == th.tagBg ||
                ((parentBg.Red() * 299 + parentBg.Green() * 587 + parentBg.Blue() * 114) / 1000 < 128)
            );
            if (parentIsDark) {
                st->SetForegroundColour(th.headerText);
            } else {
                if (!curFg.IsOk() || curFg == *wxWHITE || curFg == wxColour(248, 250, 252) ||
                    curFg == wxColour(255, 255, 0) || curFg == wxColour(0, 255, 255)) {
                    st->SetForegroundColour(th.textPrimary);
                }
            }
        }
    } else if (wxRadioBox* rb = dynamic_cast<wxRadioBox*>(win)) {
        rb->SetForegroundColour(th.textPrimary);
        wxWindow* p = rb->GetParent();
        rb->SetBackgroundColour(p ? p->GetBackgroundColour() : th.windowBg);
    } else if (wxStaticBox* sb = dynamic_cast<wxStaticBox*>(win)) {
        sb->SetForegroundColour((CurrentTheme == ThemeMode::HighContrast) ? th.textSecondary : th.textPrimary);
        wxWindow* p = sb->GetParent();
        wxColour bg = (p && p->GetBackgroundColour().IsOk()) ? p->GetBackgroundColour() : th.windowBg;
        sb->SetBackgroundColour(bg);
    } else if (wxRadioButton* r = dynamic_cast<wxRadioButton*>(win)) {
        r->SetForegroundColour(th.textPrimary);
        wxWindow* p = r->GetParent();
        wxColour bg = (p && p->GetBackgroundColour().IsOk()) ? p->GetBackgroundColour() : th.windowBg;
        r->SetBackgroundColour(bg);
    } else if (wxCheckBox* cb = dynamic_cast<wxCheckBox*>(win)) {
        cb->SetForegroundColour(th.textPrimary);
        wxWindow* p = cb->GetParent();
        wxColour bg = (p && p->GetBackgroundColour().IsOk()) ? p->GetBackgroundColour() : th.windowBg;
        cb->SetBackgroundColour(bg);
    } else if (wxChoice* ch = dynamic_cast<wxChoice*>(win)) {
        if (CurrentTheme == ThemeMode::Dark) {
            ch->SetBackgroundColour(wxColour(30, 41, 59));
            ch->SetForegroundColour(wxColour(248, 250, 252));
        } else if (CurrentTheme == ThemeMode::HighContrast) {
            ch->SetBackgroundColour(*wxBLACK);
            ch->SetForegroundColour(*wxWHITE);
        } else {
            ch->SetBackgroundColour(*wxWHITE);
            ch->SetForegroundColour(wxColour(15, 23, 42));
        }
    } else if (wxTextCtrl* tc = dynamic_cast<wxTextCtrl*>(win)) {
        if (CurrentTheme == ThemeMode::Dark) {
            tc->SetBackgroundColour(wxColour(30, 41, 59));
            tc->SetForegroundColour(wxColour(248, 250, 252));
        } else if (CurrentTheme == ThemeMode::HighContrast) {
            tc->SetBackgroundColour(*wxBLACK);
            tc->SetForegroundColour(*wxWHITE);
        } else {
            tc->SetBackgroundColour(*wxWHITE);
            tc->SetForegroundColour(wxColour(15, 23, 42));
        }
    } else if (wxComboBox* combo = dynamic_cast<wxComboBox*>(win)) {
        if (CurrentTheme == ThemeMode::Dark) {
            combo->SetBackgroundColour(wxColour(30, 41, 59));
            combo->SetForegroundColour(wxColour(248, 250, 252));
        } else if (CurrentTheme == ThemeMode::HighContrast) {
            combo->SetBackgroundColour(*wxBLACK);
            combo->SetForegroundColour(*wxWHITE);
        } else {
            combo->SetBackgroundColour(*wxWHITE);
            combo->SetForegroundColour(wxColour(15, 23, 42));
        }
    } else if (wxListBox* lb = dynamic_cast<wxListBox*>(win)) {
        if (CurrentTheme == ThemeMode::Dark) {
            lb->SetBackgroundColour(wxColour(30, 41, 59));
            lb->SetForegroundColour(wxColour(248, 250, 252));
        } else if (CurrentTheme == ThemeMode::HighContrast) {
            lb->SetBackgroundColour(*wxBLACK);
            lb->SetForegroundColour(*wxWHITE);
        } else {
            lb->SetBackgroundColour(*wxWHITE);
            lb->SetForegroundColour(wxColour(15, 23, 42));
        }
    } else if (wxButton* btn = dynamic_cast<wxButton*>(win)) {
        wxColour curFg = btn->GetForegroundColour();
        bool isSpecial = (curFg == th.danger || curFg == th.accent ||
                          curFg == wxColour(220, 38, 38) || curFg == wxColour(239, 68, 68));
        if (!isSpecial) {
            if (CurrentTheme == ThemeMode::Dark) {
                btn->SetBackgroundColour(th.buttonBg);
                btn->SetForegroundColour(th.buttonText);
            } else if (CurrentTheme == ThemeMode::HighContrast) {
                btn->SetBackgroundColour(*wxBLACK);
                btn->SetForegroundColour(*wxWHITE);
            } else {
                btn->SetBackgroundColour(th.buttonBg);
                btn->SetForegroundColour(th.buttonText);
            }
        }
    }

    for (wxWindowList::compatibility_iterator node = win->GetChildren().GetFirst(); node; node = node->GetNext()) {
        ApplyThemeToWindow(node->GetData());
    }
}
