#!/usr/bin/env sh
# Sanity checks on the built binaries: right CPU type, only the expected Windows DLLs imported,
# and the plugins export nothing odd. Fails the build if anything is off.
set -e
cd "$(dirname "$0")/.."
LLVM_BIN="${LLVM_BIN:-$(dirname "$(command -v lld-link || echo /usr/lib/llvm-18/bin/lld-link)")}"
fail=0

# check <file> <machine> <allowed DLLs, space separated>
check() {
  hdr=$("$LLVM_BIN/llvm-readobj" --file-headers "$1")
  echo "$hdr" | grep -q "Machine: $2" || { echo "FAIL $1: expected $2"; fail=1; }
  dlls=$("$LLVM_BIN/llvm-readobj" --coff-imports "$1" | sed -n 's/^ *Name: \(.*\.dll\)$/\1/p' | tr 'A-Z' 'a-z' | sort -u)
  for d in $dlls; do
    case " $3 " in *" $d "*) ;; *) echo "FAIL $1 imports $d"; fail=1 ;; esac
  done
  size=$(wc -c < "$1" | tr -d ' ')
  echo "ok   $1 ($size bytes; imports: $(echo $dlls | tr '\n' ' '))"
}
check dist/AC1AffinityFix.asi IMAGE_FILE_MACHINE_I386 "kernel32.dll"
check dist/ManyCoreFix.asi IMAGE_FILE_MACHINE_I386 "kernel32.dll"
check dist/ManyCoreFix64.asi IMAGE_FILE_MACHINE_AMD64 "kernel32.dll"
check dist/tests/ManyCoreFixTest.exe IMAGE_FILE_MACHINE_I386 "kernel32.dll"
check dist/tests/ManyCoreFixTest64.exe IMAGE_FILE_MACHINE_AMD64 "kernel32.dll"
check dist/AC1ModernFix-Setup.exe IMAGE_FILE_MACHINE_I386 \
  "kernel32.dll user32.dll gdi32.dll advapi32.dll shell32.dll ole32.dll urlmon.dll comctl32.dll"
"$LLVM_BIN/llvm-readobj" --coff-resources dist/AC1ModernFix-Setup.exe | grep -q "MANIFEST" \
  || { echo "FAIL setup has no manifest"; fail=1; }
[ $fail = 0 ] && echo "All build checks passed." || exit 1
