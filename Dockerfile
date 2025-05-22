ARG DEBIAN_RELEASE=bookworm
FROM debian:$DEBIAN_RELEASE
ENV DEBIAN_FRONTEND=noninteractive
ARG WXGTK_PACKAGE=libwxgtk3.2-dev
RUN apt-get update -yq \
    && apt-get install -yq --no-install-recommends \
        binutils-mingw-w64-i686 \
        build-essential \
        ca-certificates \
        curl \
        cxxtest \
        debhelper \
        devscripts \
        fakeroot \
        git \
        g++-mingw-w64-i686 \
        imagemagick \
        libcurl4-openssl-dev \
        libjsoncpp-dev \
        ${WXGTK_PACKAGE} \
        lsb-release \
        nsis \
        p7zip \
        pkg-config \
        wget \
    && apt-get clean \
    &&  rm -rf \
        /var/lib/apt/lists/* \
        /var/tmp/* \
        /tmp/*
ARG UID=1000
RUN mkdir -p /build/astercti
RUN chown -R $UID /build
USER $UID
WORKDIR /build/astercti
