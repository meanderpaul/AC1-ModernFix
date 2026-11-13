/* AC1ModernFix-Setup.exe - one-click installer for AC1 Modern Fix.

   Finds Assassin's Creed: Director's Cut (Steam or GOG), checks the game exe, downloads
   Ultimate ASI Loader and EaglePatch from their official GitHub releases (hash-checked),
   installs the embedded AC1AffinityFix plugin, runs the DirectX setup if xinput1_3.dll is
   missing, and can uninstall everything again.

   EaglePatch has no license, so it is never bundled: the installer downloads the official
   release on the user's PC, or uses a copy the user already downloaded.

   No C runtime and no Windows SDK: built with clang + lld, like the plugin (see build.sh).
   Copyright (c) 2026 Meanderpaul_gaming. MIT License. */

/* ---------- minimal Win32 declarations (32-bit) ---------- */
typedef int BOOL; typedef unsigned long DWORD; typedef unsigned short WORD; typedef unsigned char BYTE;
typedef unsigned int UINT; typedef long LONG; typedef __WCHAR_TYPE__ WCHAR;
typedef void *HANDLE, *HWND, *HINSTANCE, *HMODULE, *HICON, *HCURSOR, *HBRUSH, *HFONT, *HDC,
             *HKEY, *HGDIOBJ, *HMENU, *HRSRC, *HGLOBAL, *LPVOID;
typedef unsigned int WPARAM; typedef long LPARAM; typedef long LRESULT; typedef long HRESULT;
typedef unsigned long ULONG_PTR; typedef ULONG_PTR HCRYPTPROV, HCRYPTHASH;
typedef const WCHAR *LPCWSTR; typedef WCHAR *LPWSTR;
#define WINAPI __stdcall
#define IMP __declspec(dllimport)
typedef LRESULT (WINAPI *WNDPROC)(HWND, UINT, WPARAM, LPARAM);
typedef DWORD (WINAPI *THREADPROC)(LPVOID);

typedef struct { UINT cbSize, style; WNDPROC lpfnWndProc; int cbClsExtra, cbWndExtra; HINSTANCE hInstance;
                 HICON hIcon; HCURSOR hCursor; HBRUSH hbrBackground; LPCWSTR lpszMenuName, lpszClassName;
                 HICON hIconSm; } WNDCLASSEXW;
typedef struct { LONG x, y; } POINT;
typedef struct { LONG left, top, right, bottom; } RECT;
typedef struct { HWND hwnd; UINT message; WPARAM wParam; LPARAM lParam; DWORD time; POINT pt; DWORD priv; } MSG;
typedef struct { DWORD cb; LPWSTR lpReserved, lpDesktop, lpTitle; DWORD dwX, dwY, dwXSize, dwYSize,
                 dwXCountChars, dwYCountChars, dwFillAttribute, dwFlags; WORD wShowWindow, cbReserved2;
                 BYTE *lpReserved2; HANDLE hStdInput, hStdOutput, hStdError; } STARTUPINFOW;
typedef struct { HANDLE hProcess, hThread; DWORD dwProcessId, dwThreadId; } PROCESS_INFORMATION;
typedef struct { DWORD lo, hi; } FILETIME;
typedef struct { DWORD dwFileAttributes; FILETIME c, a, w; DWORD nFileSizeHigh, nFileSizeLow, r0, r1;
                 WCHAR cFileName[260]; WCHAR cAlternateFileName[14]; } WIN32_FIND_DATAW;
typedef struct { HWND hwndOwner; LPVOID pidlRoot; LPWSTR pszDisplayName; LPCWSTR lpszTitle; UINT ulFlags;
                 LPVOID lpfn; LPARAM lParam; int iImage; } BROWSEINFOW;
typedef struct { DWORD dwSize, dwICC; } INITCOMMONCONTROLSEX;

/* kernel32 */
IMP HMODULE WINAPI GetModuleHandleW(LPCWSTR);
IMP DWORD WINAPI GetModuleFileNameW(HMODULE, LPWSTR, DWORD);
IMP void WINAPI ExitProcess(UINT);
IMP HANDLE WINAPI CreateThread(LPVOID, DWORD, THREADPROC, LPVOID, DWORD, DWORD *);
IMP BOOL WINAPI CloseHandle(HANDLE);
IMP HANDLE WINAPI CreateFileW(LPCWSTR, DWORD, DWORD, LPVOID, DWORD, DWORD, HANDLE);
IMP BOOL WINAPI WriteFile(HANDLE, const void *, DWORD, DWORD *, LPVOID);
IMP BOOL WINAPI ReadFile(HANDLE, void *, DWORD, DWORD *, LPVOID);
IMP DWORD WINAPI GetFileSize(HANDLE, DWORD *);
IMP DWORD WINAPI GetFileAttributesW(LPCWSTR);
IMP BOOL WINAPI CreateDirectoryW(LPCWSTR, LPVOID);
IMP BOOL WINAPI DeleteFileW(LPCWSTR);
IMP BOOL WINAPI RemoveDirectoryW(LPCWSTR);
IMP BOOL WINAPI CopyFileW(LPCWSTR, LPCWSTR, BOOL);
IMP BOOL WINAPI MoveFileExW(LPCWSTR, LPCWSTR, DWORD);
IMP DWORD WINAPI GetTempPathW(DWORD, LPWSTR);
IMP DWORD WINAPI GetEnvironmentVariableW(LPCWSTR, LPWSTR, DWORD);
IMP HRSRC WINAPI FindResourceW(HMODULE, LPCWSTR, LPCWSTR);
IMP HGLOBAL WINAPI LoadResource(HMODULE, HRSRC);
IMP LPVOID WINAPI LockResource(HGLOBAL);
IMP DWORD WINAPI SizeofResource(HMODULE, HRSRC);
IMP BOOL WINAPI CreateProcessW(LPCWSTR, LPWSTR, LPVOID, LPVOID, BOOL, DWORD, LPVOID, LPCWSTR,
                               STARTUPINFOW *, PROCESS_INFORMATION *);
