# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.
FROM ubuntu:xenial
MAINTAINER Cyrus Harrison <cyrush@llnl.gov>

# fetch build env
RUN apt-get update && apt-get install -y \
    git \
    wget \
    curl \
    p7zip \
    unzip \
    subversion \
    build-essential \
    gcc \
    g++ \
    gfortran \
    zlib1g-dev \
    python \
    libsm-dev \
    libice-dev \
    libssl-dev \
    libx11-xcb-dev \
    libxcb-dri2-0-dev \
    libxcb-xfixes0-dev \
    libffi-dev \
    xutils-dev \
    xorg-dev \
    libfreetype6-dev \
    autoconf \
    libtool \
    m4 \
    automake \
    libxml2 \
    vim \
    emacs \
 && rm -rf /var/lib/apt/lists/*

RUN cd /usr/include && ln -s freetype2 freetype

RUN groupadd -r ci && useradd -ms /bin/bash --no-log-init -r -g ci ci
USER ci
WORKDIR /home/ci

# untar the current masonry source  (created as part of build_docker_visit_ci.py)
COPY visit.masonry.docker.src.tar /home/ci
RUN tar -xzf visit.masonry.docker.src.tar
# untar the current build_visit source (created as part of build_docker_visit_ci.py)
RUN mkdir -p /home/ci/masonry/build-mb-develop-ci-smoke/visit/src/tools/dev/scripts/
COPY visit.build_visit.docker.src.tar /home/ci/masonry
COPY masonry_docker_ci_cleanup.py /home/ci
RUN cd /home/ci/masonry/build-mb-develop-ci-smoke/visit/src/tools/dev/scripts/ && tar -xzf /home/ci/masonry/visit.build_visit.docker.src.tar

# call masonry to build tpls
RUN cd masonry && python bootstrap_visit.py opts/mb-develop-ci-smoke.json

# cleanup extract build dirs and obtain the config site file
RUN python masonry_docker_ci_cleanup.py
