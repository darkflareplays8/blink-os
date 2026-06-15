FROM debian:bookworm

ENV DEBIAN_FRONTEND=noninteractive
ENV APT_CONFIG=/etc/apt/apt.conf.d/99parallel

RUN echo 'Acquire::Queue-Mode "access";' > /etc/apt/apt.conf.d/99parallel && \
    echo 'Acquire::http::Pipeline-Depth "10";' >> /etc/apt/apt.conf.d/99parallel && \
    echo 'Acquire::Languages "none";' >> /etc/apt/apt.conf.d/99parallel

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
    aria2 \
    gnupg \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

RUN curl -fsSL https://www.virtualbox.org/download/oracle_vbox_2016.asc \
    | gpg --dearmor -o /usr/share/keyrings/oracle-virtualbox.gpg && \
    echo "deb [arch=amd64 signed-by=/usr/share/keyrings/oracle-virtualbox.gpg] \
    https://download.virtualbox.org/virtualbox/debian bookworm contrib" \
    > /etc/apt/sources.list.d/virtualbox.list && \
    apt-get update && apt-get install -y virtualbox-7.1 \
    && rm -rf /var/lib/apt/lists/*

RUN mkdir -p /build /output /config

WORKDIR /build

COPY build.sh /build/build.sh
RUN chmod +x /build/build.sh

CMD ["/build/build.sh"]