# Linux build environment for the OpenEnroth Android APK, mirrors the upstream CI setup.
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
        openjdk-17-jdk-headless git cmake ninja-build build-essential \
        unzip wget ca-certificates python3 rsync \
    && rm -rf /var/lib/apt/lists/*

ENV ANDROID_HOME=/opt/android-sdk
ENV ANDROID_SDK_ROOT=/opt/android-sdk
ENV JAVA_HOME=/usr/lib/jvm/java-17-openjdk-amd64

RUN mkdir -p $ANDROID_HOME/cmdline-tools && cd /tmp \
    && wget -q https://dl.google.com/android/repository/commandlinetools-linux-11076708_latest.zip -O clt.zip \
    && unzip -q clt.zip && mv cmdline-tools $ANDROID_HOME/cmdline-tools/latest && rm clt.zip

# License acceptance files copied from the user's existing Android Studio SDK.
COPY licenses/ $ANDROID_HOME/licenses/

RUN $ANDROID_HOME/cmdline-tools/latest/bin/sdkmanager \
        "platform-tools" "platforms;android-31" "build-tools;30.0.3" "ndk;28.1.13356709" > /dev/null

# The repo needs CMake 3.31+, Ubuntu 24.04 ships 3.28.
RUN apt-get purge -y cmake cmake-data && mkdir -p /opt/cmake \
    && wget -q https://github.com/Kitware/CMake/releases/download/v3.31.8/cmake-3.31.8-linux-x86_64.tar.gz -O /tmp/cmake.tgz \
    && tar -xzf /tmp/cmake.tgz -C /opt/cmake --strip-components=1 && rm /tmp/cmake.tgz
ENV PATH=/opt/cmake/bin:$PATH

RUN git config --global --add safe.directory '*'

COPY build.sh /build.sh
RUN sed -i 's/\r$//' /build.sh && chmod +x /build.sh
CMD ["/build.sh"]
