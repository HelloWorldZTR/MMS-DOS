FROM ubuntu:jammy

ENV DEBIAN_FRONTEND=noninteractive

RUN cp /etc/apt/sources.list /etc/apt/sources.list.bak && \
    sed -i 's/archive.ubuntu.com/mirrors.aliyun.com/g' /etc/apt/sources.list && \
    sed -i 's/security.ubuntu.com/mirrors.aliyun.com/g' /etc/apt/sources.list

WORKDIR /work
ENV INFOPATH=/usr/ia16-elf/info:/usr/share/info
RUN apt-get update && apt-get upgrade -y && apt-get install -y \
		software-properties-common install-info \
	&& add-apt-repository ppa:tkchia/build-ia16 \
	&& apt-get update \
	&& apt-get install -y \
		gcc-ia16-elf jwasm jwlink make info \
	&& cd /usr/ia16-elf/info \
	&& for file in *; do install-info $file dir; done \
	&& apt-get purge -y \
		software-properties-common \
	&& apt-get autoremove -y --purge && rm -rf /var/cache/apt
RUN apt install -y build-essential \
    nasm\
	dosfstools
# 工作目录
WORKDIR /src