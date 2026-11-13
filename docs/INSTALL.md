# Detailed install guide

This walks through the manual install step by step. The [one-click installer](../README.md#one-click-install-about-1-minute) does all of this for you. If you've modded games before, the [Manual install in the README](../README.md#manual-install-about-3-minutes) is enough.

## Before you start

- **Game version:** Assassin's Creed: Director's Cut Edition from Steam or GOG.
- **Unmodified exes:** if you've hex-edited `AssassinsCreed_Dx9.exe` or `AssassinsCreed_Dx10.exe`, or applied another crash fix that changes them, restore the originals first. In Steam: right-click the game → Properties → Installed Files → **Verify integrity of game files**. EaglePatch only loads on the exact exes it supports:

| File | MD5 |
|---|---|
| AssassinsCreed_Dx9.exe | `8E72C3333743780E43BC2C34BBF625F9` |
| AssassinsCreed_Dx10.exe | `CA87753255E2D14B1F18BB737C643792` |

  To check, open PowerShell in the game folder and run:
  ```powershell
  Get-FileHash AssassinsCreed_Dx9.exe -Algorithm MD5
  ```

- **Remove old workarounds:** turn off any Windows compatibility mode you set on the game exes (right-click exe → Properties → Compatibility → untick everything).

## Step 1 — Find the game folder

In Steam, right-click **Assassin's Creed** → **Manage** → **Browse local files**. The default location is:

```
C:\Program Files (x86)\Steam\steamapps\common\Assassins Creed
```

or `<Drive>:\SteamLibrary\steamapps\common\Assassins Creed` if you use a second drive.

## Step 2 — Install the DirectX runtime files

Run `DirectX\DXSETUP.exe` from the game folder and click through it. This installs `xinput1_3.dll`, which the game needs for controllers and which Windows 10/11 doesn't include. It's safe to run even if it's already installed.

## Step 3 — Install Ultimate ASI Loader

1. Download [Ultimate-ASI-Loader.zip (v4.68)](https://github.com/ThirteenAG/Ultimate-ASI-Loader/releases/download/v4.68/Ultimate-ASI-Loader.zip).
2. Extract `dinput8.dll` and put it in the game folder, next to `AssassinsCreed_Dx9.exe`.

Use the 32-bit (x86) build. The v4.68 link above is the right one.

## Step 4 — Install EaglePatch

1. Download the latest **EaglePatchAC1.zip** from [Sergeanur/EaglePatch releases](https://github.com/Sergeanur/EaglePatch/releases).
2. Create a folder named `scripts` in the game folder.
3. Put `EaglePatchAC1.asi` and `EaglePatchAC1.ini` in `scripts`.

## Step 5 — Install AC1AffinityFix

1. Download **AC1AffinityFix-v1.0.zip** from the [latest release](https://github.com/meanderpaul/AC1-ModernFix/releases/latest), or from the repo's [`release` folder](https://github.com/meanderpaul/AC1-ModernFix/tree/main/release).
2. Either:
   - copy `AC1AffinityFix.asi` and `AC1AffinityFix.ini` into `scripts`, **or**
   - extract the zip into the game folder and double-click `install.bat`. It checks that the ASI Loader and EaglePatch are present and copies the files for you.

## Step 6 — Play

1. Plug in your controller (wired, wireless adapter or Bluetooth all work).
2. Launch from Steam, a shortcut, or `AssassinsCreed_Dx9.exe`.
3. If Steam asks for DirectX 9 or 10, choose **DirectX 9**. DX10 works for many people, but DX9 is the most reliable on current Windows.

## Optional tweaks

Edit `scripts\EaglePatchAC1.ini` in Notepad:

| Setting | What it does |
|---|---|
| `SkipIntroVideos=1` | Skips the logo videos |
| `PS3Controls=1` | Swaps triggers and bumpers (RB becomes run) |
| `KeyboardLayout=0..3` | Picks the keyboard layout; the in-game layout menu no longer applies |

Edit `scripts\AC1AffinityFix.ini`:

| Setting | What it does |
|---|---|
| `MaxCPUs=8` | Max CPU threads the game may use. Try 4 if you still crash |
| `Enabled=0` | Turns the CPU fix off |

## Known game bug: Leap of Faith deaths

Unrelated to this fix: at very high frame rates Altaïr can die landing in hay. Turn on VSync in the game's graphics settings and in your GPU control panel.
