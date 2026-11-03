#!/usr/bin/env python3
"""Build VisIt's third-party dependencies inside a container."""

import argparse
import shutil
import subprocess
import sys
from pathlib import Path

from visit_version import find_build_script


BUILD_OPTIONS = (
    "--makeflags",
    "-j4",
    "--skip-opengl-context-check",
    "--no-visit",
    "--cleanup",
    "--parallel",
    "--mpich",
    "--llvm",
    "--mesagl",
    "--anari",
    "--ospray",
)


def build_options(include_optional=False):
    options = list(BUILD_OPTIONS)
    if include_optional:
        options.append("--optional")
    return options


def config_site_snapshot(home):
    """Record generated config sites so a newly written one can be identified."""
    destination = home / "visit-config.cmake"
    return {
        path: (path.stat().st_mtime_ns, path.stat().st_size)
        for path in home.glob("*.cmake")
        if path != destination
    }


def save_config_site(home, before):
    """Copy the config site written by build_visit to a stable filename."""
    destination = home / "visit-config.cmake"
    candidates = [
        path
        for path in home.glob("*.cmake")
        if path != destination
        and before.get(path) != (path.stat().st_mtime_ns, path.stat().st_size)
    ]
    if not candidates:
        candidates = [path for path in home.glob("*.cmake") if path != destination]
    if not candidates:
        raise RuntimeError("build_visit did not create a VisIt config-site file")

    source = max(candidates, key=lambda path: (path.stat().st_mtime_ns, path.name))
    print("Saving {} as {}".format(source, destination))
    shutil.copyfile(str(source), str(destination))


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--optional",
        action="store_true",
        help="repeat the build with VisIt's optional third-party libraries enabled",
    )
    args = parser.parse_args(argv)

    home = Path.cwd()
    try:
        build_script, version = find_build_script(home)
    except RuntimeError as error:
        print("error: {}".format(error), file=sys.stderr)
        return 2

    phase = "optional" if args.optional else "core"
    print(
        "Building {} third-party libraries for VisIt {}".format(
            phase, version.dotted
        )
    )
    config_sites = config_site_snapshot(home)
    process = subprocess.Popen(
        [str(build_script)] + build_options(args.optional),
        cwd=str(home),
        stdin=subprocess.PIPE,
    )
    process.communicate(b"yes\n")
    if process.returncode != 0:
        return process.returncode

    try:
        save_config_site(home, config_sites)
    except RuntimeError as error:
        print("error: {}".format(error), file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main())
