#!/bin/bash
# Unit tests and style check of /src in a separate build dir, output into /out/test.log.
set -u
mkdir -p /work/src
rsync -a --delete --exclude android /src/ /work/src/
cd /work
if [ ! -f buildtest/build.ninja ]; then
    cmake -S src -B buildtest -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DOE_BUILD_TESTS=ON -DOE_CHECK_STYLE=ON \
        -DOE_CHECK_TIDY=OFF -DOE_CHECK_LUA_STYLE=OFF > /out/test-configure.log 2>&1 || { tail -20 /out/test-configure.log; exit 1; }
fi
echo "== style" > /out/test.log
cmake --build buildtest --target check_style >> /out/test.log 2>&1 && echo "style ok" || echo "style FAILED"
echo "== unit tests" >> /out/test.log
cmake --build buildtest --target Run_UnitTest >> /out/test.log 2>&1 && echo "unit tests ok" || echo "unit tests FAILED"
