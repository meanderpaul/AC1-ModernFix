/* ManyCoreFix.asi - stops old PC games crashing on CPUs with lots of cores/threads.
   Loaded by Ultimate ASI Loader. Works in 32-bit and 64-bit games (build both).

   1. Limits the game to MaxCPUs logical CPUs (SetProcessAffinityMask), like AC1AffinityFix.
   2. Optionally makes the game *see* only MaxCPUs CPUs, for games that crash just from counting
      too many: patches the game exe's import table so GetSystemInfo, GetNativeSystemInfo,
      GetActiveProcessorCount and GetMaximumProcessorCount report at most MaxCPUs, and sets the
      NUMBER_OF_PROCESSORS environment variable for the game process.
   Settings: ManyCoreFix.ini next to the .asi. Optional log: ManyCoreFix.log next to the .asi.

   No C runtime, no Windows SDK. Copyright (c) 2026 Meanderpaul_gaming. MIT License. */

typedef int BOOL; typedef unsigned long DWORD; typedef unsigned short WORD; typedef unsigned char BYTE;
typedef unsigned int UINT; typedef __WCHAR_TYPE__ WCHAR; typedef __SIZE_TYPE__ SIZE_T;
typedef SIZE_T DWORD_PTR; typedef void *HANDLE, *HMODULE, *LPVOID;
typedef const WCHAR *LPCWSTR;
#define WINAPI __stdcall
#define IMP __declspec(dllimport)

typedef struct {
    WORD wProcessorArchitecture, wReserved; DWORD dwPageSize;
    LPVOID lpMinimumApplicationAddress, lpMaximumApplicationAddress; DWORD_PTR dwActiveProcessorMask;
    DWORD dwNumberOfProcessors, dwProcessorType, dwAllocationGranularity; WORD wProcessorLevel, wProcessorRevision;
} SYSTEM_INFO;

IMP HMODULE WINAPI GetModuleHandleW(LPCWSTR);
IMP DWORD WINAPI GetModuleFileNameW(HMODULE, WCHAR *, DWORD);
IMP void *WINAPI GetProcAddress(HMODULE, const char *);
IMP UINT WINAPI GetPrivateProfileIntW(LPCWSTR, LPCWSTR, int, LPCWSTR);
IMP HANDLE WINAPI GetCurrentProcess(void);
IMP BOOL WINAPI GetProcessAffinityMask(HANDLE, DWORD_PTR *, DWORD_PTR *);
IMP BOOL WINAPI SetProcessAffinityMask(HANDLE, DWORD_PTR);
IMP BOOL WINAPI VirtualProtect(LPVOID, SIZE_T, DWORD, DWORD *);
IMP BOOL WINAPI DisableThreadLibraryCalls(HMODULE);
IMP BOOL WINAPI SetEnvironmentVariableW(LPCWSTR, LPCWSTR);
IMP DWORD WINAPI GetEnvironmentVariableW(LPCWSTR, WCHAR *, DWORD);
IMP HANDLE WINAPI CreateFileW(LPCWSTR, DWORD, DWORD, LPVOID, DWORD, DWORD, HANDLE);
IMP BOOL WINAPI WriteFile(HANDLE, const void *, DWORD, DWORD *, LPVOID);
IMP BOOL WINAPI CloseHandle(HANDLE);
IMP void WINAPI GetSystemInfo(SYSTEM_INFO *);
IMP void WINAPI GetNativeSystemInfo(SYSTEM_INFO *);
IMP DWORD WINAPI GetActiveProcessorCount(WORD);
IMP DWORD WINAPI GetMaximumProcessorCount(WORD);

#define MAXP 520
static DWORD g_max = 0xFFFFFFFFul;      /* CPU count the game is allowed to see */
static DWORD_PTR g_mask;                /* affinity mask we applied, 0 if unchanged */
static HANDLE g_log = (HANDLE)0;

/* ---------- log ---------- */
static void log_s(const char *s) {
    DWORD n = 0, w; while (s[n]) n++;
    if (g_log) WriteFile(g_log, s, n, &w, 0);
}
static void log_u(SIZE_T v) {
    char b[24]; int i = 23; b[i] = 0;
    do { b[--i] = (char)('0' + (int)(v % 10)); v /= 10; } while (v && i);
    log_s(b + i);
}
static void log_hex(SIZE_T v) {
    static const char d[] = "0123456789ABCDEF"; char b[20]; int i = 19; b[i] = 0;
    do { b[--i] = d[v & 15]; v >>= 4; } while (v && i > 2);
    b[--i] = 'x'; b[--i] = '0'; log_s(b + i);
}

