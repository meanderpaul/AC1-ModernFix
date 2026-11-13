# ManyCoreFix

**Stops old PC games crashing on CPUs with lots of cores and threads.**

Many games from roughly 2005–2014 were written for 2–4 core CPUs. On a modern 12, 16, 24 or 32-thread CPU they crash on launch, on the first loading screen, or when you start a new game. The usual workaround is limiting cores in Task Manager or with a `start /affinity` shortcut, but that has to be redone every launch and Steam's Play button skips it.

ManyCoreFix is a tiny plugin, grown out of [AC1AffinityFix](HOW-IT-WORKS.md), that applies the fix from inside the game on every launch:

1. **Limits the game to `MaxCPUs` threads** (CPU affinity).
2. **Makes the game see only `MaxCPUs` CPUs** (`SpoofCPUCount=1`). Some games crash just from *counting* too many CPUs, even when limited. ManyCoreFix makes `GetSystemInfo`, `GetNativeSystemInfo`, `GetActiveProcessorCount` and `GetMaximumProcessorCount` report the lower number to the game, and sets `NUMBER_OF_PROCESSORS` for the game process.

It changes no game files, and CI tests it on Windows on every build (see [`tests/`](../tests)).

## Install

1. Find out whether the game is **32-bit or 64-bit**. Most games before about 2012 are 32-bit; PCGamingWiki lists it under "API".
2. Get **Ultimate ASI Loader** from [ThirteenAG's releases](https://github.com/ThirteenAG/Ultimate-ASI-Loader/releases): `Ultimate-ASI-Loader.zip` for 32-bit games, `Ultimate-ASI-Loader_x64.zip` for 64-bit games. Put `dinput8.dll` next to the game's exe.
3. Make a `scripts` folder next to the game's exe.
4. Copy into `scripts`:
   - 32-bit game: `ManyCoreFix.asi` and `ManyCoreFix.ini`
   - 64-bit game: `ManyCoreFix64.asi`, and `ManyCoreFix.ini` renamed to `ManyCoreFix64.ini`
5. Start the game. A `ManyCoreFix.log` (or `ManyCoreFix64.log`) should appear in `scripts`.

**No log file?** The loader isn't being loaded. Rename `dinput8.dll` to another name the loader supports that the game uses, such as `version.dll`, `winmm.dll`, `dsound.dll` or `d3d9.dll`, and try again.

## Settings (`ManyCoreFix.ini`)

| Setting | Default | What it does |
|---|---|---|
| `Enabled` | 1 | 0 turns the plugin off |
| `MaxCPUs` | 8 | Threads the game may use. Still crashing? Try 4, then 2 |
| `SpoofCPUCount` | 1 | Make the game see only `MaxCPUs` CPUs |
| `SetEnvironment` | 1 | Also set `NUMBER_OF_PROCESSORS` for the game |
| `Log` | 1 | Write a log each launch |

## Games to try it on

**None of these have been tested with ManyCoreFix yet.** They're games with documented many-core crashes. If you try one, please [open an issue](https://github.com/meanderpaul/AC1-ModernFix/issues/new) saying whether it worked, with your CPU, the `MaxCPUs` you used and your log file. Confirmed games get their own one-click installer.

| Game | Reported problem | Suggested start |
|---|---|---|
| Prototype (2009) | Crashes at the first loading screen after New Game on CPUs with more than 4 cores / 8 threads | `MaxCPUs=4` |
| Far Cry 2 | Won't launch on high-core CPUs | `MaxCPUs=8` |
| Far Cry 3 | Won't launch on high-core CPUs | `MaxCPUs=8` |
| Rocksmith 2014 Remastered | Won't launch on high-core CPUs | `MaxCPUs=8` |
| Medal of Honor: Airborne | Crashes at 32 threads | `MaxCPUs=16` |
| Child of Light | Crashes with more than 16 threads, even when limited (needs `SpoofCPUCount=1`) | `MaxCPUs=8` |
| Fallout 3 | Freezes and crashes with more than 2 cores. The usual fix is `bUseThreadedAI=1` + `iNumHWThreads=2` in `fallout.ini` | `MaxCPUs=2` |

## Limits

- Only the game's main exe is patched. A game that reads the CPU count from one of its own DLLs, or through `GetLogicalProcessorInformation`, still sees the real number. The affinity limit still applies.
- Some anti-cheat systems block injected plugins. Don't use it in online games.
