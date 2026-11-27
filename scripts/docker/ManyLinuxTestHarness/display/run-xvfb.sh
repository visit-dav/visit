#!/usr/bin/env bash
set -euo pipefail

display_number="${XVFB_DISPLAY_NUMBER:-99}"
screen="${XVFB_SCREEN:-1280x1024x24}"

# The runtime container mounts the same Docker-managed volume here.  Xorg
# requires the socket directory to be world-searchable with the sticky bit.
mkdir -p /tmp/.X11-unix
chmod 1777 /tmp/.X11-unix

exec Xvfb ":${display_number}" \
    -screen 0 "${screen}" \
    -ac \
    +extension GLX \
    +iglx \
    -nolisten tcp
