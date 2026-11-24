# VisIt release containers

Each container builds VisIt's third-party libraries, builds and packages VisIt,
installs the package, and runs a smoke test. The version is inferred from a
`visitMAJOR_MINOR_PATCH.src.tar.gz` archive and checked against its matching
`build_visitMAJOR_MINOR_PATCH` script. No script or Dockerfile contains a VisIt
release number.

The third-party build uses two Docker layers. The first builds the core
libraries; the second repeats the same `build_visit` command with `--optional`.
If an optional library fails, Docker can reuse the completed core-library
layer on the next build. Both invocations use `build_visit --cleanup`, which
removes successfully built third-party source and build directories.
The core build supplies XCB and xkbcommon explicitly so Qt cannot silently
configure without the X11 platform integration required by VisIt.

Downloaded tarballs are removed only after VisIt has been built, installed,
and smoke-tested successfully. The binary VisIt package is retained for
extraction from the completed image.

The recommended interface is `create_release.py`. It accepts the two inputs
from any location and creates an automatically cleaned temporary Docker build
context, so nothing needs to be copied into this checkout. For example:

```console
python3 /path/to/visit/scripts/docker/create_release.py \
  --source /path/to/visit3_6_0.src.tar.gz \
  --build-visit /path/to/build_visit3_6_0 \
  --platform ubuntu --os-version 24.04
```

If source and build scripts are in current directory, you can omit the path for those arguments:

```console
python3 /path/to/visit/scripts/docker/create_release.py \
  --source visit3_6_0.src.tar.gz \
  --build-visit build_visit3_6_0 \
  --platform ubuntu --os-version 24.04
```

The Ubuntu Dockerfile supports 22.04, 24.04, and 26.04 through
`UBUNTU_VERSION`. The Rocky Dockerfile supports 8.9 and 9.3 through
`ROCKY_VERSION`. Debian and Fedora accept `DEBIAN_VERSION` and
`FEDORA_VERSION`, respectively. The manylinux Dockerfile defaults to the
`manylinux_2_28_x86_64` image and accepts `MANYLINUX_VERSION`. Fedora 44 is the
default Fedora release:

```console
python3 create_release.py --source "$src" --build-visit "$bv" --platform ubuntu --os-version 22.04
python3 create_release.py --source "$src" --build-visit "$bv" --platform rocky --os-version 8.9
python3 create_release.py --source "$src" --build-visit "$bv" --platform debian --os-version 12
python3 create_release.py --source "$src" --build-visit "$bv" --platform fedora --os-version 44
python3 create_release.py --source "$src" --build-visit "$bv" --platform manylinux --os-version 2_28
```

Use `--engine podman` for Podman. On LC, first enable Podman in the calling
shell, then preserve the mappings used by the former Ubuntu release script:

```console
/collab/usr/gapps/lcweg/containers/scripts/enable-podman.sh
python3 /path/to/visit/scripts/docker/create_release.py \
  --source "$src" --build-visit "$bv" \
  --engine podman --platform ubuntu --os-version 22.04 \
  --userns-uid-map 0:0:1 --userns-uid-map 1:1:1999 \
  --userns-uid-map 65534:2047:1
```

To build an image without extracting its package and smoke-test image, add
`--build-only`. Use `--temp-dir` to choose where the temporary build context is
created. The extracted smoke-test image includes the platform label in its
name, for example `visit0000-fedora44.png`. Run
`python3 create_release.py --help` for output directory, image tag, and artifact
label options.

## Recovering failed builds

Add `--recoverable` to run the core third-party, optional third-party, and VisIt
build/test steps in named containers. Each successful step creates a visible
checkpoint image. If a step fails, the driver commits its partial filesystem to
an image ending in `-core-failed`, `-optional-failed`, or `-visit-failed` and
retains the stopped container. It also copies the `build_visit` log into
`--output-dir` and appends the platform label to its name, for example
`build_visit3_6_0_log_rocky8`.

For example:

```console
python3 create_release.py --recoverable \
  --source "$src" --build-visit "$bv" \
  --platform rocky --os-version 8.9
```

The failure message gives the exact container and image names. A missing source
archive can be supplied without losing completed work:

```console
docker cp conduit.tar.gz CONTAINER:/home/visit/
docker commit CONTAINER RECOVERY_IMAGE
python3 create_release.py --recoverable \
  --resume-from RECOVERY_IMAGE --resume-step optional
```

Recovery checkpoints contain the VisIt source archive, generated `build_visit`
script, platform metadata, release tag, and version. Consequently, resume does
not require the original `--platform`, `--os-version`, `--source`, or
`--build-visit` arguments. Recovery images created by older versions of this
driver may still require `--source` if the source archive is absent.

The VisIt build records extraction, configuration, packaging, installation,
and testing milestones in the image. Resuming a failed VisIt step reuses each
completed milestone and performs an incremental build when compilation failed.