IMP DWORD WINAPI WaitForSingleObject(HANDLE, DWORD);
IMP BOOL WINAPI GetExitCodeProcess(HANDLE, DWORD *);
IMP UINT WINAPI GetSystemDirectoryW(LPWSTR, UINT);
IMP UINT WINAPI GetWindowsDirectoryW(LPWSTR, UINT);
IMP BOOL WINAPI WritePrivateProfileStringW(LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR);
IMP UINT WINAPI GetPrivateProfileIntW(LPCWSTR, LPCWSTR, int, LPCWSTR);
IMP HANDLE WINAPI FindFirstFileW(LPCWSTR, WIN32_FIND_DATAW *);
IMP BOOL WINAPI FindNextFileW(HANDLE, WIN32_FIND_DATAW *);
IMP BOOL WINAPI FindClose(HANDLE);
IMP HANDLE WINAPI GetProcessHeap(void);
IMP LPVOID WINAPI HeapAlloc(HANDLE, DWORD, ULONG_PTR);
IMP BOOL WINAPI HeapFree(HANDLE, DWORD, LPVOID);
IMP int WINAPI lstrlenW(LPCWSTR);
IMP LPWSTR WINAPI lstrcpynW(LPWSTR, LPCWSTR, int);
IMP int WINAPI lstrcmpiW(LPCWSTR, LPCWSTR);
IMP int WINAPI MultiByteToWideChar(UINT, DWORD, const char *, int, LPWSTR, int);
IMP int WINAPI WideCharToMultiByte(UINT, DWORD, LPCWSTR, int, char *, int, const char *, BOOL *);
IMP int WINAPI MulDiv(int, int, int);
/* user32 */
IMP WORD WINAPI RegisterClassExW(const WNDCLASSEXW *);
IMP HWND WINAPI CreateWindowExW(DWORD, LPCWSTR, LPCWSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID);
IMP LRESULT WINAPI DefWindowProcW(HWND, UINT, WPARAM, LPARAM);
IMP BOOL WINAPI GetMessageW(MSG *, HWND, UINT, UINT);
IMP BOOL WINAPI TranslateMessage(const MSG *);
IMP LRESULT WINAPI DispatchMessageW(const MSG *);
IMP BOOL WINAPI IsDialogMessageW(HWND, MSG *);
IMP void WINAPI PostQuitMessage(int);
IMP LRESULT WINAPI SendMessageW(HWND, UINT, WPARAM, LPARAM);
IMP BOOL WINAPI PostMessageW(HWND, UINT, WPARAM, LPARAM);
IMP BOOL WINAPI SetWindowTextW(HWND, LPCWSTR);
IMP int WINAPI GetWindowTextW(HWND, LPWSTR, int);
IMP BOOL WINAPI EnableWindow(HWND, BOOL);
IMP int WINAPI MessageBoxW(HWND, LPCWSTR, LPCWSTR, UINT);
IMP HCURSOR WINAPI LoadCursorW(HINSTANCE, LPCWSTR);
IMP HICON WINAPI LoadIconW(HINSTANCE, LPCWSTR);
IMP BOOL WINAPI ShowWindow(HWND, int);
IMP BOOL WINAPI UpdateWindow(HWND);
IMP int WINAPI GetSystemMetrics(int);
IMP BOOL WINAPI DestroyWindow(HWND);
IMP BOOL WINAPI AdjustWindowRect(RECT *, DWORD, BOOL);
IMP HDC WINAPI GetDC(HWND);
IMP int WINAPI ReleaseDC(HWND, HDC);
IMP int WINAPI wvsprintfW(LPWSTR, LPCWSTR, __builtin_va_list);
/* gdi32 */
IMP HFONT WINAPI CreateFontW(int, int, int, int, int, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, LPCWSTR);
IMP int WINAPI GetDeviceCaps(HDC, int);
IMP HGDIOBJ WINAPI GetStockObject(int);
IMP DWORD WINAPI SetBkColor(HDC, DWORD);
IMP DWORD WINAPI SetTextColor(HDC, DWORD);
/* advapi32 */
IMP LONG WINAPI RegOpenKeyExW(HKEY, LPCWSTR, DWORD, DWORD, HKEY *);
IMP LONG WINAPI RegQueryValueExW(HKEY, LPCWSTR, DWORD *, DWORD *, BYTE *, DWORD *);
IMP LONG WINAPI RegEnumKeyExW(HKEY, DWORD, LPWSTR, DWORD *, DWORD *, LPWSTR, DWORD *, FILETIME *);
IMP LONG WINAPI RegCloseKey(HKEY);
IMP BOOL WINAPI CryptAcquireContextW(HCRYPTPROV *, LPCWSTR, LPCWSTR, DWORD, DWORD);
IMP BOOL WINAPI CryptCreateHash(HCRYPTPROV, UINT, ULONG_PTR, DWORD, HCRYPTHASH *);
IMP BOOL WINAPI CryptHashData(HCRYPTHASH, const BYTE *, DWORD, DWORD);
IMP BOOL WINAPI CryptGetHashParam(HCRYPTHASH, DWORD, BYTE *, DWORD *, DWORD);
IMP BOOL WINAPI CryptDestroyHash(HCRYPTHASH);
IMP BOOL WINAPI CryptReleaseContext(HCRYPTPROV, DWORD);
/* shell32, ole32, urlmon, comctl32 */
IMP HINSTANCE WINAPI ShellExecuteW(HWND, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR, int);
IMP LPVOID WINAPI SHBrowseForFolderW(BROWSEINFOW *);
IMP BOOL WINAPI SHGetPathFromIDListW(LPVOID, LPWSTR);
IMP HRESULT WINAPI OleInitialize(LPVOID);
IMP void WINAPI CoTaskMemFree(LPVOID);
IMP HRESULT WINAPI URLDownloadToFileW(LPVOID, LPCWSTR, LPCWSTR, DWORD, LPVOID);
IMP BOOL WINAPI InitCommonControlsEx(const INITCOMMONCONTROLSEX *);

#define INVALID_HANDLE ((HANDLE)(LONG)-1)
#define NO_ATTR 0xFFFFFFFFul
#define ATTR_DIR 0x10
#define GENERIC_READ 0x80000000ul
#define GENERIC_WRITE 0x40000000ul
#define HKCU ((HKEY)(ULONG_PTR)0x80000001ul)
#define HKLM ((HKEY)(ULONG_PTR)0x80000002ul)
#define KEY_READ 0x20019
#define WS_CHILD 0x40000000ul
#define WS_VISIBLE 0x10000000ul
#define WS_TABSTOP 0x00010000ul
#define WS_VSCROLL 0x00200000ul
#define WS_CAPTION 0x00C00000ul
#define WS_SYSMENU 0x00080000ul
#define WS_MINIMIZEBOX 0x00020000ul
#define WS_EX_CLIENTEDGE 0x200
#define ES_MULTILINE 0x4
#define ES_AUTOVSCROLL 0x40
#define ES_AUTOHSCROLL 0x80
#define ES_READONLY 0x800
#define BS_DEFPUSHBUTTON 0x1
#define BS_AUTOCHECKBOX 0x3
#define WM_CREATE 0x1
#define WM_DESTROY 0x2
#define WM_CLOSE 0x10
#define WM_SETFONT 0x30
#define WM_COMMAND 0x111
#define WM_CTLCOLORSTATIC 0x138
#define WM_GETTEXTLENGTH 0xE
#define WM_APP 0x8000
#define EM_SETSEL 0xB1
#define EM_SCROLLCARET 0xB7
#define EM_REPLACESEL 0xC2
#define BM_GETCHECK 0xF0
#define BM_SETCHECK 0xF1
#define PBM_SETPOS 0x402
#define PBM_SETRANGE32 0x406
#define MB_ICONERROR 0x10
#define MB_ICONWARNING 0x30
#define MB_ICONINFORMATION 0x40
#define MB_YESNO 0x4
#define IDYES 6
#define RT_RCDATA ((LPCWSTR)10)
#define CP_UTF8 65001
#define CREATE_NO_WINDOW 0x08000000ul

