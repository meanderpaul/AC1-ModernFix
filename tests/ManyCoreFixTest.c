/* ManyCoreFixTest.exe - automated check for ManyCoreFix.asi, run on a Windows CI machine.
   Stands in for a game: it imports GetSystemInfo etc. like a game exe does, loads the plugin,
   then checks the plugin limited the affinity and the CPU count this exe sees.
   Usage: ManyCoreFixTest.exe <plugin.asi> <expected MaxCPUs> [affinity]
   With "affinity", only the affinity limit is checked (for AC1AffinityFix, which doesn't spoof).
   Exit code 0 = pass. No C runtime. Copyright (c) 2026 Meanderpaul_gaming. MIT License. */

typedef int BOOL; typedef unsigned long DWORD; typedef unsigned short WORD; typedef unsigned int UINT;
typedef __WCHAR_TYPE__ WCHAR; typedef __SIZE_TYPE__ SIZE_T; typedef SIZE_T DWORD_PTR;
typedef void *HANDLE, *HMODULE, *LPVOID; typedef const WCHAR *LPCWSTR;
#define WINAPI __stdcall
#define IMP __declspec(dllimport)
typedef struct {
    WORD wProcessorArchitecture, wReserved; DWORD dwPageSize;
    LPVOID lpMinimumApplicationAddress, lpMaximumApplicationAddress; DWORD_PTR dwActiveProcessorMask;
    DWORD dwNumberOfProcessors, dwProcessorType, dwAllocationGranularity; WORD wProcessorLevel, wProcessorRevision;
} SYSTEM_INFO;

IMP void WINAPI GetSystemInfo(SYSTEM_INFO *);
IMP void WINAPI GetNativeSystemInfo(SYSTEM_INFO *);
IMP DWORD WINAPI GetActiveProcessorCount(WORD);
IMP HANDLE WINAPI GetCurrentProcess(void);
IMP BOOL WINAPI GetProcessAffinityMask(HANDLE, DWORD_PTR *, DWORD_PTR *);
IMP HMODULE WINAPI LoadLibraryW(LPCWSTR);
IMP HANDLE WINAPI GetStdHandle(DWORD);
IMP BOOL WINAPI WriteFile(HANDLE, const void *, DWORD, DWORD *, LPVOID);
IMP LPCWSTR WINAPI GetCommandLineW(void);
IMP void WINAPI ExitProcess(UINT);

static int g_fail;
static void out(const char *s) { DWORD n = 0, w; while (s[n]) n++; WriteFile(GetStdHandle((DWORD)-11), s, n, &w, 0); }
static void outu(DWORD v) { char b[12]; int i = 11; b[i] = 0; do { b[--i] = (char)('0' + v % 10); v /= 10; } while (v); out(b + i); }
static void check(const char *what, DWORD got, DWORD want) {
    out(got == want ? "PASS " : "FAIL "); out(what); out(": got "); outu(got); out(", expected "); outu(want); out("\r\n");
    if (got != want) g_fail = 1;
}
static DWORD bits(DWORD_PTR m) { DWORD n = 0; while (m) { n += (DWORD)(m & 1); m >>= 1; } return n; }

void entry(void) {
    static WCHAR path[520]; LPCWSTR c = GetCommandLineW(); SYSTEM_INFO si; DWORD_PTR proc, sys;
    DWORD before, want, maxc = 0, i = 0; int affOnly;

    /* argv[1] = plugin path, argv[2] = MaxCPUs */
    if (*c == '"') { c++; while (*c && *c != '"') c++; if (*c) c++; } else while (*c && *c != ' ') c++;
    while (*c == ' ') c++;
    while (*c && *c != ' ' && i < 519) path[i++] = *c++;
    path[i] = 0;
    while (*c == ' ') c++;
    while (*c >= '0' && *c <= '9') maxc = maxc * 10 + (DWORD)(*c++ - '0');
    while (*c == ' ') c++;
    affOnly = *c == 'a';
    if (!path[0] || !maxc) { out("usage: ManyCoreFixTest <ManyCoreFix.asi> <MaxCPUs>\r\n"); ExitProcess(2); }

    GetSystemInfo(&si); before = si.dwNumberOfProcessors;
    want = before < maxc ? before : maxc;
    out("CPUs before loading the plugin: "); outu(before); out("\r\n");
    if (!LoadLibraryW(path)) { out("FAIL could not load the plugin\r\n"); ExitProcess(1); }

    if (affOnly) {
        if (GetProcessAffinityMask(GetCurrentProcess(), &proc, &sys)) check("Process affinity CPUs", bits(proc), want);
        else g_fail = 1;
        out(g_fail ? "RESULT: FAIL\r\n" : "RESULT: PASS\r\n");
        ExitProcess(g_fail ? 1 : 0);
    }
    GetSystemInfo(&si);
    check("GetSystemInfo CPU count", si.dwNumberOfProcessors, want);
    check("GetSystemInfo active mask CPUs", bits(si.dwActiveProcessorMask), want);
    GetNativeSystemInfo(&si);
    check("GetNativeSystemInfo CPU count", si.dwNumberOfProcessors, want);
    check("GetActiveProcessorCount", GetActiveProcessorCount(0xFFFF), want);
    if (GetProcessAffinityMask(GetCurrentProcess(), &proc, &sys)) check("Process affinity CPUs", bits(proc), want);
    else { out("FAIL GetProcessAffinityMask\r\n"); g_fail = 1; }
    out(g_fail ? "RESULT: FAIL\r\n" : "RESULT: PASS\r\n");
    ExitProcess(g_fail ? 1 : 0);
}
