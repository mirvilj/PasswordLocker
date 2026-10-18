cmake_minimum_required(VERSION 3.16)
project(EC_Lab_Vault VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Directory variables
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(WX_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/wxWidgets")

# Source files
set(SOURCES
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/MainWindow.cpp
    ${SRC_DIR}/ItemCardPanel.cpp
    ${SRC_DIR}/EntryDialog.cpp
    ${SRC_DIR}/DetailDialog.cpp
    ${SRC_DIR}/ImageViewerDialog.cpp
    ${SRC_DIR}/PinDialog.cpp
    ${SRC_DIR}/TotpHubDialog.cpp
    ${SRC_DIR}/VaultStorage.cpp
    ${SRC_DIR}/VaultItem.cpp
    ${SRC_DIR}/VaultCrypto.cpp
    ${SRC_DIR}/TotpEngine.cpp
<<<<<<< HEAD
    ${SRC_DIR}/ThemeManager.cpp
    ${SRC_DIR}/ClipboardSanitizer.cpp
    ${SRC_DIR}/SettingsDialog.cpp
    ${SRC_DIR}/BackupRestoreDialog.cpp
    ${SRC_DIR}/PasswordAnalyzerDialog.cpp
    ${SRC_DIR}/HelpDialog.cpp
    ${SRC_DIR}/app.rc
=======
>>>>>>> origin
)

set(HEADERS
    ${SRC_DIR}/MainWindow.h
    ${SRC_DIR}/ItemCardPanel.h
    ${SRC_DIR}/EntryDialog.h
    ${SRC_DIR}/DetailDialog.h
    ${SRC_DIR}/ImageViewerDialog.h
    ${SRC_DIR}/PinDialog.h
    ${SRC_DIR}/TotpHubDialog.h
<<<<<<< HEAD
    ${SRC_DIR}/SettingsDialog.h
    ${SRC_DIR}/BackupRestoreDialog.h
    ${SRC_DIR}/PasswordAnalyzerDialog.h
    ${SRC_DIR}/HelpDialog.h
    ${SRC_DIR}/ThemeManager.h
    ${SRC_DIR}/ClipboardSanitizer.h
=======
>>>>>>> origin
    ${SRC_DIR}/VaultStorage.h
    ${SRC_DIR}/VaultItem.h
    ${SRC_DIR}/VaultCrypto.h
    ${SRC_DIR}/TotpEngine.h
    ${SRC_DIR}/json.hpp
)

add_executable(Password_Locker WIN32 ${SOURCES} ${HEADERS})

target_include_directories(Password_Locker PRIVATE
    ${SRC_DIR}
    ${WX_ROOT}/include
    ${WX_ROOT}/lib/vc14x_x64_dll/mswu
)

target_compile_definitions(Password_Locker PRIVATE
    WXUSINGDLL
    __WXMSW__
    _UNICODE
    UNICODE
    _CRT_SECURE_NO_WARNINGS
)

target_link_directories(Password_Locker PRIVATE
    ${WX_ROOT}/lib/vc14x_x64_dll
)

target_link_libraries(Password_Locker PRIVATE
    wxmsw32u_core.lib
    wxbase32u.lib
    wxpng.lib
    wxjpeg.lib
    wxzlib.lib
    bcrypt.lib
    comctl32.lib
    shell32.lib
    ole32.lib
    oleaut32.lib
    uuid.lib
    user32.lib
    gdi32.lib
    advapi32.lib
)

# Copy runtime DLLs to output directory on Windows
add_custom_command(TARGET Password_Locker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${WX_ROOT}/lib/vc14x_x64_dll/wxbase32u_vc14x_x64.dll"
        $<TARGET_FILE_DIR:Password_Locker>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${WX_ROOT}/lib/vc14x_x64_dll/wxmsw32u_core_vc14x_x64.dll"
        $<TARGET_FILE_DIR:Password_Locker>
)