/* ---------- tiny runtime ---------- */
void *memset(void *d, int c, unsigned int n) { volatile BYTE *p = d; while (n--) *p++ = (BYTE)c; return d; }
void *memcpy(void *d, const void *s, unsigned int n) {
    volatile BYTE *a = d; const BYTE *b = s; while (n--) *a++ = *b++; return d;
}

#define MAXP 520
static void scpy(WCHAR *d, LPCWSTR s) { lstrcpynW(d, s, MAXP); }
static void scat(WCHAR *d, LPCWSTR s) { int n = lstrlenW(d); if (n < MAXP - 1) lstrcpynW(d + n, s, MAXP - n); }
static void pjoin(WCHAR *d, LPCWSTR base, LPCWSTR name) {
    int n; scpy(d, base); n = lstrlenW(d);
    if (n && d[n - 1] != '\\' && d[n - 1] != '/') scat(d, L"\\");
    scat(d, name);
}
static int file_exists(LPCWSTR p) { DWORD a = GetFileAttributesW(p); return a != NO_ATTR && !(a & ATTR_DIR); }
static int dir_exists(LPCWSTR p) { DWORD a = GetFileAttributesW(p); return a != NO_ATTR && (a & ATTR_DIR); }
static void *halloc(DWORD n) { return HeapAlloc(GetProcessHeap(), 8 /* zero */, n); }
static void hfree(void *p) { if (p) HeapFree(GetProcessHeap(), 0, p); }

/* ---------- app state ---------- */
enum { ID_PATH = 100, ID_BROWSE, ID_SKIP, ID_PS3, ID_INSTALL, ID_UNINSTALL, ID_LAUNCH, ID_HELP, ID_LOG,
       ID_PROGRESS, ID_STATUS };
enum { WM_APP_LOG = WM_APP + 1, WM_APP_PROGRESS, WM_APP_DONE };
enum { DONE_INSTALLED = 1, DONE_FAILED, DONE_REMOVED };

static HINSTANCE g_inst; static HWND g_hwnd, g_path, g_log, g_prog, g_status, g_launch;
static HWND g_busyCtl[6];
static HFONT g_font, g_fontTitle;
static int g_dpi = 96, g_busy, g_installed, g_optSkip, g_optPS3;
static WCHAR g_game[MAXP], g_tmp[MAXP], g_selfDir[MAXP];
static WCHAR *g_logText; static int g_logLen;
#define LOGCAP 65536

static const WCHAR APP_TITLE[] = L"AC1 Modern Fix Setup";
static const WCHAR HELP_URL[] = L"https://github.com/meanderpaul/AC1-ModernFix/blob/main/docs/TROUBLESHOOTING.md";
static const WCHAR GAME_EXE[] = L"AssassinsCreed_Dx9.exe";
static const WCHAR DX9_MD5[] = L"8e72c3333743780e43bc2c34bbf625f9";

typedef struct { LPCWSTR name, file, url, page, md5; } PKG;
static const PKG ASI_PKG = { L"Ultimate ASI Loader v4.68 by ThirteenAG", L"Ultimate-ASI-Loader.zip",
    L"https://github.com/ThirteenAG/Ultimate-ASI-Loader/releases/download/v4.68/Ultimate-ASI-Loader.zip",
    L"github.com/ThirteenAG/Ultimate-ASI-Loader/releases/tag/v4.68", L"f185127bd44fe3dd55cd176dc4c007cb" };
static const PKG EP_PKG = { L"EaglePatch v1.1 by Sergeanur", L"EaglePatchAC1.zip",
    L"https://github.com/Sergeanur/EaglePatch/releases/download/v1.1/EaglePatchAC1.zip",
    L"github.com/Sergeanur/EaglePatch/releases/tag/v1.1", L"a5dce4980c3a47c924157ec6c4cef9a2" };

static int S(int v) { return MulDiv(v, g_dpi, 96); }

/* ---------- logging (any thread) ---------- */
static void append_log(LPCWSTR line) {           /* UI thread only */
    int n = lstrlenW(line), len = (int)SendMessageW(g_log, WM_GETTEXTLENGTH, 0, 0);
    SendMessageW(g_log, EM_SETSEL, (WPARAM)len, len);
    SendMessageW(g_log, EM_REPLACESEL, 0, (LPARAM)line);
    SendMessageW(g_log, EM_REPLACESEL, 0, (LPARAM)L"\r\n");
    SendMessageW(g_log, EM_SCROLLCARET, 0, 0);
    if (g_logText && g_logLen + n + 3 < LOGCAP) {
        lstrcpynW(g_logText + g_logLen, line, n + 1); g_logLen += n;
        lstrcpynW(g_logText + g_logLen, L"\r\n", 3); g_logLen += 2;
    }
}
static void logf(LPCWSTR fmt, ...) {
    WCHAR buf[1100]; __builtin_va_list ap;
    __builtin_va_start(ap, fmt); wvsprintfW(buf, fmt, ap); __builtin_va_end(ap);
    SendMessageW(g_hwnd, WM_APP_LOG, 0, (LPARAM)buf);
}
static void progress(int pct) { PostMessageW(g_hwnd, WM_APP_PROGRESS, (WPARAM)pct, 0); }

/* ---------- files & hashes ---------- */
static int md5_file(LPCWSTR path, WCHAR *hex) {
    static const char digits[] = "0123456789abcdef";
    HCRYPTPROV prov = 0; HCRYPTHASH h = 0; BYTE dig[16]; DWORD dl = 16, got, i; int ok = 0;
    BYTE *buf; HANDLE f = CreateFileW(path, GENERIC_READ, 1 /* share read */, 0, 3 /* OPEN_EXISTING */, 0, 0);
    if (f == INVALID_HANDLE) return 0;
    buf = halloc(65536);
    if (buf && CryptAcquireContextW(&prov, 0, 0, 1 /* PROV_RSA_FULL */, 0xF0000000ul /* VERIFYCONTEXT */) &&
        CryptCreateHash(prov, 0x8003 /* CALG_MD5 */, 0, 0, &h)) {
        ok = 1;
        while (ReadFile(f, buf, 65536, &got, 0) && got)
            if (!CryptHashData(h, buf, got, 0)) { ok = 0; break; }
        if (ok && CryptGetHashParam(h, 2 /* HP_HASHVAL */, dig, &dl, 0) && dl == 16) {
            for (i = 0; i < 16; i++) { hex[i * 2] = digits[dig[i] >> 4]; hex[i * 2 + 1] = digits[dig[i] & 15]; }
            hex[32] = 0;
        } else ok = 0;
    }
    if (h) CryptDestroyHash(h);
    if (prov) CryptReleaseContext(prov, 0);
    hfree(buf); CloseHandle(f);
    return ok;
}
static int md5_is(LPCWSTR path, LPCWSTR want) { WCHAR hex[40]; return md5_file(path, hex) && !lstrcmpiW(hex, want); }
static int same_file(LPCWSTR a, LPCWSTR b) {
    WCHAR ha[40], hb[40]; return md5_file(a, ha) && md5_file(b, hb) && !lstrcmpiW(ha, hb);
}

