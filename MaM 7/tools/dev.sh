#!/bin/bash
# Host-side wrapper: incremental desktop build of MaM 7 + test run with the MM7 data. Args are passed to docker-desktop/dev.sh.
# Example: OE_DEV_MAP=21 bash tools/dev.sh 25 "mousemove 320 240;click 1"
cd "$(dirname "$0")/.."
export MSYS_NO_PATHCONV=1
ARGS=$(printf ' %q' "$@")
docker run --rm -v "D:\Projekty\Porty pro android\MaM 7\OpenEnroth:/src:ro" -v "D:\Projekty\Porty pro android\MaM 7\gamedata\mm7:/game:ro" \
    -v "D:\Projekty\Porty pro android\MaM 7\devout7:/out" -v "D:\Projekty\Porty pro android\MaM 7\docker-desktop:/dd:ro" \
    -e OE_DEV_MAP -e OE_DEV_POS -e OE_DEV_LOGPOS -e OE_DEV_DUMP -e OE_DEV_QBIT -v oe7_desk_work:/work oe-desktop bash -c "sed 's/\r$//' /dd/dev.sh > /tmp/dev.sh && bash /tmp/dev.sh$ARGS" 2>&1 \
    | grep -vE "Loaded shader|Reloading shaders|Shaders reloaded|ALSA lib|Sound id .* is used by both|Sprite .* not loaded"
