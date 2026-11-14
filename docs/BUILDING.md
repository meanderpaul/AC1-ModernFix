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
- `dist/tests/ManyCoreFixTest*.exe`: test programs that CI runs on Windows.

Warnings are treated as errors, and `tools/check-build.sh` runs at the end to check each file's CPU type and that it imports only the expected Windows DLLs. If `./build.sh` finishes, the build is clean.

## Continuous integration

`.github/workflows/ci.yml` runs on every push and pull request:

1. **Build** on Linux with `./build.sh`.
2. **Test on Windows:** loads each plugin into `ManyCoreFixTest.exe`, which stands in for a game, with `MaxCPUs=2`. It checks that the affinity is limited and, for ManyCoreFix, that `GetSystemInfo`, `GetNativeSystemInfo`, and `GetActiveProcessorCount` all report 2. Both the 32-bit and 64-bit builds are tested.

Pushing a tag such as `v1.1` runs `release.yml`, which builds and publishes the release zips.

## What the build does

**Plugin**

1. `llvm-dlltool` creates a minimal `kernel32.lib` import library from `build/kernel32.def`, so no Windows SDK is needed.
2. `clang` compiles `src/AC1AffinityFix.c` for 32-bit Windows without builtins or stack probes.
3. `lld-link` links it as a DLL with `DllMain` as the entry point and no default libraries.

**Installer** (`installer/AC1ModernFix.iss`)

The AC1 one-click installer is built with [Inno Setup 6](https://jrsoftware.org/isinfo.php), a widely used installer tool, on Windows. After `./build.sh`:

```bat
"C:\Program Files (x86)\Inno Setup 6\ISCC.exe" installer\AC1ModernFix.iss
```

It writes `dist\AC1ModernFix-Setup.exe`. CI does this on every push, on a Windows runner.

## What the installer downloads

EaglePatch has no license, so it is never bundled. The installer downloads these exact files from their official GitHub releases, and refuses them if the SHA-256 doesn't match:

| File | Source | SHA-256 |
|---|---|---|
| `Ultimate-ASI-Loader.zip` | ThirteenAG/Ultimate-ASI-Loader v4.68 | `7517c7d5bd8c475f18e6545b7998df379eed6e2e0a19fcd2139669152f9bcddb` |
| `EaglePatchAC1.zip` | Sergeanur/EaglePatch v1.1 | `6dc496ac6b298b92fb70c3f90a34e323f77ac88e6ea97544f84e52435038aeea` |

When EaglePatch releases a new version, update `EpUrl` and `EpSha256` in `installer/AC1ModernFix.iss`.

## Building the AC1 plugin with Visual Studio instead

From an x86 Developer Command Prompt:

```bat
cl /O2 /GS- /LD src\AC1AffinityFix.c /link /NODEFAULTLIB /ENTRY:DllMain kernel32.lib /OUT:AC1AffinityFix.asi
```