/* Reads a UTF-8 text file (up to 1 MB) into a new UTF-16 buffer. Free with hfree. */
static WCHAR *read_text(LPCWSTR path) {
    HANDLE f = CreateFileW(path, GENERIC_READ, 3, 0, 3, 0, 0); DWORD n, got = 0; char *raw; WCHAR *w = 0; int wl;
    if (f == INVALID_HANDLE) return 0;
    n = GetFileSize(f, 0);
    if (n && n < 1048576 && (raw = halloc(n + 1))) {
        if (ReadFile(f, raw, n, &got, 0) && got == n && (wl = MultiByteToWideChar(CP_UTF8, 0, raw, (int)n, 0, 0)) > 0 &&
            (w = halloc((DWORD)(wl + 1) * 2)))
            MultiByteToWideChar(CP_UTF8, 0, raw, (int)n, w, wl);
        hfree(raw);
    }
    CloseHandle(f);
    return w;
}
static int write_res(int id, LPCWSTR path) {
    HRSRC r = FindResourceW(0, (LPCWSTR)(ULONG_PTR)id, RT_RCDATA); HGLOBAL g; void *data; DWORD n, w = 0; HANDLE f; int ok;
    if (!r || !(g = LoadResource(0, r)) || !(data = LockResource(g))) return 0;
    n = SizeofResource(0, r);
    f = CreateFileW(path, GENERIC_WRITE, 0, 0, 2 /* CREATE_ALWAYS */, 0x80, 0);
    if (f == INVALID_HANDLE) return 0;
    ok = WriteFile(f, data, n, &w, 0) && w == n;
    CloseHandle(f);
    return ok;
}
static int find_file(LPCWSTR dir, LPCWSTR name, WCHAR *out, int depth) {
    WCHAR pat[MAXP], sub[MAXP]; WIN32_FIND_DATAW fd; HANDLE h; int found = 0;
    pjoin(out, dir, name);
    if (file_exists(out)) return 1;
    if (depth <= 0) return 0;
    pjoin(pat, dir, L"*");
    if ((h = FindFirstFileW(pat, &fd)) == INVALID_HANDLE) return 0;
    do {
        if ((fd.dwFileAttributes & ATTR_DIR) && fd.cFileName[0] != '.') {
            pjoin(sub, dir, fd.cFileName);
            if (find_file(sub, name, out, depth - 1)) { found = 1; break; }
        }
    } while (FindNextFileW(h, &fd));
    FindClose(h);
    return found;
}
static int run(WCHAR *cmd, DWORD *exitCode) {
    STARTUPINFOW si; PROCESS_INFORMATION pi;
    memset(&si, 0, sizeof si); si.cb = sizeof si; memset(&pi, 0, sizeof pi);
    if (!CreateProcessW(0, cmd, 0, 0, 0, CREATE_NO_WINDOW, 0, 0, &si, &pi)) return 0;
    WaitForSingleObject(pi.hProcess, 0xFFFFFFFFul);
    *exitCode = 1; GetExitCodeProcess(pi.hProcess, exitCode);
    CloseHandle(pi.hThread); CloseHandle(pi.hProcess);
    return 1;
}

/* ---------- finding the game ---------- */
static int is_game_dir(LPCWSTR d) { WCHAR p[MAXP]; pjoin(p, d, GAME_EXE); return d[0] && file_exists(p); }

static int reg_str(HKEY root, LPCWSTR sub, LPCWSTR val, WCHAR *out) {
    HKEY k; DWORD type = 0, sz = (MAXP - 1) * 2; LONG r;
    out[0] = 0;
    if (RegOpenKeyExW(root, sub, 0, KEY_READ, &k)) return 0;
    r = RegQueryValueExW(k, val, 0, &type, (BYTE *)out, &sz);
    RegCloseKey(k);
    if (r || (type != 1 && type != 2)) { out[0] = 0; return 0; }
    out[sz / 2] = 0;
    return out[0] != 0;
}
static int wfind(LPCWSTR hay, LPCWSTR needle, int from) {
    int i, j;
    for (i = from; hay[i]; i++) {
        for (j = 0; needle[j] && hay[i + j] == needle[j]; j++) {}
        if (!needle[j]) return i;
    }
    return -1;
}
/* Finds "key" in Valve KeyValues text from position `from`, copies its quoted value to out.
   Returns the position after the value, or -1. */
