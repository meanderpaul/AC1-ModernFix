#!/usr/bin/env sh
# Builds dist/AC1AffinityFix.asi (32-bit Windows DLL) with clang + lld. No Windows SDK needed.
set -e
cd "$(dirname "$0")"
LLVM_BIN="${LLVM_BIN:-$(dirname "$(command -v lld-link || echo /usr/lib/llvm-18/bin/lld-link)")}"
mkdir -p build/obj dist
"$LLVM_BIN/llvm-dlltool" -m i386 -d build/kernel32.def -l build/obj/kernel32.lib -k
"${CLANG:-clang}" --target=i686-pc-windows-msvc -O2 -c -fno-builtin -fno-stack-protector -mno-stack-arg-probe \
  src/AC1AffinityFix.c -o build/obj/AC1AffinityFix.obj
"$LLVM_BIN/lld-link" /dll /machine:x86 /nodefaultlib /entry:DllMain@12 /subsystem:windows /safeseh:no \
  build/obj/AC1AffinityFix.obj build/obj/kernel32.lib /out:dist/AC1AffinityFix.asi
echo "Built dist/AC1AffinityFix.asi"
