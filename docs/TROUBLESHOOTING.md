# Troubleshooting

## The game still crashes on launch

1. **Check the exes are original.** EaglePatch silently does nothing on a modified exe. Verify game files in Steam, then reinstall the mods (verifying doesn't touch `dinput8.dll` or `scripts`). See the hashes in [INSTALL.md](INSTALL.md#before-you-start).
2. **Lower the CPU limit.** Set `MaxCPUs=4` in `scripts\AC1AffinityFix.ini`.
3. **Use DirectX 9.** Launch `AssassinsCreed_Dx9.exe` directly, or pick DirectX 9 when Steam asks.
4. **Remove compatibility mode** from both game exes (Properties → Compatibility → untick everything). Old guides recommend Windows XP mode; with this fix it usually causes more problems than it solves.
5. **Check `dinput8.dll` is the 32-bit build.** The game is 32-bit; a 64-bit loader won't load.
6. **Run `DirectX\DXSETUP.exe`** from the game folder.

## "xinput1_3.dll was not found"

Run `DirectX\DXSETUP.exe` from the game folder. If that fails, install Microsoft's [DirectX End-User Runtimes (June 2010)](https://www.microsoft.com/en-us/download/details.aspx?id=8109).

## The controller doesn't respond

1. **Is EaglePatch loading?** With `SkipIntroVideos=1` in `EaglePatchAC1.ini`, the logo videos should be skipped. If they still play, EaglePatch isn't loading — check the exe hashes and that `dinput8.dll` is next to the exe.
2. **Steam Input:** try switching it off: Steam → right-click the game → Properties → Controller → **Disable Steam Input**. If it was already off, try **Enable Steam Input** instead.
3. **Close other controller tools** such as DS4Windows, x360ce or InputMapper; they can claim the controller first.
4. **PlayStation / Switch controllers:** the game only reads XInput (Xbox-style) controllers. Turn on Steam Input for the game, or use DS4Windows, so they appear as an Xbox controller.
5. **`DisableXInputPatch`** in `EaglePatchAC1.ini` must be `0`.

## Buttons are in the wrong place

- RB/RT feel swapped compared with the console? Set `PS3Controls=1` in `EaglePatchAC1.ini`.
- The in-game "control layout" option does nothing with EaglePatch installed. That's expected; use the ini instead.

## The game opens in a window

Press **Alt + Enter**.

## Altaïr dies landing in hay

A known bug at high frame rates. Turn on VSync in the game and in your NVIDIA or AMD control panel.

## Still stuck?

Open an issue on this repo with your Windows version, CPU, GPU, controller model and what happens when you launch.
