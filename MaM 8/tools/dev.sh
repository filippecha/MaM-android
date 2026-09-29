#!/bin/bash
# Host-side wrapper: incremental desktop build + MM8 test run. Args are passed to docker-desktop/dev.sh.
# Example: bash tools/dev.sh 15 "mousemove 549 31;click 1"
cd "$(dirname "$0")/.."
export MSYS_NO_PATHCONV=1
ARGS=$(printf ' %q' "$@")
docker run --rm --cap-add=SYS_PTRACE --security-opt seccomp=unconfined -v "D:\Projekty\Porty pro android\MaM 8\OpenEnroth:/src:ro" -v "D:\Projekty\Porty pro android\MaM 8\gamedata\MM8:/game:ro" \
    -v "D:\Projekty\Porty pro android\MaM 8\devout:/out" -v "D:\Projekty\Porty pro android\MaM 8\docker-desktop:/dd:ro" \
    -e DEV_GDB -e OE_DEV_MAP -e OE_DEV_HOUSE -e OE_DEV_DOORS -e OE_DEV_DECOR -e OE_DEV_AWARDS -e OE_DEV_POS -e OE_DEV_EVENT -e OE_DEV_HOURS -e OE_DEV_EXP -e OE_DEV_REP -e OE_DEV_TALK -e OE_DEV_ITEMS -e OE_DEV_OFFHAND -e OE_DEV_SPELL -e OE_DEV_GOLD -e OE_DEV_SUMMON -e OE_DEV_NEAR_MONSTER -e OE_DEV_MAP_TOUR -e OE_DEV_SPELL_SWEEP -e OE_DEV_ROSTER -v oe8_desk_work:/work oe-desktop bash -c "sed 's/\r$//' /dd/dev.sh > /tmp/dev.sh && bash /tmp/dev.sh$ARGS" 2>&1 \
    | grep -vE "Loaded shader|Reloading shaders|Shaders reloaded|ALSA lib|Sound id .* is used by both|Sprite .* not loaded"
