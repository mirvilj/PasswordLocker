<p align="center">
  <img src="https://github.com/mirvilj/PasswordLocker/blob/main/Password%20Locker.png" alt="Repository Banner" width="100%">
</p>
# Password Locker - Offline Password Vault & 2-Factor Authenticator

A modern, polished, cross-platform C++ GUI application built with **wxWidgets** designed to store confidential passwords, documents/image attachments, and generate Time-based One-Time Passwords (TOTP / 2FA) **100% offline** with zero internet dependency.

---

## Key Features

### 1. Familiar Toolbar & Standard Menu Bar
* **File Menu**:
  * **New Account (`Ctrl+N`)**: Launch adaptive entry modal.
  * **Lock Vault (`Ctrl+L`)**: Immediately secure and lock credentials.
  * **Exit (`Alt+F4`)**: Safely exit the application.
* **Edit Menu**:
  * **Undo Last Action (`Ctrl+Z`)** & **Redo Last Action (`Ctrl+Y`)**: Seamlessly revert or reapply account additions, deletions, edits, reordering, and section tab changes.
* **View Menu**:
  * **Sections / Tabs**: Direct menu switcher to jump to any standard or custom tab.
  * **Sort Accounts**: Instant multi-criteria ordering:
    * Sort by Time Created (Newest First)
    * Sort by Time Created (Oldest First)
    * Sort Alphabetical (A - Z)
    * Sort Alphabetical (Z - A)
    * **Sort by Type (with Section Headers)**: Renders elegant header banners that group accounts by *Basic Passwords*, *Security Questions*, and *Miscellaneous & Documents*.
  * **Appearance / Theme**: Toggle between **Modern Minimalist (Light)**, **Dark Mode**, and **High-Contrast Mode (Accessibility)**.
  * **Zoom Controls**: Quick shortcuts for `Ctrl++`, `Ctrl+-`, and `Ctrl+0`.
* **Tools Menu**:
  * **Backup Vault to File (`Ctrl+B`)**: Compile encrypted `.ecvault` backups with custom passphrases.
  * **Restore Vault from File (`Ctrl+R`)**: Restore vault items and settings from `.ecvault` backup archives.
  * **Password Health & Analyzer**: Deep security audit checking for reused passwords and weak credentials, with 1-click unique suggestions.
* **Configure Menu**:
  * **Settings & Preferences**: Configure inactivity timeouts, automated backup intervals, clipboard wiping, themes, and authentication modes.
  * **Change Master PIN / Passphrase**: Update vault authentication credentials.
* **Window Menu**:
  * **Home (`Ctrl+H`)**: Quick jump to the All Items list.
  * **Authenticator Hub (`Ctrl+T`)**: Open the dedicated 2FA TOTP dashboard.
* **Help Menu**:
  * **How to Use Password Locker (`F1`)**: Interactive, formatted FAQ & user manual with clickable chapters.
  * **About Password Locker**: Application version and cryptographic details.

### 2. Inactivity Auto-Lock & Automatic 2-Minute Clipboard Sanitization
* **Inactivity Auto-Lock**: User-configurable idle lock timer under **Configure -> Settings**.
  * Options: **1 minute, 2 minutes, 5 minutes (default), 10 minutes, 15 minutes, 30 minutes, or Disabled**.
  * Global activity monitoring resets the timer upon any mouse movement, mouse click, or key press.
  * When triggered, automatically locks the vault and hides all credentials, requiring re-authentication.
* **2-Minute Clipboard Auto-Clear**: Automatically wipes copied passwords, emails, security answers, and 2FA tokens from the system clipboard after **2 minutes** to prevent unauthorized pastes or exposure to third-party tools.
* **Live Clipboard Wipe Countdown**: A status indicator in the top header shows remaining seconds until the clipboard is scrubbed.

### 3. Encrypted Backups & Restoration (`.ecvault`)
* **Compiled Encrypted Archives**: Exports a compiled snapshot of all accounts, saved passwords, security questions, custom fields, document attachments, tabs, and organization settings.
* **Non-Printable Binary Container**: Backups are encrypted with **AES-256-CBC** and **PBKDF2-SHA256 (25,000 iterations)** featuring a binary magic signature (`\x89ECVAULT`). Opening the file in Notepad, VS Code, or normal text editors shows scrambled binary data.
* **User-Defined Encryption Key**: Every backup requires an encryption passphrase known only to the user.
* **Secure Restoration**: Prompts for the passphrase, verifies cryptographic integrity, previews item counts, and safely restores accounts with full Undo support.

### 4. Master PIN vs. Passphrase Substitution (6 to 63 Characters)
* Flexible master authentication:
  * **4-Digit Quick PIN**: Numeric PIN (4 to 8 digits).
  * **Master Passphrase / Password**: Alphanumeric and symbol passphrases (**minimum 6 characters, less than 64 characters**).
* **Auto-Lock on PIN / Key Change**: Whenever credentials are changed, the vault automatically locks and prompts the user to re-enter their new PIN/passphrase to verify it before proceeding.
* **Bi-Weekly PIN Hygiene Reminders**: Every 2 weeks, Password Locker gently prompts the user to update their credentials for optimal security hygiene without enforcing arbitrary lockouts.

