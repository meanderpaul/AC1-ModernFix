Fixes Assassin's Creed: Director's Cut crashing on launch on modern many-core CPUs, and gets Xbox One / Series controllers working.

### Easiest: the one-click installer

Download **AC1ModernFix-Setup** below, unzip it, and run `AC1ModernFix-Setup.exe`. It:
- finds your Steam or GOG copy of the game
- checks your game exe is one EaglePatch supports
- downloads **Ultimate ASI Loader** (ThirteenAG) and **EaglePatch** (Sergeanur) from their official GitHub releases and checks their hashes
- installs **AC1AffinityFix** (the many-core CPU crash fix)
- installs the DirectX controller runtime if `xinput1_3.dll` is missing
- adds AC1 Modern Fix to Windows' Installed apps, so it uninstalls cleanly

Windows may show "Windows protected your PC" because the installer isn't code-signed yet. Click **More info → Run anyway**.

### Manual install

**AC1AffinityFix** zip: the plugin (32-bit, 2.5 KB, imports only kernel32), its ini and `install.bat`. You also need EaglePatch and Ultimate ASI Loader; follow the [install guide](https://github.com/meanderpaul/AC1-ModernFix/blob/main/docs/INSTALL.md).

Watch the fix in action: [Meanderpaul_gaming on YouTube](https://www.youtube.com/@meanderpaul_gaming/shorts)
