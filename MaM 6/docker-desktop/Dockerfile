# Desktop Linux build of OpenEnroth with a virtual screen, used to develop and test MM6 support on the PC.
FROM ubuntu:24.04
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
        git wget ca-certificates curl gcc-14 g++-14 ninja-build pkg-config python3 python3-pil rsync \
        libgl-dev libx11-dev libxext-dev libxrandr-dev libxcursor-dev libxi-dev libxfixes-dev \
        libwayland-dev libxkbcommon-dev libasound2-dev libpulse-dev \
        xvfb mesa-utils libgl1-mesa-dri xdotool imagemagick ccache lld \
    && rm -rf /var/lib/apt/lists/*
RUN wget -qO /tmp/cmake.tgz https://github.com/Kitware/CMake/releases/download/v3.31.8/cmake-3.31.8-linux-x86_64.tar.gz \
    && mkdir -p /opt/cmake && tar -xzf /tmp/cmake.tgz -C /opt/cmake --strip-components=1 && rm /tmp/cmake.tgz
ENV PATH=/opt/cmake/bin:$PATH
ENV CC=gcc-14 CXX=g++-14
RUN git config --global --add safe.directory '*'
RUN apt-get update && apt-get install -y --no-install-recommends gdb && rm -rf /var/lib/apt/lists/*
