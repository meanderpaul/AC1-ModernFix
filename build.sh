#!/usr/bin/env sh
# Builds everything with clang + lld only (no Windows SDK, no Visual Studio):
#   dist/AC1AffinityFix.asi        AC1 plugin (32-bit)
#   dist/ManyCoreFix.asi           many-core fix for 32-bit games
#   dist/ManyCoreFix64.asi         many-core fix for 64-bit games
#   dist/tests/ManyCoreFixTest*.exe  test programs, run by CI on Windows
# Warnings are errors, so a build either comes out clean or fails.
# The AC1 installer (installer/AC1ModernFix.iss) is built afterwards with Inno Setup on Windows (CI does it).
set -e
cd "$(dirname "$0")"
LLVM_BIN="${LLVM_BIN:-$(dirname "$(command -v lld-link || echo /usr/lib/llvm-18/bin/lld-link)")}"
CC="${CLANG:-clang}"
CFLAGS="-O2 -Wall -Wextra -Werror -fno-builtin -fno-stack-protector -mno-stack-arg-probe"
OBJ=build/obj
mkdir -p "$OBJ/x86" "$OBJ/x64" dist/tests

# implib <def> <x86|x64> <out.lib>: import library from a .def (x86 names keep @N; x64 has none)
implib() {
  if [ "$2" = x64 ]; then
    sed 's/@[0-9]*$//' "$1" > "$OBJ/x64/$(basename "$1")"
    "$LLVM_BIN/llvm-dlltool" -m i386:x86-64 -d "$OBJ/x64/$(basename "$1")" -l "$3"
  else
    "$LLVM_BIN/llvm-dlltool" -m i386 -d "$1" -l "$3" -k
  fi
}
# cc <x86|x64> <src.c> <out.obj>
cc() {
  if [ "$1" = x64 ]; then t=x86_64-pc-windows-msvc; else t=i686-pc-windows-msvc; fi
  $CC --target=$t $CFLAGS -c "$2" -o "$3"
}
# link <x86|x64> <dll|exe> <entry> <out> <inputs...>
link() {
  arch=$1; kind=$2; entry=$3; out=$4; shift 4
  m=x86; extra="/safeseh:no"; [ "$arch" = x64 ] && { m=x64; extra=""; }
  if [ "$kind" = dll ]; then
    [ "$arch" = x86 ] && entry="$entry@12"
    "$LLVM_BIN/lld-link" /dll /noimplib /machine:$m /nodefaultlib /entry:$entry /subsystem:windows $extra "$@" /out:"$out"
  else
    "$LLVM_BIN/lld-link" /machine:$m /nodefaultlib /entry:$entry $extra "$@" /out:"$out"
  fi
}

# AC1AffinityFix (32-bit)
implib build/kernel32.def x86 "$OBJ/x86/ac1-kernel32.lib"
cc x86 src/AC1AffinityFix.c "$OBJ/x86/AC1AffinityFix.obj"
link x86 dll DllMain dist/AC1AffinityFix.asi "$OBJ/x86/AC1AffinityFix.obj" "$OBJ/x86/ac1-kernel32.lib"

# ManyCoreFix (32 + 64-bit) and its test program
for a in x86 x64; do
  suffix=""; [ $a = x64 ] && suffix=64
  implib build/manycorefix/kernel32.def $a "$OBJ/$a/mcf-kernel32.lib"
  cc $a src/manycorefix/ManyCoreFix.c "$OBJ/$a/ManyCoreFix.obj"
  link $a dll DllMain "dist/ManyCoreFix$suffix.asi" "$OBJ/$a/ManyCoreFix.obj" "$OBJ/$a/mcf-kernel32.lib"
  cc $a tests/ManyCoreFixTest.c "$OBJ/$a/ManyCoreFixTest.obj"
  link $a exe entry "dist/tests/ManyCoreFixTest$suffix.exe" /subsystem:console "$OBJ/$a/ManyCoreFixTest.obj" "$OBJ/$a/mcf-kernel32.lib"
done

sh tools/check-build.sh