static int vdf_value(LPCWSTR t, LPCWSTR key, int from, WCHAR *out) {
    int i = wfind(t, key, from), n = 0;
    if (i < 0) return -1;
    i += lstrlenW(key);
    while (t[i] && t[i] != '"') i++;
    if (!t[i]) return -1;
    for (i++; t[i] && t[i] != '"' && n < MAXP - 1; i++) {
        if (t[i] == '\\' && t[i + 1]) i++;
        out[n++] = t[i];
    }
    out[n] = 0;
    return i;
}
static int try_steam_lib(LPCWSTR lib, WCHAR *out) {
    WCHAR p[MAXP], dir[MAXP]; WCHAR *t;
    pjoin(p, lib, L"steamapps\\appmanifest_15100.acf");
    if ((t = read_text(p))) {
        if (vdf_value(t, L"\"installdir\"", 0, dir) > 0) {
            pjoin(p, lib, L"steamapps\\common"); pjoin(out, p, dir);
            if (is_game_dir(out)) { hfree(t); return 1; }
        }
        hfree(t);
    }
    pjoin(out, lib, L"steamapps\\common\\Assassins Creed");
    return is_game_dir(out);
}
static int find_steam(WCHAR *out) {
    WCHAR steam[MAXP], p[MAXP], lib[MAXP]; WCHAR *t; int i, pos = 0, found = 0;
    if (!reg_str(HKCU, L"Software\\Valve\\Steam", L"SteamPath", steam) &&
        !reg_str(HKLM, L"SOFTWARE\\Valve\\Steam", L"InstallPath", steam)) return 0;
    for (i = 0; steam[i]; i++) if (steam[i] == '/') steam[i] = '\\';
    if (try_steam_lib(steam, out)) return 1;
    pjoin(p, steam, L"steamapps\\libraryfolders.vdf");
    if (!(t = read_text(p))) return 0;
    while ((pos = vdf_value(t, L"\"path\"", pos, lib)) > 0)
        if (try_steam_lib(lib, out)) { found = 1; break; }
    hfree(t);
    return found;
}
static int find_gog(WCHAR *out) {
    static const WCHAR base[] = L"SOFTWARE\\GOG.com\\Games";
    HKEY k; WCHAR name[128], sub[MAXP]; DWORD i, n; int found = 0;
    if (RegOpenKeyExW(HKLM, base, 0, KEY_READ, &k)) return 0;
    for (i = 0; !found; i++) {
        n = 128;
        if (RegEnumKeyExW(k, i, name, &n, 0, 0, 0, 0)) break;
        pjoin(sub, base, name);
        if (reg_str(HKLM, sub, L"path", out) && is_game_dir(out)) found = 1;
    }
    RegCloseKey(k);
    return found;
}
/* Returns 1 for Steam, 2 for GOG, 3 for a common folder, 0 if not found. */
static int find_game(WCHAR *out) {
    static LPCWSTR common[] = { L"C:\\Program Files (x86)\\Steam\\steamapps\\common\\Assassins Creed",
                                L"C:\\GOG Games\\Assassin's Creed",
                                L"C:\\Program Files (x86)\\GOG Galaxy\\Games\\Assassin's Creed" };
    int i;
    if (find_steam(out)) return 1;
    if (find_gog(out)) return 2;
    for (i = 0; i < 3; i++) { scpy(out, common[i]); if (is_game_dir(out)) return 3; }
    out[0] = 0;
    return 0;
}

/* ---------- install steps (worker thread) ---------- */
/* Puts a verified copy of pkg in `zip`: one the user already downloaded, or a fresh download. */
static int fetch(const PKG *pkg, WCHAR *zip) {
    WCHAR dl[MAXP]; HRESULT hr;
    pjoin(zip, g_selfDir, pkg->file);
    if (file_exists(zip) && md5_is(zip, pkg->md5)) { logf(L"  Using %s found next to the installer.", pkg->file); return 1; }
    if (GetEnvironmentVariableW(L"USERPROFILE", dl, MAXP)) {
        scat(dl, L"\\Downloads"); pjoin(zip, dl, pkg->file);
        if (file_exists(zip) && md5_is(zip, pkg->md5)) { logf(L"  Using %s from your Downloads folder.", pkg->file); return 1; }
    }
    pjoin(zip, g_tmp, pkg->file);
    DeleteFileW(zip);
    logf(L"  Downloading from %s ...", pkg->page);
    hr = URLDownloadToFileW(0, pkg->url, zip, 0, 0);
    if (hr) {
        logf(L"[X] Download failed (error 0x%08lX). Check your internet connection.", (unsigned long)hr);
        logf(L"    Or download %s yourself from %s and put it next to this installer.", pkg->file, pkg->page);
        return 0;
    }
    if (!md5_is(zip, pkg->md5)) {
        logf(L"[X] The downloaded %s isn't the expected version, so it wasn't installed.", pkg->file);
        logf(L"    Get it yourself from %s and put it next to this installer.", pkg->page);
        return 0;
    }
    return 1;
}
static int extract(LPCWSTR zip, LPCWSTR dest) {
    static WCHAR cmd[MAXP * 3]; WCHAR exe[MAXP], esc[MAXP * 2]; DWORD code; int i, n;
    CreateDirectoryW(dest, 0);
    /* Windows 10+ ships bsdtar, which reads zip. A 32-bit process reaches System32 via Sysnative. */
    GetWindowsDirectoryW(exe, MAXP); scat(exe, L"\\Sysnative\\tar.exe");
    if (!file_exists(exe)) { GetSystemDirectoryW(exe, MAXP); scat(exe, L"\\tar.exe"); }
    if (file_exists(exe)) {
        scpy(cmd, L"\""); scat(cmd, exe); scat(cmd, L"\" -xf \""); scat(cmd, zip);
        scat(cmd, L"\" -C \""); scat(cmd, dest); scat(cmd, L"\"");
        if (run(cmd, &code) && code == 0) return 1;
    }
    /* Fallback: PowerShell's Expand-Archive. Single quotes are doubled for PowerShell. */
    for (i = 0, n = 0; zip[i] && n < MAXP * 2 - 2; i++) { if (zip[i] == '\'') esc[n++] = '\''; esc[n++] = zip[i]; }
    esc[n] = 0;
    lstrcpynW(cmd, L"powershell.exe -NoProfile -NonInteractive -Command \"Expand-Archive -Force -LiteralPath '", MAXP * 3);
    { int l = lstrlenW(cmd); lstrcpynW(cmd + l, esc, MAXP * 3 - l); }
    for (i = 0, n = 0; dest[i] && n < MAXP * 2 - 2; i++) { if (dest[i] == '\'') esc[n++] = '\''; esc[n++] = dest[i]; }
    esc[n] = 0;
    { int l = lstrlenW(cmd); lstrcpynW(cmd + l, L"' -DestinationPath '", MAXP * 3 - l); }
    { int l = lstrlenW(cmd); lstrcpynW(cmd + l, esc, MAXP * 3 - l); }
    { int l = lstrlenW(cmd); lstrcpynW(cmd + l, L"'\"", MAXP * 3 - l); }
    return run(cmd, &code) && code == 0;
}
static int get_file(const PKG *pkg, LPCWSTR sub, LPCWSTR name, WCHAR *found) {
    WCHAR zip[MAXP], dest[MAXP];
    logf(L"Getting %s", pkg->name);
    if (!fetch(pkg, zip)) return 0;
    pjoin(dest, g_tmp, sub);
    if (!extract(zip, dest)) { logf(L"[X] Couldn't unzip %s.", pkg->file); return 0; }
    if (!find_file(dest, name, found, 3)) { logf(L"[X] %s wasn't in %s.", name, pkg->file); return 0; }
    return 1;
}
static int copy_to(LPCWSTR from, LPCWSTR to) {
    if (CopyFileW(from, to, 0)) return 1;
    logf(L"[X] Couldn't write %s. Close the game and try again.", to);
    return 0;
}
static int xinput_present(void) { WCHAR p[MAXP]; GetSystemDirectoryW(p, MAXP); scat(p, L"\\xinput1_3.dll"); return file_exists(p); }

