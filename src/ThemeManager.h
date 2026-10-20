#ifndef THEME_MANAGER_H
#define THEME_MANAGER_H

#include <wx/wx.h>
#include <string>

enum class ThemeMode {
    Minimalist = 0,
    Dark = 1,
    HighContrast = 2
};

struct ThemeColors {
    wxColour windowBg;
    wxColour headerBg;
    wxColour headerText;
    wxColour cardBg;
    wxColour cardBorder;
    wxColour textPrimary;
    wxColour textSecondary;
    wxColour textMuted;
    wxColour accent;
    wxColour accentText;
    wxColour buttonBg;
    wxColour buttonText;
    wxColour success;
    wxColour warning;
    wxColour danger;
    wxColour tagBg;
    wxColour tagText;
    wxColour sectionHeaderBg;
    wxColour sectionHeaderText;
};

class ThemeManager {
public:
    static ThemeMode CurrentTheme;
    static ThemeColors Colors;

    static void SetTheme(ThemeMode mode);
    static void SetThemeByName(const std::string& name);
    static std::string GetThemeName();
    static ThemeColors GetColors(ThemeMode mode);
    static void ApplyThemeToWindow(wxWindow* win);
};

#endif // THEME_MANAGER_H
