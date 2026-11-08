# How it works

A short tour of why Assassin's Creed (2008) breaks on modern PCs and what each piece of this fix does.

## The launch crash

There are two separate problems that both show up as "crashes when I press Play".

### 1. Too many CPU threads

AC1 was written when four cores was a lot. On CPUs with many logical processors (12, 16, 24, 32 threads) the game's startup code can crash. The long-standing community workaround is to restrict the game to fewer cores, usually with Task Manager or a `start /affinity` batch file. That works, but only if you always launch through the batch file, and Steam's Play button bypasses it.

**AC1AffinityFix** does the same thing from inside the game. It's a tiny plugin (a DLL with an `.asi` extension) that Ultimate ASI Loader loads as soon as the game starts, before the game's own initialisation runs. It reads `MaxCPUs` from its ini, picks that many of the CPUs Windows allows the process to use, and calls `SetProcessAffinityMask`. Because the loader runs on every launch, the fix applies whether you start the game from Steam, a shortcut or the exe.

The whole plugin is about 40 lines of C with no runtime library, see [`src/AC1AffinityFix.c`](../src/AC1AffinityFix.c).

### 2. A dead Ubisoft server

The game tries to contact `gconnect.ubi.com`, which no longer exists, and on some systems this hangs or crashes it. Older guides fix it by hex-editing the exe to blank out that address. **EaglePatch removes this telemetry at runtime instead**, so the exe stays untouched.

## The controller

The game imports `XINPUT1_3.dll`, but its controller code only accepts the original Xbox 360 controller. Newer Xbox One and Series controllers speak the same XInput protocol but aren't recognised, so the game falls back to its DirectInput path, which doesn't handle them either. **EaglePatch** patches the game in memory to accept all XInput controllers, adds hot-plug detection so you can connect the pad after starting the game, and lets gamepad and keyboard/mouse work at the same time.

`xinput1_3.dll` itself isn't part of Windows 10 or 11. It comes from the DirectX June 2010 runtime, which Steam is supposed to install on first launch but sometimes doesn't. The installer is in the game folder at `DirectX\DXSETUP.exe`.

## How the pieces load

```
Steam / shortcut / exe
        │
        ▼
AssassinsCreed_Dx9.exe  ── imports dinput8.dll
        │
        ▼
dinput8.dll  (Ultimate ASI Loader — forwards real DirectInput calls,
        │     and loads every .asi in the scripts folder)
        ├──► scripts\AC1AffinityFix.asi   limit CPU threads
        └──► scripts\EaglePatchAC1.asi    XInput support, telemetry removal
```

Nothing on disk is modified apart from the files you add, so uninstalling is just deleting `dinput8.dll` and `scripts`.
