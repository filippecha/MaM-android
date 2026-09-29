#!/bin/bash
# Runs the game under gdb until the main menu and prints the first C++ exception or crash with its backtrace.
cd "$(dirname "$0")/.."
DEV_GDB=1 timeout 3000 bash tools/dev.sh "${1:-25}" "${2:-key Escape;key Escape;key Escape;sleep 4}" > devout/first.txt 2>&1
grep -E "error:|FAILED" devout/first.txt | head -5
grep -E "critical|what\(\)|Assertion|SIGSEGV|SIGABRT" devout/gdb.log | head -3
grep -E "^#[0-9]+ " devout/gdb.log | grep -v "openal\|alc\.cpp\|gthr\|mutex\|stl_algo\|libc\|__cxa\|Exception::\|throw\|char_traits\|basic_string" | head -${3:-8}
ls -t devout/shot-*.png | head -1
