# Building for the R36S

The device runs an **Ubuntu 19.10 (eoan) arm64** userland, so the binary is
built in a matching container. On an Apple Silicon Mac this means an arm64 VM
(e.g. [Colima](https://github.com/abiosoft/colima)) so Docker runs native arm64;
on an arm64 Linux box Docker is native already.

## 1. Device blobs (not included — extract from your own device)

Two files are pulled from the device's own rootfs and are **not redistributable**
(the Mali driver is proprietary ARM firmware). Copy them out of your device's
root filesystem (e.g. from a card backup) and drop them next to the Dockerfile:

```
docker/libgo2-lib/libgo2.so
docker/libgo2-lib/libmali-bifrost-g31-rxp0-wayland-gbm.so
```

The **go2 headers** come from christianhaitian's libgo2 source:

```
git clone https://github.com/christianhaitian/libgo2
cp -r libgo2/src docker/libgo2-src/src   # provides src/*.h
```

The Dockerfile installs the headers, `libgo2.so`, and the Mali blob, then
symlinks `libEGL.so`/`libGLESv2.so` to the Mali driver so the binary's `NEEDED`
records match the stock build (Mali, not Mesa software GL).

## 2. Build the image and compile

```sh
docker build -t es-build:eoan r36s/docker/
# from the repo root (the ES source):
docker run --rm -v "$PWD":/src es-build:eoan bash -lc '
  cp -r /src /work && cd /work &&
  cmake -DGLES=ON . && make -j"$(nproc)" emulationstation &&
  aarch64-linux-gnu-strip -s emulationstation -o /src/emulationstation.custom
'
```

> Build **inside the container FS** (as above), not directly on an exFAT bind
> mount — cmake fails on virtiofs/exFAT.

The result is `emulationstation.custom`. Preflight it by `chroot`‑ing into a
loop‑mounted device rootfs (native arm64) and checking `ldd` reports 0 missing
libraries and `--help` exits 0, then install it with **Install Custom ES**.

## Notes

- `theme_probe.cpp` is a small host‑side utility that parses a `theme.xml`
  through the real `ThemeData` parser (link against `libes-core.a`) — handy for
  debugging theme elements without deploying.
- The `apt` sources must point at `old-releases.ubuntu.com/ubuntu` (eoan is EOL);
  the Dockerfile already does this.
