# Desktop Linux Wine used to check that MM6.exe itself runs under Wine (the phone adds box64 on top).
FROM ubuntu:24.04
ENV DEBIAN_FRONTEND=noninteractive
RUN dpkg --add-architecture i386 && apt-get update && apt-get install -y --no-install-recommends \
        wget ca-certificates gnupg xvfb xdotool imagemagick zstd x11-utils cabextract \
    && mkdir -pm755 /etc/apt/keyrings \
    && wget -qO /etc/apt/keyrings/winehq-archive.key https://dl.winehq.org/wine-builds/winehq.key \
    && wget -qNP /etc/apt/sources.list.d/ https://dl.winehq.org/wine-builds/ubuntu/dists/noble/winehq-noble.sources \
    && apt-get update && apt-get install -y --install-recommends winehq-devel \
    && rm -rf /var/lib/apt/lists/*
ENV WINEDEBUG=-all
