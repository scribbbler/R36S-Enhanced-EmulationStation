FROM ubuntu:19.10
ENV DEBIAN_FRONTEND=noninteractive

# eoan is EOL; point apt at old-releases (which still hosts arm64 for eoan)
RUN sed -i \
    -e 's|http://ports.ubuntu.com/ubuntu-ports|http://old-releases.ubuntu.com/ubuntu|g' \
    -e 's|http://archive.ubuntu.com/ubuntu|http://old-releases.ubuntu.com/ubuntu|g' \
    -e 's|http://security.ubuntu.com/ubuntu|http://old-releases.ubuntu.com/ubuntu|g' \
    /etc/apt/sources.list \
 && apt-get update

# dependency list from christianhaitian/rk3326_core_builds scripts/es_build.sh
RUN apt-get install -y --no-install-recommends \
    build-essential git wget ca-certificates cmake pkg-config \
    libboost-system-dev libboost-filesystem-dev libboost-locale-dev libboost-date-time-dev \
    libfreeimage-dev libfreetype6-dev libeigen3-dev libcurl4-openssl-dev libasound2-dev \
    libsdl2-dev libsdl2-mixer-dev rapidjson-dev libvlc-dev libvlccore-dev vlc-bin \
 && rm -rf /var/lib/apt/lists/*

# Rockchip Mali GLES headers/libs (same package the official build uses)
RUN wget -q "https://github.com/christianhaitian/EmulationStation-fcamod/raw/master/libmali-rk-dev_1.7-1%2Bdeb10_arm64.deb" -O /tmp/libmali.deb \
 && dpkg -i --force-all /tmp/libmali.deb && rm /tmp/libmali.deb

# libgo2: headers from christianhaitian/libgo2, library taken from the device rootfs
COPY libgo2-src/src/*.h /usr/include/go2/
COPY libgo2-lib/libgo2.so /usr/lib/aarch64-linux-gnu/libgo2.so

# Mali GPU blob from the device rootfs; point the GL link names at it so the
# binary links the same GL stack as stock (NEEDED libMali.so / lib*.so names,
# not Mesa's versioned SONAMEs which resolve to software GL on the device)
COPY libgo2-lib/libmali-bifrost-g31-rxp0-wayland-gbm.so /usr/lib/aarch64-linux-gnu/
RUN cd /usr/lib/aarch64-linux-gnu \
 && ln -sf libmali-bifrost-g31-rxp0-wayland-gbm.so libMali.so \
 && ln -sf libMali.so libEGL.so \
 && ln -sf libMali.so libGLESv1_CM.so \
 && ln -sf libMali.so libGLESv2.so

WORKDIR /work