static int do_install(void) {
    WCHAR scripts[MAXP], dst[MAXP], src[MAXP], bak[MAXP], dir[MAXP]; static WCHAR cmd[MAXP + 16]; DWORD code; int i;
    pjoin(scripts, g_game, L"scripts");
    CreateDirectoryW(scripts, 0);
    if (!dir_exists(scripts)) { logf(L"[X] Couldn't create %s", scripts); return 0; }
    progress(5);

    /* 1. Ultimate ASI Loader -> dinput8.dll in the game folder */
    if (!get_file(&ASI_PKG, L"asi", L"dinput8.dll", src)) return 0;
    pjoin(dst, g_game, L"dinput8.dll"); pjoin(bak, g_game, L"dinput8.dll.bak");
    if (file_exists(dst) && !same_file(dst, src) && !file_exists(bak)) {
        CopyFileW(dst, bak, 1);
        logf(L"  Backed up your existing dinput8.dll to dinput8.dll.bak");
    }
    if (!copy_to(src, dst)) return 0;
    logf(L"[OK] Ultimate ASI Loader installed");
    progress(35);

    /* 2. EaglePatch -> scripts\ (keeps the user's existing ini) */
    if (!get_file(&EP_PKG, L"eaglepatch", L"EaglePatchAC1.asi", src)) return 0;
    pjoin(dst, scripts, L"EaglePatchAC1.asi");
    if (!copy_to(src, dst)) return 0;
    for (i = lstrlenW(src); i > 0 && src[i - 1] != '\\'; i--) {}
    lstrcpynW(dir, src, i + 1);
    pjoin(src, dir, L"EaglePatchAC1.ini"); pjoin(dst, scripts, L"EaglePatchAC1.ini");
    if (!file_exists(dst) && !copy_to(src, dst)) return 0;
    logf(L"[OK] EaglePatch installed (Xbox controller support, no Ubisoft server crash)");
    progress(65);

    /* 3. AC1AffinityFix, embedded in this exe (keeps the user's existing ini) */
    pjoin(dst, scripts, L"AC1AffinityFix.asi");
    if (!write_res(101, dst)) { logf(L"[X] Couldn't write %s. Close the game and try again.", dst); return 0; }
    pjoin(dst, scripts, L"AC1AffinityFix.ini");
    if (!file_exists(dst) && !write_res(102, dst)) { logf(L"[X] Couldn't write %s", dst); return 0; }
    logf(L"[OK] AC1AffinityFix installed (many-core CPU crash fix)");
    progress(75);

    /* 4. Options */
    pjoin(dst, scripts, L"EaglePatchAC1.ini");
    WritePrivateProfileStringW(L"EaglePatchAC1", L"SkipIntroVideos", g_optSkip ? L"1" : L"0", dst);
    WritePrivateProfileStringW(L"EaglePatchAC1", L"PS3Controls", g_optPS3 ? L"1" : L"0", dst);
    logf(L"[OK] Settings saved: skip intro videos %s, PS3-style controls %s",
         g_optSkip ? L"on" : L"off", g_optPS3 ? L"on" : L"off");
    progress(80);

    /* 5. xinput1_3.dll from the DirectX June 2010 runtime */
    if (xinput_present()) logf(L"[OK] DirectX controller runtime (xinput1_3.dll) found");
    else {
        pjoin(dst, g_game, L"DirectX\\DXSETUP.exe");
        if (file_exists(dst)) {
            logf(L"  Installing the DirectX runtime the game needs. This can take a minute...");
            scpy(cmd, L"\""); scat(cmd, dst); scat(cmd, L"\" /silent");
            if (!run(cmd, &code)) logf(L"[!] Couldn't start DirectX\\DXSETUP.exe");
        }
        if (xinput_present()) logf(L"[OK] DirectX controller runtime installed");
        else logf(L"[!] xinput1_3.dll is still missing. Install Microsoft's \"DirectX End-User Runtimes (June 2010)\".");
    }
    progress(100);
    return 1;
}
static int do_uninstall(void) {
    static LPCWSTR files[] = { L"scripts\\AC1AffinityFix.asi", L"scripts\\AC1AffinityFix.ini",
                               L"scripts\\EaglePatchAC1.asi", L"scripts\\EaglePatchAC1.ini", L"dinput8.dll" };
    WCHAR p[MAXP], bak[MAXP]; int i;
    for (i = 0; i < 5; i++) {
        pjoin(p, g_game, files[i]);
        if (file_exists(p) && !DeleteFileW(p)) { logf(L"[X] Couldn't remove %s. Close the game and try again.", p); return 0; }
        progress(15 * (i + 1));
    }
    pjoin(p, g_game, L"dinput8.dll"); pjoin(bak, g_game, L"dinput8.dll.bak");
    if (file_exists(bak) && MoveFileExW(bak, p, 1)) logf(L"  Restored your previous dinput8.dll");
    pjoin(p, g_game, L"scripts");
    if (dir_exists(p) && !RemoveDirectoryW(p)) logf(L"  Left the scripts folder in place because other mods are in it.");
    progress(100);
    logf(L"[OK] AC1 Modern Fix removed. Your game files are back to normal.");
    return 1;
}
static DWORD WINAPI install_thread(LPVOID uninstall) {
    int ok;
    OleInitialize(0); /* URLDownloadToFileW uses COM */
    ok = uninstall ? do_uninstall() : do_install();
    PostMessageW(g_hwnd, WM_APP_DONE, ok ? (uninstall ? DONE_REMOVED : DONE_INSTALLED) : DONE_FAILED, 0);
    return 0;
}

