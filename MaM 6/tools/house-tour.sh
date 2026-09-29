#!/bin/bash
# Enters every given house of New Sorpigal in a fresh game and saves a screenshot of its main menu into
# devout/house-<id>.png. Example: bash tools/house-tour.sh 48 57 69 79
cd "$(dirname "$0")/.."
for house in "$@"; do
    OE_DEV_HOUSE=$house timeout 600 bash tools/dev.sh 20 "$(cat tools/newgame-steps.txt);sleep 4;mousemove 300 300;sleep 1;mousemove 556 70 click 1;sleep 2" 2>&1 \
        | grep -iE "error|assert|fault" | head -5 | sed "s/^/house $house: /"
    last=$(ls -t devout/shot-*.png | head -1)
    prev=$(ls -t devout/shot-*.png | sed -n 3p)
    cp "$prev" "devout/house-$house.png"
    cp "$last" "devout/house-$house-b.png"
done
