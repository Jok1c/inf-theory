#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include "RsaLogic.h"

#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shlwapi.h>

#include <cstdint>
#include <string>
#include <vector>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "shlwapi.lib")

namespace {

constexpr int kMargin = 14;
constexpr int kClientW = 660;
constexpr int kClientH = 748;
constexpr int kLabelW = 118;
constexpr int kEditW = 120;
constexpr int kRowH = 30;
constexpr int kBtnH = 30;

enum ControlId : int {
    ID_EDIT_P = 100,
    ID_EDIT_Q,
    ID_EDIT_KS,
    ID_EDIT_R,
    ID_EDIT_PHI,
    ID_EDIT_KO,
    ID_EDIT_INPUT,
    ID_EDIT_OUTPUT,
    ID_BTN_BROWSE_IN,
    ID_BTN_BROWSE_OUT,
    ID_BTN_CALC,
    ID_BTN_ENCRYPT,
    ID_BTN_DECRYPT,
    ID_BTN_CLEAR,
    ID_EDIT_LOG,
};

HWND g_hwnd = nullptr;
HWND g_editP = nullptr;
HWND g_editQ = nullptr;
HWND g_editKs = nullptr;
HWND g_editR = nullptr;
HWND g_editPhi = nullptr;
HWND g_editKo = nullptr;
HWND g_editInput = nullptr;
HWND g_editOutput = nullptr;
HWND g_btnEncrypt = nullptr;
HWND g_btnDecrypt = nullptr;
HWND g_editLog = nullptr;

HFONT g_font = nullptr;
HBRUSH g_brushWindow = nullptr;
HBRUSH g_brushLog = nullptr;
HBRUSH g_brushReadOnly = nullptr;

HWND create_label(HWND parent, const wchar_t *text, int x, int y, int w, int h, bool bold = false)
{
    HWND hwnd = CreateWindowW(L"STATIC", text, WS_CHILD | WS_VISIBLE, x, y, w, h, parent, nullptr,
                              GetModuleHandleW(nullptr), nullptr);
    SendMessageW(hwnd, WM_SETFONT, reinterpret_cast<WPARAM>(g_font), TRUE);
    if (bold)
        SendMessageW(hwnd, WM_SETFONT, reinterpret_cast<WPARAM>(g_font), TRUE);
    return hwnd;
}

HWND create_edit(HWND parent, int id, int x, int y, int w, int h, bool readOnly = false)
{
    DWORD style = WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL;
    if (readOnly)
        style |= ES_READONLY;
    HWND hwnd =
        CreateWindowW(L"EDIT", L"", style, x, y, w, h, parent, reinterpret_cast<HMENU>(id),
                      GetModuleHandleW(nullptr), nullptr);
    SendMessageW(hwnd, WM_SETFONT, reinterpret_cast<WPARAM>(g_font), TRUE);
    return hwnd;
}

HWND create_button(HWND parent, const wchar_t *text, int id, int x, int y, int w, int h,
                   bool enabled = true)
{
    HWND hwnd = CreateWindowW(L"BUTTON", text, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, x, y, w, h,
                              parent, reinterpret_cast<HMENU>(id), GetModuleHandleW(nullptr),
                              nullptr);
    SendMessageW(hwnd, WM_SETFONT, reinterpret_cast<WPARAM>(g_font), TRUE);
    EnableWindow(hwnd, enabled ? TRUE : FALSE);
    return hwnd;
}

HWND create_group(HWND parent, const wchar_t *text, int x, int y, int w, int h)
{
    HWND hwnd = CreateWindowW(L"BUTTON", text, WS_CHILD | WS_VISIBLE | BS_GROUPBOX, x, y, w, h,
                              parent, nullptr, GetModuleHandleW(nullptr), nullptr);
    SendMessageW(hwnd, WM_SETFONT, reinterpret_cast<WPARAM>(g_font), TRUE);
    return hwnd;
}

std::wstring get_text(HWND edit)
{
    int len = GetWindowTextLengthW(edit);
    if (len <= 0)
        return L"";
    std::wstring text(len, L'\0');
    GetWindowTextW(edit, text.data(), len + 1);
    return text;
}

void set_text(HWND edit, const std::wstring &text)
{
    SetWindowTextW(edit, text.c_str());
}

void clear_log()
{
    SetWindowTextW(g_editLog, L"");
}

void set_log(const std::vector<std::wstring> &lines)
{
    std::wstring text;
    for (const auto &line : lines) {
        if (!text.empty())
            text += L"\r\n";
        text += line;
    }
    set_text(g_editLog, text);
}

void append_log(const std::wstring &line)
{
    std::wstring current = get_text(g_editLog);
    if (!current.empty())
        current += L"\r\n";
    current += line;
    set_text(g_editLog, current);
}

void show_error(const std::wstring &msg)
{
    MessageBoxW(g_hwnd, msg.c_str(), L"Ошибка", MB_ICONERROR);
}

void focus_field(RsaFocusField field)
{
    HWND target = nullptr;
    switch (field) {
    case RsaFocusField::P:
        target = g_editP;
        break;
    case RsaFocusField::Q:
        target = g_editQ;
        break;
    case RsaFocusField::Ks:
        target = g_editKs;
        break;
    case RsaFocusField::R:
        target = g_editR;
        break;
    default:
        break;
    }
    if (target)
        SetFocus(target);
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

void set_crypto_enabled(bool enabled)
{
    EnableWindow(g_btnEncrypt, enabled ? TRUE : FALSE);
    EnableWindow(g_btnDecrypt, enabled ? TRUE : FALSE);
}

void on_calc_keys()
{
    uint32_t p = 0, q = 0, Ks = 0;
    if (!parse_uint32(get_text(g_editP), p) || !parse_uint32(get_text(g_editQ), q) ||
        !parse_uint32(get_text(g_editKs), Ks)) {
        show_error(L"Введите корректные числовые параметры p, q и Kз.");
        return;
    }

    RsaResult result = compute_keys(p, q, Ks);
    if (!result.ok) {
        show_error(result.error);
        focus_field(result.focus);
        return;
    }

    set_text(g_editR, std::to_wstring(result.r));
    set_text(g_editPhi, std::to_wstring(result.phi));
    set_text(g_editKo, std::to_wstring(result.Ko));
    set_crypto_enabled(true);
    clear_log();
    append_log(L"Ключи вычислены.");
}

void on_encrypt()
{
    if (get_text(g_editInput).empty()) {
        show_error(L"Выберите входной файл!");
        return;
    }
    if (get_text(g_editOutput).empty()) {
        show_error(L"Укажите выходной файл!");
        return;
    }

    uint32_t p = 0, q = 0, Ks = 0;
    if (!parse_uint32(get_text(g_editP), p) || !parse_uint32(get_text(g_editQ), q) ||
        !parse_uint32(get_text(g_editKs), Ks)) {
        show_error(L"Введите корректные параметры.");
        return;
    }

    RsaResult result = encrypt_file(get_text(g_editInput), get_text(g_editOutput), p, q, Ks);
    if (!result.ok) {
        show_error(result.error);
        focus_field(result.focus);
        return;
    }
    set_log(result.log);
}

void on_decrypt()
{
    if (get_text(g_editInput).empty()) {
        show_error(L"Выберите входной файл!");
        return;
    }
    if (get_text(g_editOutput).empty()) {
        show_error(L"Укажите выходной файл!");
        return;
    }

    uint32_t r = 0, Ks = 0;
    if (!parse_uint32(get_text(g_editR), r) || !parse_uint32(get_text(g_editKs), Ks)) {
        show_error(L"Введите корректные r и Kз.");
        return;
    }

    RsaResult result = decrypt_file(get_text(g_editInput), get_text(g_editOutput), r, Ks);
    if (!result.ok) {
        show_error(result.error);
        focus_field(result.focus);
        return;
    }
    set_log(result.log);
}

void on_clear()
{
    set_text(g_editP, L"");
    set_text(g_editQ, L"");
    set_text(g_editKs, L"");
    set_text(g_editR, L"");
    set_text(g_editPhi, L"");
    set_text(g_editKo, L"");
    set_text(g_editInput, L"");
    set_text(g_editOutput, L"");
    clear_log();
    set_crypto_enabled(false);
}

void on_browse_input()
{
    std::wstring path;
    if (!browse_open(path))
        return;
    set_text(g_editInput, path);

    wchar_t outPath[MAX_PATH] = {};
    wcsncpy_s(outPath, path.c_str(), _TRUNCATE);
    PathRemoveExtensionW(outPath);
    wcscat_s(outPath, L".txt");
    set_text(g_editOutput, outPath);
}

void on_browse_output()
{
    std::wstring path = get_text(g_editOutput);
    if (!browse_save(path))
        return;
    set_text(g_editOutput, path);
}

void create_ui(HWND hwnd)
{
    const int contentW = kClientW - 2 * kMargin;
    const int fieldX = kMargin + kLabelW + 10;
    const int fieldW = contentW - kLabelW - 10;
    int y = kMargin;

    create_label(hwnd, L"RSA — Шифрование / Дешифрирование файла", kMargin, y, contentW, 22, true);
    y += 32;

    create_group(hwnd, L"Параметры", kMargin, y, contentW, 108);
    int rowY = y + 22;
    create_label(hwnd, L"p (простое):", kMargin + 10, rowY, kLabelW, 20);
    g_editP = create_edit(hwnd, ID_EDIT_P, fieldX, rowY - 2, kEditW, 24);

    rowY += kRowH;
    create_label(hwnd, L"q (простое):", kMargin + 10, rowY, kLabelW, 20);
    g_editQ = create_edit(hwnd, ID_EDIT_Q, fieldX, rowY - 2, kEditW, 24);

    rowY += kRowH;
    create_label(hwnd, L"Kз (закрытый):", kMargin + 10, rowY, kLabelW, 20);
    g_editKs = create_edit(hwnd, ID_EDIT_KS, fieldX, rowY - 2, kEditW, 24);

    y += 118;
    create_group(hwnd, L"Вычисленные значения", kMargin, y, contentW, 108);
    rowY = y + 22;
    create_label(hwnd, L"r = p*q:", kMargin + 10, rowY, kLabelW, 20);
    g_editR = create_edit(hwnd, ID_EDIT_R, fieldX, rowY - 2, fieldW, 24, true);

    rowY += kRowH;
    create_label(hwnd, L"φ(r):", kMargin + 10, rowY, kLabelW, 20);
    g_editPhi = create_edit(hwnd, ID_EDIT_PHI, fieldX, rowY - 2, fieldW, 24, true);

    rowY += kRowH;
    create_label(hwnd, L"Ko (открытый):", kMargin + 10, rowY, kLabelW, 20);
    g_editKo = create_edit(hwnd, ID_EDIT_KO, fieldX, rowY - 2, fieldW, 24, true);

    y += 118;
    create_button(hwnd, L"Вычислить ключи", ID_BTN_CALC, kMargin + contentW - 180, y, 180, kBtnH);
    y += kBtnH + 14;

    constexpr int kBrowseW = 78;
    create_group(hwnd, L"Файлы", kMargin, y, contentW, 100);
    rowY = y + 22;
    create_label(hwnd, L"Входной файл:", kMargin + 10, rowY, kLabelW, 20);
    create_button(hwnd, L"Обзор...", ID_BTN_BROWSE_IN, fieldX, rowY - 2, kBrowseW, 26);
    g_editInput = create_edit(hwnd, ID_EDIT_INPUT, fieldX + kBrowseW + 8, rowY - 2,
                              fieldW - kBrowseW - 8, 24);

    rowY += kRowH + 2;
    create_label(hwnd, L"Выходной файл:", kMargin + 10, rowY, kLabelW, 20);
    create_button(hwnd, L"Обзор...", ID_BTN_BROWSE_OUT, fieldX, rowY - 2, kBrowseW, 26);
    g_editOutput = create_edit(hwnd, ID_EDIT_OUTPUT, fieldX + kBrowseW + 8, rowY - 2,
                               fieldW - kBrowseW - 8, 24);

    y += 110;
    create_button(hwnd, L"Очистить", ID_BTN_CLEAR, kMargin, y, 130, 36);
    g_btnEncrypt = create_button(hwnd, L"Зашифровать", ID_BTN_ENCRYPT, kMargin + 142, y, 150, 36, false);
    g_btnDecrypt = create_button(hwnd, L"Расшифровать", ID_BTN_DECRYPT, kMargin + 302, y, 150, 36, false);
    y += 46;

    const int logH = kClientH - y - kMargin;
    g_editLog = CreateWindowW(
        L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL,
        kMargin, y, contentW, logH > 80 ? logH : 80, hwnd, reinterpret_cast<HMENU>(ID_EDIT_LOG),
        GetModuleHandleW(nullptr), nullptr);
    SendMessageW(g_editLog, WM_SETFONT, reinterpret_cast<WPARAM>(g_font), TRUE);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_BTN_BROWSE_IN:
            on_browse_input();
            return 0;
        case ID_BTN_BROWSE_OUT:
            on_browse_output();
            return 0;
        case ID_BTN_CALC:
            on_calc_keys();
            return 0;
        case ID_BTN_ENCRYPT:
            on_encrypt();
            return 0;
        case ID_BTN_DECRYPT:
            on_decrypt();
            return 0;
        case ID_BTN_CLEAR:
            on_clear();
            return 0;
        }
        break;
    case WM_CTLCOLORSTATIC: {
        HDC hdc = reinterpret_cast<HDC>(wParam);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(45, 52, 64));
        return reinterpret_cast<LRESULT>(g_brushWindow);
    }
    case WM_CTLCOLOREDIT: {
        HDC hdc = reinterpret_cast<HDC>(wParam);
        HWND ctrl = reinterpret_cast<HWND>(lParam);
        SetBkMode(hdc, OPAQUE);
        if (ctrl == g_editLog) {
            SetBkColor(hdc, RGB(252, 251, 235));
            SetTextColor(hdc, RGB(28, 32, 40));
            return reinterpret_cast<LRESULT>(g_brushLog);
        }
        if (ctrl == g_editR || ctrl == g_editPhi || ctrl == g_editKo) {
            SetBkColor(hdc, RGB(228, 234, 242));
            SetTextColor(hdc, RGB(55, 62, 74));
            return reinterpret_cast<LRESULT>(g_brushReadOnly);
        }
        SetBkColor(hdc, RGB(255, 255, 255));
        SetTextColor(hdc, RGB(20, 24, 32));
        return reinterpret_cast<LRESULT>(GetStockObject(WHITE_BRUSH));
    }
    case WM_CTLCOLORBTN:
        return reinterpret_cast<LRESULT>(g_brushWindow);
    case WM_DESTROY:
        if (g_font)
            DeleteObject(g_font);
        if (g_brushWindow)
            DeleteObject(g_brushWindow);
        if (g_brushLog)
            DeleteObject(g_brushLog);
        if (g_brushReadOnly)
            DeleteObject(g_brushReadOnly);
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

    g_brushWindow = CreateSolidBrush(RGB(243, 246, 250));
    g_brushLog = CreateSolidBrush(RGB(252, 251, 235));
    g_brushReadOnly = CreateSolidBrush(RGB(228, 234, 242));

    const wchar_t *className = L"TI3RsaWindow";
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = g_brushWindow;
    wc.lpszClassName = className;
    RegisterClassExW(&wc);

    const DWORD winStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    RECT rc = {0, 0, kClientW, kClientH};
    AdjustWindowRect(&rc, winStyle, FALSE);
    const int winW = rc.right - rc.left;
    const int winH = rc.bottom - rc.top;

    g_hwnd = CreateWindowExW(0, className, L"RSA", winStyle, CW_USEDEFAULT, CW_USEDEFAULT, winW,
                               winH, nullptr, nullptr, hInstance, nullptr);

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
