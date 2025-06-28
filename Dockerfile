FROM ubuntu:22.04

# Build tools
RUN apt-get update && apt-get install -y \
    build-essential \
    gcc \
    binutils \
    nasm \
    qemu-system-x86 \
    make \
    wget \
    # unzip \
    # git \
    # vim \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src

CMD ["/bin/bash"]
