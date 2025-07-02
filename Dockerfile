FROM ubuntu:noble

ENV DEBIAN_FRONTEND=noninteractive

# RUN cp /etc/apt/sources.list /etc/apt/sources.list.bak && \
#     sed -i 's/archive.ubuntu.com/mirrors.aliyun.com/g' /etc/apt/sources.list && \
#     sed -i 's/security.ubuntu.com/mirrors.aliyun.com/g' /etc/apt/sources.list

WORKDIR /work

ENV INFOPATH=/usr/ia16-elf/info:/usr/share/info

RUN apt-get update && apt-get upgrade -y && apt-get install -y \
		software-properties-common install-info

RUN sh -c '\
  ARCH=$(uname -m); \
  case "$ARCH" in \
    x86_64) \
      echo "Installing gcc-ia16-elf for x86_64..." && \
      apt-get update && apt-get install -y software-properties-common && \
      add-apt-repository ppa:tkchia/build-ia16 && \
      apt-get update && \
      apt-get install -y \
        gcc-ia16-elf jwasm jwlink make info && \
      cd /usr/ia16-elf/info && \
      for file in *; do install-info "$file" dir; done && \
      apt-get purge -y software-properties-common && \
      apt-get autoremove -y --purge && rm -rf /var/cache/apt \
      ;; \
    aarch64) \
      echo "Installing gcc-ia16-elf for ARM64..." && \
      apt-get update && apt-get install -y software-properties-common && \
      add-apt-repository ppa:catacombae/gcc-ia16-arm64 && \
      apt-get update && \
      apt-get install -y \
        gcc-ia16-elf make info && \
      cd /usr/ia16-elf/info && \
      for file in *; do install-info "$file" dir; done && \
      apt-get purge -y software-properties-common && \
      apt-get autoremove -y --purge && rm -rf /var/cache/apt \
      ;; \
    *) \
      echo "Unsupported architecture: $ARCH" && exit 1 \
      ;; \
  esac'


RUN apt install -y build-essential \
    nasm\
	dosfstools

# Working directory
WORKDIR /src