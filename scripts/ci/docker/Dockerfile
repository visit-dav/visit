# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.
FROM ubuntu:bionic
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
    libxml2-dev \
    vim \
    emacs \
    bison \
    flex \
    cpio \
    locales \
 && rm -rf /var/lib/apt/lists/*

RUN localedef -i en_US -c -f UTF-8 -A /usr/share/locale/locale.alias en_US.UTF-8
ENV LANG en_US.utf8

RUN cd /usr/include && ln -s freetype2 freetype

#  -- Azure will add its own user, so skip this -- 
# RUN groupadd -r ci && useradd -ms /bin/bash --no-log-init -r -g ci ci
# USER ci
# WORKDIR /home/ci


# untar the current masonry source  (created as part of build_docker_visit_ci.py)
COPY visit.masonry.docker.src.tar /
RUN tar -xzf visit.masonry.docker.src.tar
# untar the current build_visit source (created as part of build_docker_visit_ci.py)
RUN mkdir -p /masonry/build-mb-develop-ci-smoke/visit/src/tools/dev/scripts/
COPY visit.build_visit.docker.src.tar /masonry
COPY masonry_docker_ci_cleanup.py /
RUN cd /masonry/build-mb-develop-ci-smoke/visit/src/tools/dev/scripts/ && tar -xzf /masonry/visit.build_visit.docker.src.tar

# ensure build_visit script is executable
RUN cd /masonry/build-mb-develop-ci-smoke/visit/src/tools/dev/scripts/ && chmod u+x build_visit

# call masonry to build tpls
RUN cd masonry && python bootstrap_visit.py opts/mb-develop-ci-smoke.json

# cleanup extract build dirs and obtain the config site file
RUN python masonry_docker_ci_cleanup.py
# change perms
RUN chmod -R a+rX /masonry

# add helper script to clone visit
RUN echo "git clone https://github.com/visit-dav/visit.git" > /clone.sh
RUN chmod +x /clone.sh

# add helper script to add cmake to path
RUN echo "export TPLS_PATH=/masonry/build-mb-develop-ci-smoke/thirdparty_shared/third_party/" > /env_setup.sh
RUN echo "export CMAKE_BIN_DIR=`ls -d ${TPLS_PATH}/cmake/*/ci/bin/`" >> /env_setup.sh
RUN echo "export PATH=$PATH:$CMAKE_BIN_DIR" >> /env_setup.sh
RUN chmod +x /env_setup.sh
