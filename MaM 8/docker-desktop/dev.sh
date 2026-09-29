#!/bin/bash
# Incremental desktop build of /src (engine) and a test run with game data from /game.
# Usage: dev.sh <seconds to run> [xdotool commands separated by ';' executed after startup]
set -u
mkdir -p /work/src
rsync -a --delete --exclude android /src/ /work/src/
cd /work
# Bounds checks of std::array and friends catch data files that are bigger than the tables they are read into.
if [ ! -f build/build.ninja ] || ! grep -q _GLIBCXX_ASSERTIONS build/CMakeCache.txt; then
    cmake -S src -B build -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_CXX_FLAGS=-D_GLIBCXX_ASSERTIONS -DOE_BUILD_TESTS=OFF -DOE_CHECK_STYLE=OFF \
        -DOE_CHECK_TIDY=OFF -DOE_CHECK_LUA_STYLE=OFF > /out/configure.log 2>&1 || { tail -20 /out/configure.log; exit 1; }
fi
if ! cmake --build build > /out/build.log 2>&1; then
    grep -E "error:|FAILED:" /out/build.log | head -30
    exit 1
fi
echo "build ok"
printf "pcm.!default { type null }
ctl.!default { type hw card 0 }
" > ~/.asoundrc
Xvfb :99 -screen 0 640x480x24 >/dev/null 2>&1 &
sleep 2
export DISPLAY=:99 LIBGL_ALWAYS_SOFTWARE=1 OPENENROTH_MM7_PATH=/game
rm -rf /root/.openenroth
rm -f /out/shot-*.png
if [ -n "${DEV_GDB:-}" ]; then
    # Backtraces into /out/gdb.log: of C++ exceptions for DEV_GDB=1, otherwise DEV_GDB is a breakpoint like "AudioPlayer.cpp:364".
    STOP="catch throw"
    [ "$DEV_GDB" != 1 ] && STOP="break $DEV_GDB"
    printf "set pagination off\n$STOP\ncommands\nbt 12\ncontinue\nend\nrun\nbt\n" > /tmp/gdb.cmd
    gdb -batch -x /tmp/gdb.cmd ./build/src/Bin/OpenEnroth/OpenEnroth > /out/gdb.log 2>&1 &
else
    ./build/src/Bin/OpenEnroth/OpenEnroth > /out/run.log 2>&1 &
fi
PID=$!
T=${1:-20}
sleep "$T"
import -window root /out/shot-1.png 2>/dev/null
i=2
IFS=';' read -ra STEPS <<< "${2:-}"
for step in "${STEPS[@]}"; do
    if [[ $step == !* ]]; then # "!cmd" runs a shell command right away, e.g. "!import -window root /out/fast-1.png".
        eval "${step:1}"
        continue
    fi
    eval "xdotool $step"
    sleep 3
    import -window root /out/shot-$i.png 2>/dev/null
    i=$((i+1))
done
if kill -0 $PID 2>/dev/null; then echo "still running"; kill $PID; else wait $PID; echo "exited with $?"; fi
grep -vE "Sprite .* not loaded|OpenAL: error" /out/run.log | tail -25
