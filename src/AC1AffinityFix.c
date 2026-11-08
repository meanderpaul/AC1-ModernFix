/* AC1AffinityFix.asi - limits Assassin's Creed to N logical CPUs at startup.
   Fixes the launch crash on modern many-core CPUs. Loaded by Ultimate ASI Loader.
   Settings are read from AC1AffinityFix.ini next to the .asi.
   Copyright (c) 2026 Meanderpaul_gaming. MIT License. */
typedef int BOOL; typedef unsigned long DWORD; typedef void* HANDLE; typedef void* HINSTANCE;
typedef unsigned long DWORD_PTR; typedef unsigned int UINT;
#define WINAPI __stdcall
#define DLL_PROCESS_ATTACH 1
__declspec(dllimport) HANDLE WINAPI GetCurrentProcess(void);
__declspec(dllimport) BOOL WINAPI GetProcessAffinityMask(HANDLE, DWORD_PTR*, DWORD_PTR*);
__declspec(dllimport) BOOL WINAPI SetProcessAffinityMask(HANDLE, DWORD_PTR);
__declspec(dllimport) UINT WINAPI GetPrivateProfileIntA(const char*, const char*, int, const char*);
__declspec(dllimport) DWORD WINAPI GetModuleFileNameA(HINSTANCE, char*, DWORD);
__declspec(dllimport) BOOL WINAPI DisableThreadLibraryCalls(HINSTANCE);

static void apply(HINSTANCE self) {
    char ini[520]; DWORD n = GetModuleFileNameA(self, ini, 512), i;
    int maxCpus = 8, enabled = 1;
    if (n > 4 && n < 512) {            /* same path, .asi -> .ini */
        ini[n-3]='i'; ini[n-2]='n'; ini[n-1]='i';
        enabled = (int)GetPrivateProfileIntA("AC1AffinityFix", "Enabled", 1, ini);
        maxCpus = (int)GetPrivateProfileIntA("AC1AffinityFix", "MaxCPUs", 8, ini);
    }
    if (!enabled || maxCpus < 1) return;
    DWORD_PTR proc = 0, sys = 0, mask = 0; int count = 0;
    HANDLE h = GetCurrentProcess();
    if (!GetProcessAffinityMask(h, &proc, &sys) || !proc) return;
    for (i = 0; i < 32 && count < maxCpus; i++)
        if (proc & ((DWORD_PTR)1 << i)) { mask |= (DWORD_PTR)1 << i; count++; }
    if (mask && mask != proc) SetProcessAffinityMask(h, mask);
}

BOOL WINAPI DllMain(HINSTANCE inst, DWORD reason, void* reserved) {
    (void)reserved;
    if (reason == DLL_PROCESS_ATTACH) { DisableThreadLibraryCalls(inst); apply(inst); }
    return 1;
}
