#!/bin/bash
# Android build of MaM 8, runs inside the openenroth-android image (see MaM 7/docker/Dockerfile).
# /src = OpenEnroth (read-only), /gamedata = folder with MM8/, /out = results.
set -euo pipefail

mkdir -p /work/src
rsync -a --delete \
    --exclude 'android/openenroth/.cxx' --exclude 'android/openenroth/build' \
    --exclude 'android/.gradle' --exclude 'android/build' \
    /src/ /work/src/

# Optional hook for throwaway test patches, applied to the working copy only.
if [ -f /out/prebuild.sh ]; then
    (cd /work/src && bash /out/prebuild.sh)
fi

cd /work/src/android
sed -i 's/\r$//' gradlew

# Personal signing key, kept in /out so that later builds can update the installed app.
export SIGNING_KEYSTORE=/out/keystore.jks
export SIGNING_STORE_PASSWORD=openenroth-local
export SIGNING_KEY_PASSWORD=openenroth-local
export SIGNING_KEY_ALIAS=openenroth
if [ ! -f "$SIGNING_KEYSTORE" ]; then
    keytool -genkeypair -keystore "$SIGNING_KEYSTORE" -storetype JKS -alias "$SIGNING_KEY_ALIAS" \
        -keyalg RSA -keysize 2048 -validity 10000 \
        -storepass "$SIGNING_STORE_PASSWORD" -keypass "$SIGNING_KEY_PASSWORD" \
        -dname "CN=MaM 8 local build"
fi

export GITHUBARCH="${GITHUBARCH:-arm64-v8a}"
export CMAKE_BUILD_PARALLEL_LEVEL="${CMAKE_BUILD_PARALLEL_LEVEL:-$(nproc)}"

GRADLE_ARGS=(assembleRelease --no-daemon --console=plain -Pandroid.native.buildOutput=verbose)
if [ -d /gamedata/MM8 ]; then
    # Only the files the engine reads: videos, music, LODs and MM8-Rel.exe for its tables. The backup keeps
    # files over 100 MB also as *.partNNN pieces, those must not end up in the APK.
    mkdir -p /work/gamedata/mm8
    rsync -a --delete --exclude '*.part[0-9][0-9][0-9]' --exclude '*.dll' --exclude '*.ini' \
        /gamedata/MM8/ /work/gamedata/mm8/
    GRADLE_ARGS+=(-PGAME_DATA_DIR=/work/gamedata)
fi

bash ./gradlew "${GRADLE_ARGS[@]}" 2>&1 | tee /out/build.log

cp -v openenroth/build/outputs/apk/release/openenroth-release.apk /out/MaM8.apk