/* ---------- hooks ---------- */
static void clamp(SYSTEM_INFO *si) {
    if (si->dwNumberOfProcessors > g_max) si->dwNumberOfProcessors = g_max;
    if (g_mask) si->dwActiveProcessorMask = g_mask;
}
static void WINAPI hook_GetSystemInfo(SYSTEM_INFO *si) { GetSystemInfo(si); clamp(si); }
static void WINAPI hook_GetNativeSystemInfo(SYSTEM_INFO *si) { GetNativeSystemInfo(si); clamp(si); }
static DWORD WINAPI hook_GetActiveProcessorCount(WORD g) { DWORD n = GetActiveProcessorCount(g); return n > g_max ? g_max : n; }
static DWORD WINAPI hook_GetMaximumProcessorCount(WORD g) { DWORD n = GetMaximumProcessorCount(g); return n > g_max ? g_max : n; }

typedef struct { const char *name; void *hook; void *real[2]; } HOOK;
static HOOK g_hooks[] = {
    { "GetSystemInfo", (void *)hook_GetSystemInfo, { 0, 0 } },
    { "GetNativeSystemInfo", (void *)hook_GetNativeSystemInfo, { 0, 0 } },
    { "GetActiveProcessorCount", (void *)hook_GetActiveProcessorCount, { 0, 0 } },
    { "GetMaximumProcessorCount", (void *)hook_GetMaximumProcessorCount, { 0, 0 } },
};
#define NHOOKS (int)(sizeof g_hooks / sizeof g_hooks[0])

/* Replaces imported function pointers in module `base`'s import address table.
   An entry is matched by address (kernel32 or kernelbase export), so it works whatever DLL
   name the exe imports it through. Returns the number of entries patched. */
static int patch_iat(BYTE *base) {
    DWORD peoff, impRva, *desc; WORD magic; BYTE *opt; int patched = 0, h, k;
    if (!base || base[0] != 'M' || base[1] != 'Z') return 0;
    peoff = *(DWORD *)(base + 0x3C);
    if (base[peoff] != 'P' || base[peoff + 1] != 'E') return 0;
    opt = base + peoff + 24;
    magic = *(WORD *)opt;
    impRva = *(DWORD *)(opt + (magic == 0x20B ? 112 : 96) + 8);   /* DataDirectory[1].VirtualAddress */
    if (!impRva) return 0;
    for (desc = (DWORD *)(base + impRva); desc[3]; desc += 5) {     /* Name != 0 */
        void **iat = (void **)(base + desc[4]);                     /* FirstThunk */
        for (; *iat; iat++)
            for (h = 0; h < NHOOKS; h++)
                for (k = 0; k < 2; k++)
                    if (g_hooks[h].real[k] && *iat == g_hooks[h].real[k]) {
                        DWORD old;
                        if (VirtualProtect(iat, sizeof(void *), 0x04 /* PAGE_READWRITE */, &old)) {
                            *iat = g_hooks[h].hook;
                            VirtualProtect(iat, sizeof(void *), old, &old);
                            patched++;
                            if (g_log) { log_s("  hooked "); log_s(g_hooks[h].name); log_s("\r\n"); }
                        }
                        h = NHOOKS; break;
                    }
    }
    return patched;
}

