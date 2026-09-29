#!/bin/bash
# Runs xdotool steps ('$W' is the game window) in the running reference session, then saves /out/<name>.png.
export DISPLAY=:99
W=$(cat /tmp/window)
IFS=';' read -ra STEPS <<< "${1:-}"
for step in "${STEPS[@]}"; do
    step=${step//\$W/$W}
    eval "xdotool $step"
done
sleep "${3:-1}"
import -window root "/out/${2:-live}.png"
