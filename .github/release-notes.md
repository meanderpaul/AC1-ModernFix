Fixes Assassin's Creed: Director's Cut crashing on launch on modern many-core CPUs, from every launch path (Steam's Play button, shortcuts, the exe).

**Download the AC1AffinityFix zip below.** It contains:
- `AC1AffinityFix.asi`: the plugin (32-bit, 2.5 KB, imports only kernel32)
- `AC1AffinityFix.ini`: settings (`MaxCPUs=8`, `Enabled=1`)
- `install.bat`: copies the plugin into `scripts\` and checks the other pieces are installed

This plugin is one part of the fix. For the full fix, including Xbox controller support, you also need **EaglePatch** by Sergeanur and **Ultimate ASI Loader** by ThirteenAG. Follow the [install guide](https://github.com/meanderpaul/AC1-ModernFix/blob/main/docs/INSTALL.md).

Watch the fix in action: [Meanderpaul_gaming on YouTube](https://www.youtube.com/@meanderpaul_gaming/shorts)