/* ---------- UI ---------- */
static void save_log(void) {
    WCHAR p[MAXP]; char *u; int n; DWORD w; HANDLE f;
    if (!g_logText || !g_game[0]) return;
    pjoin(p, g_game, L"AC1ModernFix-install.log");
    n = WideCharToMultiByte(CP_UTF8, 0, g_logText, g_logLen, 0, 0, 0, 0);
    if (n <= 0 || !(u = halloc((DWORD)n))) return;
    WideCharToMultiByte(CP_UTF8, 0, g_logText, g_logLen, u, n, 0, 0);
    f = CreateFileW(p, GENERIC_WRITE, 0, 0, 2, 0x80, 0);
    if (f != INVALID_HANDLE) { WriteFile(f, u, (DWORD)n, &w, 0); CloseHandle(f); }
    hfree(u);
}
static void set_busy(int busy) {
    int i;
    g_busy = busy;
    for (i = 0; i < 6; i++) EnableWindow(g_busyCtl[i], !busy);
    EnableWindow(g_launch, !busy && g_installed);
}
static void check_installed(void) {
    WCHAR p[MAXP]; pjoin(p, g_game, L"scripts\\AC1AffinityFix.asi");
    g_installed = is_game_dir(g_game) && file_exists(p);
    EnableWindow(g_launch, g_installed && !g_busy);
}
static void read_path(void) {
    int n; GetWindowTextW(g_path, g_game, MAXP);
    n = lstrlenW(g_game);
    while (n > 3 && (g_game[n - 1] == '\\' || g_game[n - 1] == ' ' || g_game[n - 1] == '"')) g_game[--n] = 0;
    if (g_game[0] == '"') { lstrcpynW(g_game, g_game + 1, MAXP); }
}
static void report_folder(void) {
    if (!is_game_dir(g_game)) {
        logf(L"[!] %s isn't in that folder. Click Browse and pick the folder that has it.", GAME_EXE);
        g_installed = 0; EnableWindow(g_launch, 0); return;
    }
    check_installed();
    {   /* show the settings already in the game's EaglePatch ini, so reinstalling keeps them */
        WCHAR ini[MAXP]; pjoin(ini, g_game, L"scripts\\EaglePatchAC1.ini");
        if (file_exists(ini)) {
            SendMessageW(g_busyCtl[3], BM_SETCHECK, GetPrivateProfileIntW(L"EaglePatchAC1", L"SkipIntroVideos", 0, ini) == 1, 0);
            SendMessageW(g_busyCtl[4], BM_SETCHECK, GetPrivateProfileIntW(L"EaglePatchAC1", L"PS3Controls", 0, ini) == 1, 0);
        }
    }
    if (g_installed) logf(L"AC1 Modern Fix is already installed here. Install again to repair it, or Uninstall to remove it.");
    else logf(L"Ready. Close the game if it's running, then click Install.");
}
static void on_browse(void) {
    BROWSEINFOW bi; WCHAR name[MAXP], path[MAXP]; LPVOID pidl;
    memset(&bi, 0, sizeof bi);
    bi.hwndOwner = g_hwnd; bi.pszDisplayName = name;
    bi.lpszTitle = L"Pick your Assassin's Creed folder (the one with AssassinsCreed_Dx9.exe)";
    bi.ulFlags = 0x1 | 0x40; /* RETURNONLYFSDIRS | NEWDIALOGSTYLE */
    if (!(pidl = SHBrowseForFolderW(&bi))) return;
    if (SHGetPathFromIDListW(pidl, path)) { SetWindowTextW(g_path, path); read_path(); report_folder(); }
    CoTaskMemFree(pidl);
}
static void start_work(int uninstall) {
    WCHAR hex[40]; HANDLE t;
    read_path();
    if (!is_game_dir(g_game)) {
        MessageBoxW(g_hwnd, L"That folder doesn't contain AssassinsCreed_Dx9.exe.\n\nClick Browse and pick your Assassin's Creed folder. "
                    L"In Steam: right-click the game > Manage > Browse local files.", APP_TITLE, MB_ICONWARNING);
        return;
    }
    if (uninstall) {
        if (MessageBoxW(g_hwnd, L"Remove AC1 Modern Fix, EaglePatch and the ASI Loader from this game?",
                        APP_TITLE, MB_YESNO | MB_ICONINFORMATION) != IDYES) return;
    } else {
        WCHAR exe[MAXP]; pjoin(exe, g_game, GAME_EXE);
        if (md5_file(exe, hex) && lstrcmpiW(hex, DX9_MD5) &&
            MessageBoxW(g_hwnd, L"Your AssassinsCreed_Dx9.exe has been modified, or it's a version EaglePatch doesn't support "
                        L"(for example the Ubisoft Connect version).\n\nThe CPU crash fix will still work, but Xbox controller "
                        L"support won't load until you restore the original exe. In Steam: right-click the game > Properties > "
                        L"Installed Files > Verify integrity of game files.\n\nInstall anyway?",
                        APP_TITLE, MB_YESNO | MB_ICONWARNING) != IDYES) return;
        g_optSkip = SendMessageW(g_busyCtl[3], BM_GETCHECK, 0, 0) == 1;
        g_optPS3 = SendMessageW(g_busyCtl[4], BM_GETCHECK, 0, 0) == 1;
    }
    SendMessageW(g_prog, PBM_SETPOS, 0, 0);
    SetWindowTextW(g_status, uninstall ? L"Removing..." : L"Installing...");
    logf(L"");
    logf(uninstall ? L"Uninstalling from %s" : L"Installing to %s", g_game);
    set_busy(1);
    t = CreateThread(0, 262144, install_thread, (LPVOID)(ULONG_PTR)uninstall, 0, 0);
    if (t) CloseHandle(t); else { set_busy(0); logf(L"[X] Couldn't start the installer thread."); }
}
/* Opens a URL or starts a program through Explorer so it doesn't inherit this installer's admin rights. */
static void open_unelevated(LPCWSTR target) {
    WCHAR q[MAXP + 4]; scpy(q, L"\""); scat(q, target); scat(q, L"\"");
    ShellExecuteW(g_hwnd, L"open", L"explorer.exe", q, 0, 1);
}
static void on_launch(void) {
    WCHAR exe[MAXP]; WCHAR low[MAXP]; int i;
    scpy(low, g_game);
    for (i = 0; low[i]; i++) if (low[i] >= 'A' && low[i] <= 'Z') low[i] += 32;
    if (wfind(low, L"\\steamapps\\common\\", 0) >= 0) open_unelevated(L"steam://rungameid/15100");
    else { pjoin(exe, g_game, GAME_EXE); ShellExecuteW(g_hwnd, L"open", exe, 0, g_game, 1); }
}

