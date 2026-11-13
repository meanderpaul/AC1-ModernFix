# Building from source

Both the plugin and the installer are plain C with no C runtime, so they build with clang and lld on Linux, macOS or Windows. No Visual Studio or Windows SDK is required.

## Requirements

- clang (with the `i686-pc-windows-msvc` target, included in standard LLVM builds)
- `lld-link`, `llvm-dlltool` and `llvm-rc` from LLVM

On Ubuntu/Debian: `sudo apt install clang lld llvm`

## Build

```sh
./build.sh
```

Outputs:

- `dist/AC1AffinityFix.asi`: the AC1 plugin, a 32-bit Windows DLL of about 2.5 KB that imports only `kernel32.dll`.
- `dist/ManyCoreFix.asi` / `dist/ManyCoreFix64.asi`: the general many-core fix for 32- and 64-bit games.
- `dist/AC1ModernFix-Setup.exe`: the one-click installer (about 65 KB), with the AC1 plugin embedded.
- `dist/tests/ManyCoreFixTest*.exe`: test programs that CI runs on Windows.

Warnings are treated as errors, and `tools/check-build.sh` runs at the end to check each file's CPU type and that it imports only the expected Windows DLLs. If `./build.sh` finishes, the build is clean.

## Continuous integration

`.github/workflows/ci.yml` runs on every push and pull request:

1. **Build** on Linux with `./build.sh`.
2. **Test on Windows:** loads each plugin into `ManyCoreFixTest.exe`, which stands in for a game, with `MaxCPUs=2`. It checks that the affinity is limited and, for ManyCoreFix, that `GetSystemInfo`, `GetNativeSystemInfo`, `GetActiveProcessorCount` and `NUMBER_OF_PROCESSORS` all report 2. Both the 32-bit and 64-bit builds are tested.

Pushing a tag such as `v1.1` runs `release.yml`, which builds and publishes the release zips.

## What the build does

**Plugin**

1. `llvm-dlltool` creates a minimal `kernel32.lib` import library from `build/kernel32.def`, so no Windows SDK is needed.
2. `clang` compiles `src/AC1AffinityFix.c` for 32-bit Windows without builtins or stack probes.
3. `lld-link` links it as a DLL with `DllMain` as the entry point and no default libraries.

**Installer** (`src/setup/`)

1. `llvm-dlltool` creates an import library for each Windows DLL the installer calls, from `build/setup/*.def`. If you call a new Windows function, add its decorated name (`Name@bytes-of-arguments`) to the right `.def` file.
2. `llvm-rc` compiles `src/setup/setup.rc`: the icon, the manifest (admin rights, modern controls, DPI awareness), version info, and the freshly built `AC1AffinityFix.asi` plus its ini as embedded resources.
3. `clang` compiles `src/setup/AC1ModernFixSetup.c` and `lld-link` links the GUI exe with `entry` as its entry point.

## What the installer downloads

EaglePatch has no license, so it is never bundled. The installer downloads these exact files from their official GitHub releases, and refuses them if the MD5 doesn't match:

| File | Source | MD5 |
|---|---|---|
| `Ultimate-ASI-Loader.zip` | ThirteenAG/Ultimate-ASI-Loader v4.68 | `f185127bd44fe3dd55cd176dc4c007cb` |
| `EaglePatchAC1.zip` | Sergeanur/EaglePatch v1.1 | `a5dce4980c3a47c924157ec6c4cef9a2` |

If a user already has either zip next to the installer or in their Downloads folder, and it matches, that copy is used instead. When EaglePatch releases a new version, update the URL and hash in `AC1ModernFixSetup.c` (`EP_PKG`).

## Building the AC1 plugin with Visual Studio instead

From an x86 Developer Command Prompt:

```bat
cl /O2 /GS- /LD src\AC1AffinityFix.c /link /NODEFAULTLIB /ENTRY:DllMain kernel32.lib /OUT:AC1AffinityFix.asi
```
