#!/bin/bash
set -e

DISTRO_NAME="blink-os"
DISTRO_VERSION="${VERSION:-1.0.0}"
ARCH="amd64"
BUILD_DIR="/build"
OUTPUT_DIR="/output"

mkdir -p "$OUTPUT_DIR"

apt-get update
apt-get install -y \
    live-build \
    debootstrap \
    squashfs-tools \
    xorriso \
    grub-pc-bin \
    grub-efi-amd64-bin \
    mtools \
    dosfstools

cd "$BUILD_DIR"

lb config \
    --architectures "$ARCH" \
    --distribution bookworm \
    --debian-installer live \
    --debian-installer-gui true \
    --archive-areas "main contrib non-free non-free-firmware" \
    --apt-recommends false \
    --binary-images iso-hybrid \
    --memtest none \
    --win32-loader false \
    --image-name "$DISTRO_NAME-$DISTRO_VERSION" \
    --bootappend-live "boot=live components quiet splash" \
    --linux-packages "linux-image-amd64 linux-headers-amd64"

cp /config/packages/packages.list "$BUILD_DIR/config/package-lists/blink.list.chroot"

mkdir -p "$BUILD_DIR/config/includes.chroot/etc/skel"
mkdir -p "$BUILD_DIR/config/includes.chroot/usr/share/blink"
mkdir -p "$BUILD_DIR/config/includes.chroot/etc/blink"

cp -r /config/desktop   "$BUILD_DIR/config/includes.chroot/usr/share/blink/"
cp -r /config/installer "$BUILD_DIR/config/includes.chroot/usr/share/blink/"

[ -d /config/branding ] && cp -r /config/branding "$BUILD_DIR/config/includes.chroot/usr/share/blink/"

cp /config/hooks/*.hook.chroot "$BUILD_DIR/config/hooks/normal/" 2>/dev/null || true

lb build

ISO_FILE=$(ls "$BUILD_DIR"/*.iso | head -1)
ISO_NAME="$DISTRO_NAME-$DISTRO_VERSION-$ARCH.iso"
cp "$ISO_FILE" "$OUTPUT_DIR/$ISO_NAME"

OVA_NAME="$DISTRO_NAME-$DISTRO_VERSION.ova"

VBoxManage createvm --name "BlinkOS" --ostype "Debian_64" --register 2>/dev/null && \
VBoxManage modifyvm "BlinkOS" \
    --memory 2048 \
    --cpus 2 \
    --vram 128 \
    --graphicscontroller vmsvga \
    --audio none \
    --usb on && \
VBoxManage createhd \
    --filename "/tmp/blink-disk.vdi" \
    --size 20480 \
    --format VDI && \
VBoxManage storagectl "BlinkOS" --name "SATA" --add sata --controller IntelAhci && \
VBoxManage storageattach "BlinkOS" \
    --storagectl "SATA" \
    --port 0 \
    --device 0 \
    --type hdd \
    --medium "/tmp/blink-disk.vdi" && \
VBoxManage storagectl "BlinkOS" --name "IDE" --add ide && \
VBoxManage storageattach "BlinkOS" \
    --storagectl "IDE" \
    --port 0 \
    --device 0 \
    --type dvddrive \
    --medium "$OUTPUT_DIR/$ISO_NAME" && \
VBoxManage export "BlinkOS" \
    --output "$OUTPUT_DIR/$OVA_NAME" \
    --ovf20 \
    --manifest \
    --options manifest && \
VBoxManage unregistervm "BlinkOS" --delete && \
echo "$OVA_NAME" > "$OUTPUT_DIR/ova_name.txt" || \
echo "WARNING: OVA generation skipped (VirtualBox kernel modules unavailable in Docker)" && \
echo "install-iso-only" > "$OUTPUT_DIR/ova_name.txt"

echo "$ISO_NAME" > "$OUTPUT_DIR/iso_name.txt"