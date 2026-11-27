# VisIt Many-Linux Runtime Tests

This harness installs and tests an existing versioned VisIt many-Linux binary
distribution on Rocky Linux 8/9, Fedora 44, Debian 13, and Ubuntu
20.04/22.04/24.04/26.04. It does not build VisIt.

The base profile contains the X11, XCB, XKB, font, and GL runtime libraries
required by the default GUI-capable VisIt build. It does not contain compilers
or development headers.

For the current VisIt 3.6.0 manylinux cycle, those graphical libraries are an
intentional host prerequisite. Each profile image represents a supported host;
the distribution is not currently required to run on a minimal image without
X11. The ELF audit may report host-resolved graphical libraries for inventory,
but that is not a packaging defect under this policy.

The Dockerfiles use official `rockylinux`, `fedora`, `debian`, and `ubuntu`
base-image tags. The Fedora and Debian profiles are pinned to releases 44 and
13, respectively. For a fully reproducible CI job, those tags can later be
paired with tested image digests.

## Inputs

Put these files in one artifact directory:

- One `visitMAJOR_MINOR_PATCH*.tar.gz`, `.tgz`, `.tar.xz`, or `.tar.bz2`
  archive, such as `visit3_6_0.linux-x86_64.tar.gz`.
- The matching `visit-install` script.

The installer is run as:

```sh
visit-install -c none -f ARCHIVE /opt/visit
```

The harness derives `VISIT_VERSION` by converting the numeric underscore-
separated version immediately following `visit` in the binary archive name
(for example, `visit3_7_1...` becomes `3.7.1`). Source archives are ignored.

The container includes `cpio` because `visit-install` uses it internally to
copy the extracted distribution. The harness validates the installed Python
and launcher afterward because `visit-install` can return success even when
an internal copy command fails.

## Run the tests

The only host-side entry points are `run-linux-tests.py` for one profile and
`run-linux-matrix.py` for several profiles. They use the same named options on
Windows, Linux, and macOS. Use Python 3 from `PATH` or specify its complete
path.

Run one profile:

```sh
python3 /path/to/visit/scripts/docker/ManyLinuxTestHarness/run-linux-tests.py --profile rocky8 --artifact-directory /path/to/visit-artifacts --mode core
```

For example, when Python is not in `PATH` on Windows:

```powershell
C:\path\to\python.exe C:\path\to\visit\scripts\docker\ManyLinuxTestHarness\run-linux-tests.py --profile rocky8 --artifact-directory C:\path\to\visit-artifacts --mode core
```

Run the complete matrix, including GUI rendering:

```sh
python3 /path/to/visit/scripts/docker/ManyLinuxTestHarness/run-linux-matrix.py --artifact-directory /path/to/visit-artifacts --mode all
```

The default matrix contains `rocky8`, `rocky9`, `ubuntu20`, `ubuntu22`,
`ubuntu24`, `ubuntu26`, `fedora`, and `debian`. It continues after a failed
profile. At the end it prints a row for every profile with `PASS` or `FAIL` and
the failed layers recorded in that profile's `suite_failures.txt`. Use
`--profiles` to select or reorder entries:

```sh
python3 /path/to/visit/scripts/docker/ManyLinuxTestHarness/run-linux-matrix.py --artifact-directory /path/to/visit-artifacts --mode all --profiles rocky8 rocky9 fedora debian
```

Single-profile and matrix results default to
`CURRENT_WORKING_DIRECTORY/results/PROFILE`. Override this location with
`--results-directory` for a single profile or `--results-root` for the matrix:

```sh
python3 /path/to/visit/scripts/docker/ManyLinuxTestHarness/run-linux-tests.py --profile rocky8 --artifact-directory /path/to/visit-artifacts --mode audit --results-directory ./tmp/visit-audit
```

The supported modes are `install`, `layout`, `audit`, `cli`, `engine`, `gui`,
`core`, and `all`. `core` runs installation, the ELF audit, layout checks, a
CLI test, and a headless engine/data/rendering test. `all` adds GUI rendering.
The driver automatically runs `install` before an individually selected test
layer. After installation succeeds, suite mode continues through independent
layers and reports all failures at the end.

The options `--glibc-baseline`, `--timeout`, `--platform`, `--no-build`,
`--strict-host-deps`, and `--strict-rpaths` are also available. Here,
`--profile` selects the Linux distribution while `--platform` selects Docker's
hardware platform and defaults to `linux/amd64`. Run either script with
`--help` for complete usage.

