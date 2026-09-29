#!/bin/bash
# Interactive reference session of the original MM6.exe under Wine: starts the game and keeps it running, control it
# with `docker exec mm6ref /dw/ref-cmd.sh "<xdotool steps>" <shot name>`.
set -u
export WINEPREFIX=/tmp/pfx WINEARCH=win64 DISPLAY=:99 WINEDLLOVERRIDES="mscoree,mshtml="
printf "pcm.!default { type null }
ctl.!default { type hw card 0 }
" > ~/.asoundrc
Xvfb :99 -screen 0 640x480x24 >/dev/null 2>&1 &
sleep 2
timeout 120 wineboot -i >/dev/null 2>&1; timeout 60 wineserver -w
cp -r /game /tmp/pfx/drive_c/MM6
mkdir -p /tmp/cnc && tar -I zstd -xf /dw/cnc-ddraw/ddraw.tzst -C /tmp/cnc && cp /tmp/cnc/syswow64/ddraw.dll /tmp/pfx/drive_c/MM6/
cp /dw/cnc-ddraw/ddraw-used.ini /tmp/pfx/drive_c/MM6/ddraw.ini
export WINEDLLOVERRIDES="mscoree,mshtml=;ddraw=n,b"
cd /tmp/pfx/drive_c/MM6
wine explorer /desktop=mm6,640x480 MM6.exe -nomovie > /out/ref-wine.log 2>&1 &
sleep 12
W=$(xdotool search --name "Wine desktop" | head -1)
echo "$W" > /tmp/window
sleep infinity
