# AC1 Modern Fix

**Get Assassin's Creed: Director's Cut running on a modern Windows PC — no launch crash, working Xbox controller.**

By **Meanderpaul_gaming**

The Steam and GOG versions of Assassin's Creed (2008) often crash the moment you press Play on a current PC, and Xbox One / Series controllers do nothing even though the store page lists controller support. This repo packages a tested fix for both, built around the community's EaglePatch plus a small plugin of my own that fixes the many-core CPU crash on every launch path, including Steam's Play button.

| Problem | Cause | Fixed by |
|---|---|---|
| Crashes on launch | The game breaks on CPUs with lots of threads, and tries to reach a Ubisoft server that no longer exists | **AC1AffinityFix** (this repo) + **EaglePatch** |
| Xbox controller ignored | The game only recognises the original Xbox 360 pad | **EaglePatch** |
| `xinput1_3.dll` missing | Not included in Windows 10/11 | DirectX installer already in your game folder |

## Quick install (about 3 minutes)

You need the **original, unmodified** Steam or GOG game files. EaglePatch checks the exe and won't load on a modified one.

1. **Get the two community files** (official pages only):
   - **EaglePatchAC1.zip** by Sergeanur — [github.com/Sergeanur/EaglePatch/releases](https://github.com/Sergeanur/EaglePatch/releases)
   - **Ultimate-ASI-Loader.zip** by ThirteenAG — [direct download, v4.68](https://github.com/ThirteenAG/Ultimate-ASI-Loader/releases/download/v4.68/Ultimate-ASI-Loader.zip)
2. **Get AC1AffinityFix** — [download AC1AffinityFix-v1.0.zip](https://github.com/meanderpaul/AC1-ModernFix/raw/main/release/AC1AffinityFix-v1.0.zip) (or browse the [`release` folder](release/)).
3. **Open your game folder.** In Steam: right-click Assassin's Creed → Manage → Browse local files.
4. Copy **`dinput8.dll`** from the ASI Loader zip into the game folder.
5. Create a folder called **`scripts`** inside the game folder.
6. Copy **`EaglePatchAC1.asi`** and **`EaglePatchAC1.ini`** into `scripts`.
7. Copy **`AC1AffinityFix.asi`** and **`AC1AffinityFix.ini`** into `scripts`.
8. Run **`DirectX\DXSETUP.exe`** from the game folder once (installs `xinput1_3.dll` if it's missing).
9. Plug in your controller and press **Play** in Steam. If asked, choose **DirectX 9**.

Your game folder should end up like this:

```
Assassins Creed\
├── AssassinsCreed_Dx9.exe
├── AssassinsCreed_Dx10.exe
├── dinput8.dll                  ← ASI Loader
└── scripts\
    ├── EaglePatchAC1.asi
    ├── EaglePatchAC1.ini
    ├── AC1AffinityFix.asi       ← this repo
    └── AC1AffinityFix.ini
```

The download zip also has an `install.bat` that checks your folder and copies the AC1AffinityFix files into `scripts` for you.

## Recommended settings

In `scripts\EaglePatchAC1.ini`:

- `SkipIntroVideos=1` — skip the logo videos at startup.
- `PS3Controls=1` — optional; swaps triggers and bumpers so R1/RB is "run".

In `scripts\AC1AffinityFix.ini`:

- `MaxCPUs=8` — how many CPU threads the game may use. Lower it to 4 if you still crash.
- `Enabled=0` — turn the CPU fix off.

## Docs

- [Detailed install guide](docs/INSTALL.md)
- [Troubleshooting](docs/TROUBLESHOOTING.md)
- [How it works](docs/HOW-IT-WORKS.md)
- [Building AC1AffinityFix from source](docs/BUILDING.md)

## Uninstall

Delete `dinput8.dll` and the `scripts` folder from the game folder. Nothing else is changed.

## Credits

- **EaglePatch** — [Sergeanur](https://github.com/Sergeanur/EaglePatch). Does the heavy lifting: XInput controller support, hot-plug detection, telemetry removal. Not included in this repo; download it from the official page.
- **Ultimate ASI Loader** — [ThirteenAG](https://github.com/ThirteenAG/Ultimate-ASI-Loader). Loads the plugins.
- **PCGamingWiki** — [Assassin's Creed page](https://www.pcgamingwiki.com/wiki/Assassin%27s_Creed), for documenting the known issues.
- **AC1AffinityFix**, install script and docs — Meanderpaul_gaming. MIT licensed.

Assassin's Creed is a trademark of Ubisoft. This project is not affiliated with or endorsed by Ubisoft, Valve, or Microsoft.