### 5. Crash-Protection Atomic Writes & Automated Backups
* **Atomic Temp File Writes**: Writes are staged to `data/vault.dat.tmp`, flushed, and atomically committed to prevent database corruption during sudden crashes or power failures.
* **Automated Backup Scheduling**: Set auto-backup frequency under **Configure -> Settings**:
  * Every Day, Every 3 Days, Every Week (Default), Every Month, or Manual Only.
  * Automatically creates timestamped recovery archives in `data/backups/`.

### 6. Password Health & Reused Passwords Audit
* **Reused Credential Detection**: Audits all accounts to find identical passwords reused across services.
* **Weak Password Alerts**: Flags credentials shorter than 8 characters.
* **Security Health Score**: Calculates an overall rating (0 - 100%) based on password diversity and strength.
* **One-Click Unique Suggestions**: Instantly generates cryptographically strong 18-character replacement passwords with copy capabilities.

### 7. Modern Minimalist Design, Dark Mode & High Contrast
* **Modern Minimalist (Light)**: Clean slate background, crisp borders, and subtle contrast.
* **Dark Mode**: Sleek charcoal and deep slate tones for low-light environments.
* **High-Contrast Mode**: Pure black background, solid high-visibility borders, bright yellow secondary text, and cyan accents for accessibility.

### 8. Dynamic Account Types & Adaptive Forms
* **Type 1: Basic App Password (General Purpose)**
* **Type 2: Account with Security Questions** (Dynamic `[+ Add Question]` controls)
* **Type 3: Miscellaneous Information & Image Attachments** (Custom key-value fields and encrypted image uploads)

### 9. Fullscreen Document & Image Viewer
* Unlocked cards display preview thumbnails of attached photos and scanned documents.
* Dedicated fullscreen viewer with auto-zoom to screen fit, interactive zoom up to 500%, pan/drag navigation, and instant `Esc` exit.

### 10. Integrated 2-Factor Authenticator (TOTP)
* Standard RFC 6238 TOTP generator (100% offline).
* Live 30-second rotating code countdown bar with 1-click copying.
* Dedicated **Authenticator Hub** dashboard (`Ctrl+T`).

---

## Project Structure

```
EC_LAb/
├── CMakeLists.txt                 # Cross-platform CMake build configuration
├── build.bat                      # 1-click build script using MSVC 2022
├── run.bat                        # 1-click launcher for Password Locker
├── bin/                           # Output executable and runtime DLLs
│   ├── Password_Locker.exe        # Compiled Password Locker application
│   ├── wxbase32u_vc14x_x64.dll
│   ├── wxmsw32u_core_vc14x_x64.dll
│   └── wxpng.lib, wxjpeg.lib, wxzlib.lib
├── data/                          # Encrypted offline vault storage
│   ├── vault.dat                  # AES-256 encrypted database
│   ├── vault_recovery.dat         # Auto-recovery snapshot
│   └── backups/                   # Automated .ecvault backup archives
├── src/                           # C++ source code
│   ├── main.cpp                   # wxWidgets application entry point & image handlers
│   ├── MainWindow.h/.cpp          # Menu bar, toolbar, tabs, inactivity monitor, zoom
│   ├── ThemeManager.h/.cpp        # Light, Dark, and High-Contrast visual palettes
│   ├── ClipboardSanitizer.h/.cpp  # 2-minute auto-clearing clipboard security engine
│   ├── SettingsDialog.h/.cpp      # Inactivity auto-lock, auto-backup, key mode, theme
│   ├── BackupRestoreDialog.h/.cpp # Encrypted .ecvault archive creator and restorer
│   ├── PasswordAnalyzerDialog.h/.cpp # Reused password audit & generator
│   ├── HelpDialog.h/.cpp          # Clickable FAQ chapters and user guide
│   ├── ItemCardPanel.h/.cpp       # Themed item card with drag-and-drop & bold timestamp
│   ├── EntryDialog.h/.cpp         # Adaptive input form with document upload
│   ├── DetailDialog.h/.cpp        # Unlocked account view, thumbnail preview, live 2FA
│   ├── ImageViewerDialog.h/.cpp   # Fullscreen zoom-to-fit document viewer & pan controls
│   ├── PinDialog.h/.cpp           # 4-digit PIN vs 6-63 char passphrase dialog
│   ├── TotpHubDialog.h/.cpp       # Dedicated 2FA Authenticator Hub
│   ├── VaultStorage.h/.cpp        # Encrypted storage, undo/redo, backups, sorting
│   ├── VaultItem.h/.cpp           # Account data model, image encoding & serialization
│   ├── VaultCrypto.h/.cpp         # AES-256 + PBKDF2 cryptography & Base64 engine
│   ├── TotpEngine.h/.cpp          # RFC 6238 TOTP generator
│   └── json.hpp                   # Modern JSON parser
└── wxWidgets/                     # Pre-configured wxWidgets 3.2.6 headers & x64 DLLs
```

---

## How to Build and Run

### Option 1: 1-Click Launch (Recommended)
Double-click or run:
```bat
run.bat
```
This automatically compiles the project (if not yet built) and launches `Password_Locker.exe`.

### Option 2: Build Manually via `build.bat`
```bat
build.bat
```

### Option 3: Build with CMake
```bat
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

---

## Master Authentication
* **Default Master PIN**: `1234`
* You can change between a **4-digit PIN** or a **password (6-63 characters)** anytime by clicking **`PIN Settings`** in the toolbar or navigating to **`Configure -> Settings`**.
