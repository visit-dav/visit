# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.
FROM ubuntu:18.04
MAINTAINER Cyrus Harrison <cyrush@llnl.gov>

# fetch build env
RUN apt-get update && apt-get install -y \
    git \
    git-lfs \
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
    p7zip-full \
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

# fix for issue with freetype headers
RUN cd /usr/include && ln -s freetype2 freetype

# untar the current masonry source  (created as part of build_docker_visit_ci.py)
COPY visit.masonry.docker.src.tar /
RUN tar -xzf visit.masonry.docker.src.tar

# so add them as env entries to make things a little more tractable
ENV OPTS_FILE mb-develop-ci-smoke-py3.json
# use standard build dir (we expect this to be set in the json opts via build_dir)
ENV BUILD_DIR build-mb-develop-ci-smoke

# untar the current build_visit source (created as part of build_docker_visit_ci.py)
# TODO THE DIR HAS UNIQUE NAME BASED ON opts/mb-develop-ci-smoke-py2.json
RUN mkdir -p /masonry/${BUILD_DIR}/visit/src/tools/dev/scripts/
COPY visit.build_visit.docker.src.tar /masonry
RUN cd /masonry/${BUILD_DIR}/visit/src/tools/dev/scripts/ && tar -xzf /masonry/visit.build_visit.docker.src.tar

# copy in clean up script
COPY masonry_docker_ci_cleanup.py /

# make sure perms are ok in our container
RUN chmod -R a+rX /masonry

# call masonry to build tpls
RUN cd masonry && python bootstrap_visit.py opts/${OPTS_FILE}

# cleanup extract build dirs and obtain the config site file
RUN python masonry_docker_ci_cleanup.py

# change perms for tpls
RUN chmod -R a+rX /masonry

