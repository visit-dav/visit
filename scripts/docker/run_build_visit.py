#!/usr/bin/env python3
"""Build VisIt's third-party dependencies inside a container."""

import argparse
import subprocess
import sys
from pathlib import Path

from visit_version import find_build_script


BUILD_OPTIONS = (
    "--makeflags",
    "-j4",
    "--skip-opengl-context-check",
    "--no-visit",
    "--parallel",
    "--mpich",
    "--llvm",
    "--mesagl",
    "--anari",
    "--ospray",
    "--no-moab",
)


def build_options(include_optional=False):
    options = list(BUILD_OPTIONS)
    if include_optional:
        options.append("--optional")
    return options


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
    process = subprocess.Popen(
        [str(build_script)] + build_options(args.optional),
        cwd=str(home),
        stdin=subprocess.PIPE,
    )
    process.communicate(b"yes\n")
    return process.returncode


if __name__ == "__main__":
    sys.exit(main())
