# Building AC1AffinityFix from source

The plugin is plain C with no C runtime, so it builds with clang and lld on Linux, macOS or Windows. No Visual Studio or Windows SDK is required.

## Requirements

- clang (with the `i686-pc-windows-msvc` target, included in standard LLVM builds)
- `lld-link` and `llvm-dlltool` from LLVM

On Ubuntu/Debian: `sudo apt install clang lld llvm`

## Build

```sh
./build.sh
```

The output is `dist/AC1AffinityFix.asi`, a 32-bit Windows DLL of about 2.5 KB that imports only `kernel32.dll`.

## What the build does

1. `llvm-dlltool` creates a minimal `kernel32.lib` import library from `build/kernel32.def`, so no Windows SDK is needed.
2. `clang` compiles `src/AC1AffinityFix.c` for 32-bit Windows without builtins or stack probes.
3. `lld-link` links it as a DLL with `DllMain` as the entry point and no default libraries.

## Building with Visual Studio instead

From an x86 Developer Command Prompt:

```bat
cl /O2 /GS- /LD src\AC1AffinityFix.c /link /NODEFAULTLIB /ENTRY:DllMain kernel32.lib /OUT:AC1AffinityFix.asi
```