static HWND ctl(LPCWSTR cls, LPCWSTR text, DWORD style, int x, int y, int w, int h, int id, DWORD ex) {
    HWND c = CreateWindowExW(ex, cls, text, WS_CHILD | WS_VISIBLE | style, S(x), S(y), S(w), S(h), g_hwnd,
                             (HMENU)(ULONG_PTR)id, g_inst, 0);
    SendMessageW(c, WM_SETFONT, (WPARAM)g_font, 1);
    return c;
}
static void on_create(void) {
    HWND title; int src;
    title = ctl(L"STATIC", L"AC1 Modern Fix", 0, 20, 14, 520, 30, 0, 0);
    SendMessageW(title, WM_SETFONT, (WPARAM)g_fontTitle, 1);
    ctl(L"STATIC", L"Fixes the launch crash and Xbox controller support in Assassin's Creed: Director's Cut (Steam / GOG).",
        0, 20, 46, 520, 20, 0, 0);
    ctl(L"STATIC", L"Game folder:", 0, 20, 78, 300, 18, 0, 0);
    g_path = g_busyCtl[0] = ctl(L"EDIT", L"", WS_TABSTOP | ES_AUTOHSCROLL, 20, 98, 420, 24, ID_PATH, WS_EX_CLIENTEDGE);
    g_busyCtl[1] = ctl(L"BUTTON", L"Browse...", WS_TABSTOP, 448, 97, 92, 26, ID_BROWSE, 0);
    g_busyCtl[3] = ctl(L"BUTTON", L"Skip intro videos", WS_TABSTOP | BS_AUTOCHECKBOX, 20, 132, 180, 22, ID_SKIP, 0);
    g_busyCtl[4] = ctl(L"BUTTON", L"PS3-style controls (RB = run)", WS_TABSTOP | BS_AUTOCHECKBOX, 210, 132, 250, 22, ID_PS3, 0);
    SendMessageW(g_busyCtl[3], BM_SETCHECK, 1, 0);
    g_busyCtl[2] = ctl(L"BUTTON", L"Install", WS_TABSTOP | BS_DEFPUSHBUTTON, 20, 166, 130, 32, ID_INSTALL, 0);
    g_busyCtl[5] = ctl(L"BUTTON", L"Uninstall", WS_TABSTOP, 160, 166, 110, 32, ID_UNINSTALL, 0);
    ctl(L"BUTTON", L"Help", WS_TABSTOP, 280, 166, 80, 32, ID_HELP, 0);
    g_launch = ctl(L"BUTTON", L"Launch game", WS_TABSTOP, 410, 166, 130, 32, ID_LAUNCH, 0);
    g_prog = ctl(L"msctls_progress32", L"", 0, 20, 212, 520, 14, ID_PROGRESS, 0);
    SendMessageW(g_prog, PBM_SETRANGE32, 0, 100);
    g_status = ctl(L"STATIC", L"", 0, 20, 232, 520, 18, ID_STATUS, 0);
    g_log = ctl(L"EDIT", L"", WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY, 20, 254, 520, 150, ID_LOG,
                WS_EX_CLIENTEDGE);
    ctl(L"STATIC", L"By Meanderpaul_gaming. Uses EaglePatch by Sergeanur and Ultimate ASI Loader by ThirteenAG.",
        0, 20, 414, 520, 18, 0, 0);
    EnableWindow(g_launch, 0);

    src = find_game(g_game);
    if (src) {
        SetWindowTextW(g_path, g_game);
        logf(L"Found Assassin's Creed (%s): %s", src == 1 ? L"Steam" : src == 2 ? L"GOG" : L"common folder", g_game);
        report_folder();
    } else logf(L"Couldn't find the game automatically. Click Browse and pick your Assassin's Creed folder.");
}
static LRESULT WINAPI wndproc(HWND h, UINT m, WPARAM w, LPARAM l) {
    switch (m) {
    case WM_CREATE: g_hwnd = h; on_create(); return 0;
    case WM_COMMAND:
        switch (w & 0xFFFF) {
        case ID_BROWSE: on_browse(); return 0;
        case ID_INSTALL: start_work(0); return 0;
        case ID_UNINSTALL: start_work(1); return 0;
        case ID_LAUNCH: on_launch(); return 0;
        case ID_HELP: open_unelevated(HELP_URL); return 0;
        }
        break;
    case WM_APP_LOG: append_log((LPCWSTR)l); return 0;
    case WM_APP_PROGRESS: SendMessageW(g_prog, PBM_SETPOS, w, 0); return 0;
    case WM_APP_DONE:
        set_busy(0);
        check_installed();
        if (w == DONE_INSTALLED) {
            logf(L"All done! Plug in your controller and click Launch game. Choose DirectX 9 if Steam asks.");
            SetWindowTextW(g_status, L"Installed. Click Launch game to play.");
            save_log();
        } else if (w == DONE_REMOVED) SetWindowTextW(g_status, L"Removed.");
        else {
            SetWindowTextW(g_status, L"Something went wrong. See the details below, or click Help.");
            save_log();
        }
        return 0;
    case WM_CTLCOLORSTATIC:
        if ((HWND)l == g_log) { SetBkColor((HDC)w, 0xFFFFFF); SetTextColor((HDC)w, 0x202020); return (LRESULT)GetStockObject(0); }
        break;
    case WM_CLOSE:
        if (g_busy) { MessageBoxW(h, L"Still working. Please wait a moment.", APP_TITLE, MB_ICONINFORMATION); return 0; }
        DestroyWindow(h); return 0;
    case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    return DefWindowProcW(h, m, w, l);
}

void entry(void) {
    static const WCHAR cls[] = L"AC1ModernFixSetup";
    WNDCLASSEXW wc; INITCOMMONCONTROLSEX icc = { sizeof(INITCOMMONCONTROLSEX), 0x20 | 0x4000 };
    RECT rc; MSG msg; HDC dc; DWORD style = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX; int i, cw, ch;

    g_inst = GetModuleHandleW(0);
    OleInitialize(0);
    InitCommonControlsEx(&icc);
    dc = GetDC(0); g_dpi = GetDeviceCaps(dc, 90 /* LOGPIXELSY */); ReleaseDC(0, dc);
    if (g_dpi < 96) g_dpi = 96;
    g_font = CreateFontW(-MulDiv(9, g_dpi, 72), 0, 0, 0, 400, 0, 0, 0, 1, 0, 0, 5, 0, L"Segoe UI");
    g_fontTitle = CreateFontW(-MulDiv(15, g_dpi, 72), 0, 0, 0, 700, 0, 0, 0, 1, 0, 0, 5, 0, L"Segoe UI");
    g_logText = halloc(LOGCAP * 2);

    GetModuleFileNameW(0, g_selfDir, MAXP);
    for (i = lstrlenW(g_selfDir); i > 0 && g_selfDir[i - 1] != '\\'; i--) {}
    g_selfDir[i] = 0;
    GetTempPathW(MAXP, g_tmp); scat(g_tmp, L"AC1ModernFix");
    CreateDirectoryW(g_tmp, 0);

    memset(&wc, 0, sizeof wc);
    wc.cbSize = sizeof wc; wc.lpfnWndProc = wndproc; wc.hInstance = g_inst;
    wc.hIcon = wc.hIconSm = LoadIconW(g_inst, (LPCWSTR)1);
    wc.hCursor = LoadCursorW(0, (LPCWSTR)32512);
    wc.hbrBackground = (HBRUSH)(ULONG_PTR)16; /* COLOR_BTNFACE + 1 */
    wc.lpszClassName = cls;
    RegisterClassExW(&wc);

    rc.left = rc.top = 0; rc.right = S(560); rc.bottom = S(444);
    AdjustWindowRect(&rc, style, 0);
    cw = rc.right - rc.left; ch = rc.bottom - rc.top;
    CreateWindowExW(0, cls, APP_TITLE, style, (GetSystemMetrics(0) - cw) / 2, (GetSystemMetrics(1) - ch) / 2,
                    cw, ch, 0, 0, g_inst, 0);
    ShowWindow(g_hwnd, 1); UpdateWindow(g_hwnd);
    while (GetMessageW(&msg, 0, 0, 0) > 0)
        if (!IsDialogMessageW(g_hwnd, &msg)) { TranslateMessage(&msg); DispatchMessageW(&msg); }
    ExitProcess((UINT)msg.wParam);
}
