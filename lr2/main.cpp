#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include "LfsrLogic.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>

#include <string>
#include <vector>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "comdlg32.lib")

namespace {

constexpr int kMargin = 8;
constexpr int kClientW = 680;
constexpr int kOutputH = 140;
constexpr int kTextH = 72;

enum ControlId : int {
    ID_EDIT_TEXT = 100,
    ID_EDIT_FILE,
    ID_BTN_BROWSE_FILE,
    ID_EDIT_SOURCE_BITS,
    ID_EDIT_KEY,
    ID_STATIC_KEY_ERROR,
    ID_BTN_XOR,
    ID_BTN_SAVE,
    ID_EDIT_LFSR_BITS,
    ID_EDIT_RESULT_BITS,
};

HWND g_hwnd = nullptr;
HWND g_editText = nullptr;
HWND g_editFile = nullptr;
HWND g_editSourceBits = nullptr;
HWND g_editKey = nullptr;
HWND g_staticKeyError = nullptr;
HWND g_btnSave = nullptr;
HWND g_editLfsrBits = nullptr;
HWND g_editResultBits = nullptr;

HFONT g_font = nullptr;
HFONT g_fontMono = nullptr;

std::vector<uint8_t> g_sourceBytes;
std::vector<uint8_t> g_resultBytes;
std::wstring g_outputFileName = L"encrypted_text.bin";
bool g_sourceFromText = true;

HWND create_label(HWND parent, const wchar_t *text, int x, int y, int w, int h)
{
    HWND hwnd = CreateWindowW(L"STATIC", text, WS_CHILD | WS_VISIBLE, x, y, w, h, parent, nullptr,
                              GetModuleHandleW(nullptr), nullptr);
    SendMessageW(hwnd, WM_SETFONT, reinterpret_cast<WPARAM>(g_font), TRUE);
    return hwnd;
}

HWND create_edit(HWND parent, int id, int x, int y, int w, int h, DWORD extraStyle = 0)
{
    DWORD style = WS_CHILD | WS_VISIBLE | WS_BORDER | extraStyle;
    HWND hwnd =
        CreateWindowW(L"EDIT", L"", style, x, y, w, h, parent, reinterpret_cast<HMENU>(id),
                      GetModuleHandleW(nullptr), nullptr);
    SendMessageW(hwnd, WM_SETFONT, reinterpret_cast<WPARAM>(g_font), TRUE);
    return hwnd;
}

HWND create_readonly_bits(HWND parent, int id, int x, int y, int w, int h)
{
    HWND hwnd = CreateWindowW(
        L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_READONLY | WS_VSCROLL | ES_AUTOVSCROLL,
        x, y, w, h, parent, reinterpret_cast<HMENU>(id), GetModuleHandleW(nullptr), nullptr);
    SendMessageW(hwnd, WM_SETFONT, reinterpret_cast<WPARAM>(g_fontMono), TRUE);
    return hwnd;
}

HWND create_button(HWND parent, const wchar_t *text, int id, int x, int y, int w, int h, bool enabled = true)
{
    HWND hwnd = CreateWindowW(L"BUTTON", text, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, x, y, w, h, parent,
                              reinterpret_cast<HMENU>(id), GetModuleHandleW(nullptr), nullptr);
    SendMessageW(hwnd, WM_SETFONT, reinterpret_cast<WPARAM>(g_font), TRUE);
    EnableWindow(hwnd, enabled ? TRUE : FALSE);
    return hwnd;
}

HWND create_group(HWND parent, const wchar_t *text, int x, int y, int w, int h)
{
    HWND hwnd = CreateWindowW(L"BUTTON", text, WS_CHILD | WS_VISIBLE | BS_GROUPBOX, x, y, w, h, parent, nullptr,
                              GetModuleHandleW(nullptr), nullptr);
    SendMessageW(hwnd, WM_SETFONT, reinterpret_cast<WPARAM>(g_font), TRUE);
    return hwnd;
}

std::wstring get_text(HWND edit)
{
    int len = GetWindowTextLengthW(edit);
    if (len <= 0)
        return L"";
    std::wstring text(static_cast<size_t>(len), L'\0');
    GetWindowTextW(edit, text.data(), len + 1);
    return text;
}

void set_text(HWND edit, const std::wstring &text)
{
    SetWindowTextW(edit, text.c_str());
}

void show_error(const std::wstring &msg)
{
    MessageBoxW(g_hwnd, msg.c_str(), L"Ошибка", MB_ICONERROR);
}

void set_save_enabled(bool enabled)
{
    EnableWindow(g_btnSave, enabled ? TRUE : FALSE);
}

void reset_crypto_outputs()
{
    set_save_enabled(false);
    g_resultBytes.clear();
    set_text(g_editLfsrBits, L"Здесь появятся биты сгенерированного ключа...");
    set_text(g_editResultBits, L"Здесь появятся биты результата...");
}

void update_source_bits_display()
{
    if (g_sourceBytes.empty()) {
        set_text(g_editSourceBits, L"Здесь появятся биты исходных данных...");
        return;
    }
    const std::wstring bits = bytes_to_bits_string(g_sourceBytes);
    set_text(g_editSourceBits, bits.empty() ? L"Здесь появятся биты исходных данных..." : bits);
}

void update_key_validation_ui()
{
    const LfsrKeyValidation v = validate_register_key(get_text(g_editKey));
    set_text(g_staticKeyError, v.error);
}

void reload_source_from_text()
{
    g_sourceFromText = true;
    g_sourceBytes = wide_to_utf8(get_text(g_editText));
    g_outputFileName = suggest_output_filename(L"", true);
    set_text(g_editFile, L"");
    update_source_bits_display();
    reset_crypto_outputs();
}

void reload_source_from_file(const std::wstring &path)
{
    std::wstring error;
    std::vector<uint8_t> data;
    if (!read_binary_file(path, data, error)) {
        show_error(error);
        g_sourceBytes.clear();
        set_text(g_editSourceBits, L"Ошибка при чтении файла.");
        reset_crypto_outputs();
        return;
    }

    g_sourceFromText = false;
    g_sourceBytes = std::move(data);
    g_outputFileName = suggest_output_filename(path, false);
    set_text(g_editText, L"");
    update_source_bits_display();
    reset_crypto_outputs();
}

bool browse_open(std::wstring &path)
{
    wchar_t file[MAX_PATH] = {};
    OPENFILENAMEW ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = g_hwnd;
    ofn.lpstrFile = file;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"Все файлы (*.*)\0*.*\0";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    if (!GetOpenFileNameW(&ofn))
        return false;
    path = file;
    return true;
}

bool browse_save(std::wstring &path)
{
    wchar_t file[MAX_PATH] = {};
    if (!path.empty())
        wcsncpy_s(file, path.c_str(), _TRUNCATE);

    OPENFILENAMEW ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = g_hwnd;
    ofn.lpstrFile = file;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"Все файлы (*.*)\0*.*\0";
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
    if (!GetSaveFileNameW(&ofn))
        return false;
    path = file;
    return true;
}

void on_browse_file()
{
    std::wstring path;
    if (!browse_open(path))
        return;
    set_text(g_editFile, path);
    reload_source_from_file(path);
}

void on_xor()
{
    if (g_sourceBytes.empty()) {
        set_text(g_editResultBits, L"Сначала введите текст или выберите файл.");
        return;
    }

    const std::wstring key = get_text(g_editKey);
    const LfsrEncryptResult enc = lfsr_xor_crypt(g_sourceBytes, key);
    if (!enc.ok) {
        set_text(g_editResultBits, enc.error);
        return;
    }

    g_resultBytes = enc.result;
    const std::wstring lfsrBits = bytes_to_bits_string(enc.keyPreview, g_sourceBytes.size());
    set_text(g_editLfsrBits, lfsrBits.empty() ? L"Здесь появятся биты сгенерированного ключа..." : lfsrBits);
    const std::wstring resultBits = bytes_to_bits_string(g_resultBytes);
    set_text(g_editResultBits,
             resultBits.empty() ? L"Здесь появятся биты результата..." : resultBits);
    set_save_enabled(true);
}

void on_save()
{
    if (g_resultBytes.empty())
        return;

    std::wstring path = g_outputFileName;
    if (!browse_save(path))
        return;

    if (!write_binary_file(path, g_resultBytes)) {
        show_error(L"Не удалось сохранить файл.");
        return;
    }
    g_outputFileName = path;
}

void on_key_changed()
{
    const std::wstring filtered = filter_binary_key(get_text(g_editKey));
    if (filtered != get_text(g_editKey))
        set_text(g_editKey, filtered);
    update_key_validation_ui();
}

void create_ui(HWND hwnd)
{
    const int innerW = kClientW - 2 * kMargin;
    int y = kMargin;

    create_label(hwnd, L"LFSR — Шифрование / Дешифрование", kMargin, y, innerW, 24);
    y += 32;

    create_group(hwnd, L"1. Исходные данные", kMargin, y, innerW, 56 + kTextH + kOutputH);
    int rowY = y + 22;
    create_label(hwnd, L"Текст:", kMargin + 12, rowY, 48, 20);
    g_editText = create_edit(hwnd, ID_EDIT_TEXT, kMargin + 60, rowY - 2, innerW - 72, kTextH,
                             ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL);

    rowY += kTextH + 8;
    create_label(hwnd, L"Файл:", kMargin + 12, rowY, 48, 20);
    g_editFile = create_edit(hwnd, ID_EDIT_FILE, kMargin + 60, rowY - 2, innerW - 168, 22, ES_READONLY | ES_AUTOHSCROLL);
    create_button(hwnd, L"Обзор...", ID_BTN_BROWSE_FILE, kMargin + innerW - 96, rowY - 2, 84, 24);

    rowY += 30;
    g_editSourceBits = create_readonly_bits(hwnd, ID_EDIT_SOURCE_BITS, kMargin + 12, rowY, innerW - 24, kOutputH);
    set_text(g_editSourceBits, L"Здесь появятся биты исходных данных...");

    y += 56 + kTextH + kOutputH + 10;
    create_group(hwnd, L"2. Ключ и управление", kMargin, y, innerW, 56 + 36 + kOutputH * 2);
    rowY = y + 22;
    create_label(hwnd, L"Ключ (38 бит):", kMargin + 12, rowY, 110, 20);
    g_editKey = create_edit(hwnd, ID_EDIT_KEY, kMargin + 124, rowY - 2, innerW - 136, 22, ES_AUTOHSCROLL);
    SendMessageW(g_editKey, EM_SETLIMITTEXT, kRegisterLength, 0);

    rowY += 26;
    g_staticKeyError = create_label(hwnd, L"", kMargin + 12, rowY, innerW - 24, 18);
    SendMessageW(g_staticKeyError, WM_SETFONT, reinterpret_cast<WPARAM>(g_font), TRUE);

    rowY += 22;
    create_button(hwnd, L"Зашифровать / Расшифровать (XOR)", ID_BTN_XOR, kMargin + 12, rowY, 300, 32);
    g_btnSave = create_button(hwnd, L"Сохранить результат", ID_BTN_SAVE, kMargin + 322, rowY, 200, 32, false);

    rowY += 40;
    create_label(hwnd, L"Сгенерированный ключ (LFSR):", kMargin + 12, rowY, innerW - 24, 18);
    rowY += 20;
    g_editLfsrBits = create_readonly_bits(hwnd, ID_EDIT_LFSR_BITS, kMargin + 12, rowY, innerW - 24, kOutputH);
    set_text(g_editLfsrBits, L"Здесь появятся биты сгенерированного ключа...");

    rowY += kOutputH + 8;
    create_label(hwnd, L"Результат:", kMargin + 12, rowY, innerW - 24, 18);
    rowY += 20;
    g_editResultBits = create_readonly_bits(hwnd, ID_EDIT_RESULT_BITS, kMargin + 12, rowY, innerW - 24, kOutputH);
    set_text(g_editResultBits, L"Здесь появятся биты результата...");
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_COMMAND:
        switch (HIWORD(wParam)) {
        case EN_CHANGE:
            if (LOWORD(wParam) == ID_EDIT_TEXT) {
                reload_source_from_text();
                return 0;
            }
            if (LOWORD(wParam) == ID_EDIT_KEY) {
                on_key_changed();
                return 0;
            }
            break;
        default:
            break;
        }
        switch (LOWORD(wParam)) {
        case ID_BTN_BROWSE_FILE:
            on_browse_file();
            return 0;
        case ID_BTN_XOR:
            on_xor();
            return 0;
        case ID_BTN_SAVE:
            on_save();
            return 0;
        }
        break;
    case WM_DESTROY:
        if (g_font)
            DeleteObject(g_font);
        if (g_fontMono)
            DeleteObject(g_fontMono);
        PostQuitMessage(0);
        return 0;
    default:
        break;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

} // namespace

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
    INITCOMMONCONTROLSEX icc = {sizeof(icc), ICC_STANDARD_CLASSES};
    InitCommonControlsEx(&icc);

    g_font = CreateFontW(-13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                         CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    g_fontMono = CreateFontW(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                             CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, L"Consolas");

    const wchar_t *className = L"TI2LfsrWindow";
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    wc.lpszClassName = className;
    RegisterClassExW(&wc);

    g_hwnd = CreateWindowExW(0, className, L"LFSR Шифрование и Дешифрование",
                             WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT,
                             CW_USEDEFAULT, kClientW + 16, 820, nullptr, nullptr, hInstance, nullptr);

    create_ui(g_hwnd);
    ShowWindow(g_hwnd, nCmdShow);
    UpdateWindow(g_hwnd);

    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
}
