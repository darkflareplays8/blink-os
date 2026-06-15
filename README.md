# Blink OS

Fast. Clean. Yours.

A Debian-based Linux distribution built for people who want performance without the learning curve. Familiar like Windows, light enough to run on anything, no bloat.

---

## Downloads

Grab the latest release from the [Releases page](https://github.com/darkflareplays8/blink-os/releases).

| File | Use |
|------|-----|
| `.iso` | Flash to USB with [Balena Etcher](https://etcher.balena.io/) and boot from it |
| `.ova` | Import into [VirtualBox](https://www.virtualbox.org/) to try without installing |

---

## System requirements

- 64-bit CPU
- 512MB RAM minimum (1GB recommended)
- 8GB storage minimum

---

## What's included

- MATE desktop — Windows-like layout, taskbar at the bottom
- Nemo file manager — works like File Explorer
- Firefox ESR
- LibreOffice
- VLC
- GIMP
- Arc Dark theme + Papirus icons
- Full hardware firmware support out of the box
- Calamares graphical installer

---

## Building from source

You need Docker Desktop installed.

```bash
git clone https://github.com/darkflareplays8/blink-os
cd blink-os
docker build -t blink-os-builder ./build
mkdir output
docker run --privileged \
  -e VERSION=1.0.0 \
  -v $(pwd)/config:/config:ro \
  -v $(pwd)/output:/output \
  blink-os-builder
```

The ISO and OVA will appear in the `output/` folder.

---

## Releasing a new version

```bash
git tag v1.0.0
git push origin v1.0.0
```

GitHub Actions builds the ISO and OVA automatically and publishes them as a release.

---

## License

GPL-3.0
