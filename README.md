<<<<<<< HEAD
# Password Locker - Offline Password Vault & 2-Factor Authenticator

A modern, polished, cross-platform C++ GUI application built with **wxWidgets** designed to store confidential passwords, documents/image attachments, and generate Time-based One-Time Passwords (TOTP / 2FA) **100% offline** with zero internet dependency.

---

## Key Features

### 1. Dynamic Account Types & Adaptive Forms
When adding a new entry, the form dynamically adapts its inputs based on the selected account type:
* **Type 1: Basic App Password (General Purpose)**
  * Flexible and open fields suitable for any account requirement.
  * Fields: Account Label, Username, Email, Password (with built-in Strong Password Generator and Show/Hide toggle), Website URL, Notes, and optional 2FA Secret Key.
* **Type 2: Account with Security Questions**
  * Inherits all general-purpose options from Basic App Passwords.
  * Features a dynamic **`[+ Add Question]`** tool allowing users to log any number of security questions and answers with individual remove (`Del`) controls.
* **Type 3: Miscellaneous Information & Image Attachments**
  * Flexible storage where the user can define their own labels (e.g. `API Key`, `Server IP`, `License Key`, `Safe Combo`, `Recovery Seed`).
  * Features a dynamic **`[+ Add Custom Field]`** tool to create custom key-value pairs.
  * **Document / Photo Upload**: Users can attach images (PNG, JPG, JPEG, BMP). The image is encrypted alongside all metadata and stored securely in the offline vault.
  * Allows creating and choosing custom **tabs / sections** to group similar items together.
  * Multiline documentation area and optional 2FA Secret Key.

### 2. Fullscreen Document & Image Viewer
* **Interactive Image Viewing**: Unlocked accounts display a crisp thumbnail preview of attached documents/photos.
* **Full Screen Mode**: Clicking the thumbnail or "Full Screen" button immediately opens a dedicated fullscreen canvas that auto-zooms the image to perfectly fit the screen while preserving aspect ratio.
* **Navigation & Controls**:
  * **Interactive Zoom**: Zoom in/out smoothly up to 500% with mouse scroll or on-screen `[+]` / `[-]` buttons.
  * **Pan / Drag**: Click and drag to pan across high-resolution documents.
  * **Minimize**: Click the "Minimize" button or press the **`Escape`** key to return to the vault.
  * **Image Removal**: Integrated "Remove Image" option with instant vault synchronization.

### 3. Automated Time-Based Ordering & Bold Timestamp Display
* Every entry records the exact submission timestamp when saved.
* By default, the vault automatically orders items chronologically by submission time.
* **Each item displays its submission time prominently in BOLD text** (e.g., **Submitted: Oct 12, 2026 - 10:45:00 AM**).
* An instant **`Sort by Time`** button in the header re-automates submission time ordering at any time.

### 4. Interactive Zoom Tool (Up to 200%)
* **Toolbar Zoom Controls**: Quick `[-]`, `100%`, `[+]`, and `Reset` buttons in the top header.
* **Mouse Scroll Zoom**: Hold **`Ctrl` + Mouse Wheel Scroll** over the main window to dynamically scale UI elements, text, and cards between 80% and 200%.

### 5. Drag-and-Drop Reordering & Custom Tabs
* **Drag-and-Drop**: Users can click and hold cards in the list to drag them to new positions, enabling manual organization and grouping.
* Dedicated **`Up`** / **`Down`** controls are also available on each card for quick manual reordering.
* **Custom Section Tabs**: Click **`+ New Tab`** in the toolbar to create custom sections (e.g. `Personal`, `Work`, `Finances`, `Development`). Items can be categorized upon creation or moved between tabs via right-click context menu.

### 6. Zero-Trust Master PIN Protection & Hidden Content
* **Confidential Data Hidden by Default**: In the list, all passwords, security question answers, and custom values are kept strictly hidden and masked (`Protected Content`). Only the account label, type badge, and bold submission timestamp are visible.
* **Master PIN Authentication**: Clicking any card or clicking **`Unlock & View`** prompts for the Master PIN.
  * **Default Master PIN**: `1234` (configurable via **`PIN Settings`** in the toolbar).
  * Once unlocked, the details dialog reveals all data with convenient 1-click **Copy** buttons for usernames, emails, passwords, and answers.
* **Offline Encryption**: The vault data (`data/vault.dat`) is encrypted on disk using **AES-256-CBC** with keys derived using **PBKDF2-SHA256 (10,000 iterations)**. Attached images are encrypted as Base64 payloads directly within the vault file.

### 7. Integrated 2-Factor Authenticator (TOTP)
* Standard **RFC 6238 TOTP** engine (HMAC-SHA1 + Base32 decoding).
* Works completely offline—no network required to verify or generate 2FA codes.
* **Live Countdown Display**: Unlocked cards and the Authenticator hub display the live rotating 6-digit code alongside a 30-second progress countdown bar and 1-click code copier.
* **Authenticator Hub**: A dedicated dashboard accessible from the toolbar (**`Authenticator`**) showing all active 2FA accounts and rotating tokens simultaneously.

---

## Project Structure

```
EC_LAb/
├── CMakeLists.txt              # Cross-platform CMake configuration
├── build.bat                   # 1-click build script using MSVC 2022
├── run.bat                     # 1-click launcher for Password Locker
├── bin/                        # Output executable and runtime DLLs
│   ├── Password_Locker.exe     # Compiled Password Locker application
│   ├── wxbase32u_vc14x_x64.dll
│   ├── wxmsw32u_core_vc14x_x64.dll
│   ├── wxpng.lib, wxjpeg.lib, wxzlib.lib
├── data/                       # Encrypted offline vault storage
│   └── vault.dat
├── src/                        # C++ source code
│   ├── main.cpp                # wxWidgets application entry point & image handlers
│   ├── MainWindow.h/.cpp       # Main dashboard, tabs, zoom controller
│   ├── ItemCardPanel.h/.cpp    # Item card with drag-and-drop & bold timestamp
│   ├── EntryDialog.h/.cpp      # Adaptive input form with image attachment upload
│   ├── DetailDialog.h/.cpp     # Unlocked account view, thumbnail preview, live 2FA
│   ├── ImageViewerDialog.h/.cpp# Fullscreen zoom-to-fit document viewer & pan controls
│   ├── PinDialog.h/.cpp        # Master PIN authentication dialog
│   ├── TotpHubDialog.h/.cpp    # Dedicated 2FA Authenticator Hub
│   ├── VaultStorage.h/.cpp     # Encrypted vault manager, tabs, reordering
│   ├── VaultItem.h/.cpp        # Account data model, image encoding & JSON serialization
│   ├── VaultCrypto.h/.cpp      # AES-256 + PBKDF2 cryptography & Base64 engine
│   ├── TotpEngine.h/.cpp       # RFC 6238 TOTP generator
│   └── json.hpp                # Modern JSON parser
└── wxWidgets/                  # Pre-configured wxWidgets 3.2.6 headers & x64 DLLs
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

## Master PIN
* **Default PIN**: `1234`
* You can change the Master PIN anytime by clicking **`PIN Settings`** in the top navigation bar.
=======
# PasswordLocker
An application designed for offline password storage and organization. It consists of frequently used forms and supports the storage of answers to security questions, and uploading images for the possible storage of documents or other information. Also performs 2-factor authentication processes.
>>>>>>> e9f1e8dfa51b947dd8a7aeec83dc01f11bf274f2
