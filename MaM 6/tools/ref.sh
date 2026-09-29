#!/bin/bash
# Host-side wrapper for docker-wine/ref.sh: reference screenshots of the original MM6 into devout/ref-N.png.
# Example: bash tools/ref.sh 'mousemove --window $W 570 30 click 1;mousemove --window $W 180 452 click 1'
cd "$(dirname "$0")/.."
export MSYS_NO_PATHCONV=1
ARGS=$(printf ' %q' "$@")
docker run --rm -v "D:\Projekty\Porty pro android\MaM 6\gamedata\MM6:/game:ro" -v "D:\Projekty\Porty pro android\MaM 6\devout:/out" \
    -v "D:\Projekty\Porty pro android\MaM 6\docker-wine:/dw:ro" \
    mm6-wine bash -c "sed 's/\r$//' /dw/ref.sh > /tmp/ref.sh && bash /tmp/ref.sh$ARGS" 2>&1 | tail -5