static void apply(HMODULE self) {
    WCHAR ini[MAXP], logp[MAXP], num[12]; DWORD n = GetModuleFileNameW(self, ini, MAXP - 4), i;
    int enabled = 1, maxCpus = 8, spoof = 1, env = 1, logOn = 0, count = 0, h;
    DWORD_PTR proc = 0, sys = 0, mask = 0; HANDLE cur = GetCurrentProcess();
    HMODULE k32 = GetModuleHandleW(L"kernel32.dll"), kb = GetModuleHandleW(L"kernelbase.dll");

    if (n > 4 && n < MAXP - 4) {                                    /* same path, .asi -> .ini / .log */
        for (i = 0; i <= n; i++) logp[i] = ini[i];
        ini[n - 3] = 'i'; ini[n - 2] = 'n'; ini[n - 1] = 'i';
        logp[n - 3] = 'l'; logp[n - 2] = 'o'; logp[n - 1] = 'g';
        enabled = (int)GetPrivateProfileIntW(L"ManyCoreFix", L"Enabled", 1, ini);
        maxCpus = (int)GetPrivateProfileIntW(L"ManyCoreFix", L"MaxCPUs", 8, ini);
        spoof = (int)GetPrivateProfileIntW(L"ManyCoreFix", L"SpoofCPUCount", 1, ini);
        env = (int)GetPrivateProfileIntW(L"ManyCoreFix", L"SetEnvironment", 1, ini);
        logOn = (int)GetPrivateProfileIntW(L"ManyCoreFix", L"Log", 1, ini);
        if (logOn) {
            g_log = CreateFileW(logp, 0x40000000ul /* GENERIC_WRITE */, 1, 0, 2 /* CREATE_ALWAYS */, 0x80, 0);
            if (g_log == (HANDLE)(SIZE_T)-1) g_log = 0;
        }
    }
    log_s("ManyCoreFix 1.0 (");
    log_s(sizeof(void *) == 8 ? "64-bit" : "32-bit");
    log_s(")\r\nEnabled="); log_u((unsigned)enabled); log_s(" MaxCPUs="); log_u((unsigned)maxCpus);
    log_s(" SpoofCPUCount="); log_u((unsigned)spoof); log_s(" SetEnvironment="); log_u((unsigned)env); log_s("\r\n");
    if (!enabled || maxCpus < 1) { log_s("Disabled, nothing changed.\r\n"); goto done; }

    /* 1. affinity */
    if (GetProcessAffinityMask(cur, &proc, &sys) && proc) {
        for (i = 0; i < sizeof(DWORD_PTR) * 8 && count < maxCpus; i++)
            if (proc & ((DWORD_PTR)1 << i)) { mask |= (DWORD_PTR)1 << i; count++; }
        log_s("Process affinity "); log_hex(proc);
        if (mask && mask != proc && SetProcessAffinityMask(cur, mask)) {
            g_mask = mask; log_s(" -> "); log_hex(mask); log_s(" ("); log_u((unsigned)count); log_s(" CPUs)\r\n");
        } else log_s(" unchanged\r\n");
    }

    /* 2. CPU count the game sees */
    if (spoof) {
        g_max = (DWORD)maxCpus;
        for (h = 0; h < NHOOKS; h++) {
            g_hooks[h].real[0] = k32 ? GetProcAddress(k32, g_hooks[h].name) : 0;
            g_hooks[h].real[1] = kb ? GetProcAddress(kb, g_hooks[h].name) : 0;
        }
        h = patch_iat((BYTE *)GetModuleHandleW(0));
        log_s("Game sees at most "); log_u((unsigned)maxCpus); log_s(" CPUs ("); log_u((unsigned)h);
        log_s(" imports patched)\r\n");
    }
    if (env && GetActiveProcessorCount(0xFFFF /* all groups */) > (DWORD)maxCpus) {
        int v = maxCpus, j = 0; WCHAR t[12];
        do { t[j++] = (WCHAR)('0' + v % 10); v /= 10; } while (v && j < 11);
        for (i = 0; i < (DWORD)j; i++) num[i] = t[j - 1 - i];
        num[j] = 0;
        if (SetEnvironmentVariableW(L"NUMBER_OF_PROCESSORS", num)) {
            WCHAR back[12]; DWORD bn = GetEnvironmentVariableW(L"NUMBER_OF_PROCESSORS", back, 12), q;
            log_s("NUMBER_OF_PROCESSORS="); log_u((unsigned)maxCpus); log_s(" (reads back ");
            for (q = 0; q < bn && q < 11; q++) { char ch[2]; ch[0] = (char)back[q]; ch[1] = 0; log_s(ch); }
            log_s(")\r\n");
        }
    }
done:
    if (g_log) { CloseHandle(g_log); g_log = 0; }
}

BOOL WINAPI DllMain(HMODULE inst, DWORD reason, void *reserved) {
    (void)reserved;
    if (reason == 1 /* DLL_PROCESS_ATTACH */) { DisableThreadLibraryCalls(inst); apply(inst); }
    return 1;
}
