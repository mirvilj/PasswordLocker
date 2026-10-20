# Developer & AI Agent Guide - Password Locker (`EC_LAb`)

## 1. Project Overview

**Password Locker** is an offline, security-focused C++ GUI application built with **wxWidgets 3.2.6**. It serves as an encrypted password organizer, confidential document/image attachment vault, and Time-based One-Time Password (TOTP / RFC 6238) authenticator with zero internet dependency.

- **Primary Language**: C++17
- **UI Toolkit**: wxWidgets (pre-bundled under `wxWidgets/`, compiled for MSVC x64)
- **Target OS**: Windows (x64)
- **Security Standard**: AES-256-CBC, PBKDF2-SHA256, atomic write commits, automated encryption backups.

---

## 2. Directory Structure

```
EC_LAb/
├── CMakeLists.txt                 # CMake configuration
├── build.bat                      # Build script using MSVC 2022 x64
├── run.bat                        # Launcher script (builds if missing)
├── agents.md                      # This AI agent & developer guidelines document
├── README.md                      # End-user documentation & feature overview
├── bin/                           # Build outputs (Password_Locker.exe, DLLs)
├── data/                          # Encrypted vault storage & backup archives
│   ├── vault.dat                  # Current AES-256 encrypted database
│   ├── vault_recovery.dat         # Auto-recovery snapshot
│   └── backups/                   # Automated and manual .ecvault archives
├── src/                           # Application source and header files
└── wxWidgets/                     # Pre-packaged wxWidgets 3.2.6 headers and DLLs
```

---

## 3. Build & Execution Workflow

### Build Environment
- **Compiler**: Microsoft Visual C++ (MSVC 2022 Community) x64 (`vcvars64.bat`).
- **Flags**: `/nologo /EHsc /MD /std:c++17 /O2 /utf-8 /DWXUSINGDLL /D__WXMSW__ /D_UNICODE /DUNICODE /D_CRT_SECURE_NO_WARNINGS`.

### Build Commands
To compile all translation units and link the executable:
```bat
build.bat
```
The script compiles all `.cpp` files in `src/`, links with `wxmsw32u_core.lib`, `wxbase32u.lib`, Windows subsystem libraries, copies runtime DLLs to `bin/`, and produces:
- `bin/Password_Locker.exe`
- `bin/EC_Lab_Vault.exe`

### Running the Application
```bat
run.bat
```

---

## 4. Architecture & Key Modules

| Module | Files | Responsibility |
|---|---|---|
| **Entry Point** | `src/main.cpp` | `wxApp` initialization, image format handlers (`PNG`, `JPEG`, `BMP`), window instantiation. |
| **Main Window** | `src/MainWindow.h/.cpp` | Main frame, menu bar, header toolbar, dynamic category tabs, inactivity detection, zoom controls. |
| **Theming Engine** | `src/ThemeManager.h/.cpp` | Global palette manager for **Minimalist**, **Dark Mode**, and **High-Contrast Mode**. Traverses window hierarchies to ensure WCAG contrast compliance. |
| **Vault Storage** | `src/VaultStorage.h/.cpp` | In-memory item store, atomic disk serialization, undo/redo state stack, multi-criteria sorting, auto-backup checks. |
| **Cryptography** | `src/VaultCrypto.h/.cpp` | AES-256-CBC encryption/decryption, PBKDF2 key derivation (25,000 rounds), Base64 encoding/decoding. |
| **TOTP Engine** | `src/TotpEngine.h/.cpp` | Offline RFC 6238 TOTP calculation with Base32 decoding and dynamic truncated HMAC-SHA1. |
| **Clipboard Sanitizer** | `src/ClipboardSanitizer.h/.cpp` | System clipboard cleaner that automatically wipes credentials after a 2-minute timer. |
| **Item Card UI** | `src/ItemCardPanel.h/.cpp` | Dynamic account row card with drag-and-drop reordering, zoom scaling, and quick actions. |
| **Entry Dialog** | `src/EntryDialog.h/.cpp` | Adaptive modal dialog for creating and editing accounts across Account Types 1, 2, and 3. |
| **Detail Dialog** | `src/DetailDialog.h/.cpp` | Unlocked view displaying credentials, copy buttons, live TOTP token countdowns, and attachment previews. |
| **Settings Dialog** | `src/SettingsDialog.h/.cpp` | User preferences for auto-lock timeouts, auto-backups, clipboard wipes, master key type, and appearance. |
| **PIN / Auth Dialog** | `src/PinDialog.h/.cpp` | Authentication dialog supporting 4-8 digit numeric PINs or 6-63 character master passphrases. |
| **Backup / Restore** | `src/BackupRestoreDialog.h/.cpp` | Container creation and restoration for encrypted `.ecvault` archives. |
| **Password Analyzer** | `src/PasswordAnalyzerDialog.h/.cpp` | Audit dialog identifying reused and weak credentials, with one-click strong password generation. |
| **Document Viewer** | `src/ImageViewerDialog.h/.cpp` | Fullscreen zoom-to-fit viewer for encrypted attached images and documents. |
| **Authenticator Hub** | `src/TotpHubDialog.h/.cpp` | Dedicated multi-token 2FA hub with live countdown progress bars. |

---

## 5. Coding Standards & Invariants for Agents

### 1. UI & Menu Bar Invariants
- **Undo & Redo**: Must **only** be located in the menu bar under the **Edit** section (`&Undo\tCtrl+Z` and `&Redo\tCtrl+Y`). **Do not add on-screen buttons** for undo and redo to the main header toolbar.
- **Vault Status Label (`m_statusLabel`)**: This control is strictly reserved for indicating vault security state (`Vault: Locked`, `Vault: Unlocked`, `Vault: Locked (Inactivity)`, `Vault: Locked (Key Changed)`). **Never overwrite `m_statusLabel`** with temporary operation messages (e.g., undo/redo messages).
- **Default Master PIN**: `1234`.

### 2. Theming & Contrast Invariants
- The application supports three modes: `ThemeMode::Minimalist`, `ThemeMode::Dark`, and `ThemeMode::HighContrast`.
- **Never hardcode dark colors** (e.g. `wxColour(15, 23, 42)`, `wxColour(100, 116, 139)`, or `*wxBLACK`) for text labels or subtexts without checking the current theme mode.
- In `ThemeMode::HighContrast`, use:
  - Background: `*wxBLACK`
  - Primary text: `*wxWHITE`
  - Secondary text / Headers: `wxColour(255, 255, 0)` (Yellow)
  - Accents / Muted hints: `wxColour(0, 255, 255)` (Cyan)
- When introducing new controls or dialogs:
  - Always call `ThemeManager::ApplyThemeToWindow(this)` in the dialog constructor or `SetupUI()`.
  - For `wxStaticBox`, `wxRadioBox`, `wxChoice`, and `wxScrolledWindow`, explicitly assign both foreground and background colours matching the container panel to prevent native Windows GDI theme fallback to black text.

### 3. wxWidgets Memory & Sizer Rules
- **Shadowing Member Variables**: Do not declare local variables with the same name as class member pointers in constructors (e.g., `wxBoxSizer* m_questionsSizer = ...`). Always assign directly to `m_questionsSizer = new wxBoxSizer(...)`.
- Child windows added to a sizer should have their parent properly set to the containing panel or scrolled window.

### 4. Cryptography & Storage Rules
- All file writes to `data/vault.dat` must remain atomic (write to temporary file, flush, commit).
- Always push state to `m_storage.PushUndoState("Action Name")` prior to performing mutating actions on vault items or tabs.
