# VisIt release containers

Each container builds VisIt's third-party libraries, builds and packages VisIt,
installs the package, and runs a smoke test. The version is inferred from a
`visitMAJOR_MINOR_PATCH.src.tar.gz` archive and checked against its matching
`build_visitMAJOR_MINOR_PATCH` script. No script or Dockerfile contains a VisIt
release number.

The third-party build uses two Docker layers. The first builds the core
libraries; the second repeats the same `build_visit` command with `--optional`.
If an optional library fails, Docker can reuse the completed core-library
layer on the next build.

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

The Ubuntu Dockerfile supports 22.04 and 24.04 through `UBUNTU_VERSION`. The
Rocky Dockerfile supports 8.9 and 9.3 through `ROCKY_VERSION`. Debian and
Fedora accept `DEBIAN_VERSION` and `FEDORA_VERSION`, respectively:

```console
python3 create_release.py --source "$src" --build-visit "$bv" --platform ubuntu --os-version 22.04
python3 create_release.py --source "$src" --build-visit "$bv" --platform rocky --os-version 8.9
python3 create_release.py --source "$src" --build-visit "$bv" --platform debian --os-version 12
python3 create_release.py --source "$src" --build-visit "$bv" --platform fedora --os-version 44
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
created. Run `python3 create_release.py --help` for output directory, image tag,
and artifact label options.
