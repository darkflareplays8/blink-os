FROM debian:bookworm

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    live-build \
    debootstrap \
    squashfs-tools \
    xorriso \
    grub-pc-bin \
    grub-efi-amd64-bin \
    mtools \
    dosfstools \
    curl \
    wget \
    git \
    python3 \
    python3-pip \
    virtualbox \
    virtualbox-ext-pack \
    && rm -rf /var/lib/apt/lists/*

RUN mkdir -p /build /output /config

WORKDIR /build

COPY build.sh /build/build.sh
RUN chmod +x /build/build.sh

CMD ["/build/build.sh"]