The ELF audit treats missing `libhwloc.so.*` from `libtbbbind*.so*` and missing
`libze_loader.so.*` from `libispcrt_device_gpu.so*` as optional OSPRay runtime
dependencies. They are recorded in `optional_missing_sonames.txt` and its
unique summary, but do not fail the audit. The exception is consumer-specific:
the same missing SONAME required by any other packaged ELF file remains a
failure.

The default runtime image tag contains a revision suffix. This prevents an
older locally cached image that lacks a newly discovered installer dependency
from being reused accidentally. The installed test scripts validate that an
image revision marker is present but do not hard-code its value, so increasing
the image tag cannot itself produce a false stale-image error.

For GUI testing, the driver creates a temporary Docker volume containing the
X11 Unix socket and a network-isolated, profile-matched Xvfb sidecar. Xvfb is
separate from the runtime container so its X11 packages do not hide
dependencies missing from the VisIt distribution. The GUI layer validates the
display with `xdpyinfo` and `glxinfo`, enables Mesa loader diagnostics, and
runs VisIt with debug level 1. It retains component `.vlog` files and the Xvfb
container log under `gui-smoke` so an early viewer exit remains diagnosable.

Inside an already-running runtime container, `run-tests.sh audit` expects
VisIt to have been installed first.

## Image definitions

- `runtime/Dockerfile.rocky8` and `display/Dockerfile.xvfb-rocky8`: Rocky 8.
- `runtime/Dockerfile.rocky9` and `display/Dockerfile.xvfb-rocky9`: Rocky 9.
- `runtime/Dockerfile.fedora` and `display/Dockerfile.xvfb-fedora`: Fedora 44,
  selected with `FEDORA_VERSION`.
- `runtime/Dockerfile.debian` and `display/Dockerfile.xvfb-debian`: Debian 13,
  selected with `DEBIAN_VERSION`.
- `runtime/Dockerfile.ubuntu` and `display/Dockerfile.xvfb-ubuntu`: Ubuntu
  20.04, 22.04, 24.04, or 26.04 selected with `UBUNTU_VERSION`.

All runtime images execute the same scripts under `common/`. The Debian and
Ubuntu images use Debian package tooling while Rocky and Fedora use RPM
tooling; the installation report records the appropriate `cpio` package
provenance.
The Ubuntu runtime setup also handles Ubuntu 24.04's pre-existing UID-1000
`ubuntu` account by normalizing it to the test account. It selects either
`libglib2.0-0` or the newer `libglib2.0-0t64` package according to what each
Ubuntu release provides.

## Configuration

The runtime image defaults to a glibc 2.28 ceiling, matching Rocky Linux 8.
Override it when testing a different compatibility promise:

```sh
docker run --rm \
  -e VISIT_GLIBC_BASELINE=2.17 \
  -v /absolute/path/to/artifacts:/artifacts:ro \
  -v /absolute/path/to/results:/results \
  visit-manylinux-test:rocky8 core
```

Other useful variables are:

- `VISIT_ARCHIVE`: exact archive path inside the container.
- `VISIT_VERSION`: normally derived from `VISIT_ARCHIVE`; an explicit value
  is accepted only when it matches the archive filename.
- `VISIT_INSTALLER`: exact installer path inside the container.
- `VISIT_TEST_TIMEOUT`: per-command timeout in seconds; default is 120.
- `VISIT_ALLOWED_HOST_SONAMES`: colon-separated additions to the small core
  glibc host-library allowlist.
- `VISIT_STRICT_HOST_DEPS=1`: fail when non-core libraries resolve from the
  runtime profile image instead of the VisIt package. By default these are
  reported but do not block functional tests.
- `VISIT_STRICT_RPATHS=1`: fail on absolute build-tree RPATH/RUNPATH entries.
  By default these are reported without blocking functional tests.
- `VISIT_BIN`: exact installed launcher path when automatic discovery is not
  sufficient.

The ELF audit writes both summary failures and complete dependency-resolution
reports under the results directory.

The installation layer also records `install/archive.sha256`, the complete
archive manifest, Python entries found in the archive, and Python entries
created by `visit-install`. It also records the test-image revision and the
installed `cpio` package in `install/image_environment.txt`. The host driver
prints the relevant reports automatically when a container test fails.
If a Docker image cannot be built, it also leaves `runtime-image-build.log`
or `xvfb-image-build.log` in that profile's results directory; successful
build logs are removed.
A portable VisIt package is expected to contain its own Python; the runtime
images intentionally do not install a system Python as a fallback.
