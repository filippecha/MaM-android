#!/bin/bash
# Reference screenshots of the original MM6.exe under Wine (builtin ddraw), used only to copy the MM6 UI layout.
# Usage: ref.sh "<xdotool steps separated by ';'>" - a screenshot is taken after each step into /out/ref-N.png.
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
import -window root /out/ref-1.png
i=2
IFS=';' read -ra STEPS <<< "${1:-}"
for step in "${STEPS[@]}"; do
    step=${step//\$W/$W}
    eval "xdotool $step"
    sleep 3
    import -window root /out/ref-$i.png
    i=$((i+1))
done
