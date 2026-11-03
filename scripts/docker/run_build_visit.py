#!/usr/bin/env python3
"""Build VisIt's third-party dependencies inside a container."""

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
    "--optional",
    "--llvm",
    "--mesagl",
    "--anari",
    "--ospray",
    "--no-moab",
)


def main():
    home = Path.cwd()
    try:
        build_script, version = find_build_script(home)
    except RuntimeError as error:
        print("error: {}".format(error), file=sys.stderr)
        return 2

    print("Building third-party libraries for VisIt {}".format(version.dotted))
    process = subprocess.Popen(
        [str(build_script)] + list(BUILD_OPTIONS),
        cwd=str(home),
        stdin=subprocess.PIPE,
    )
    process.communicate(b"yes\n")
    return process.returncode


if __name__ == "__main__":
    sys.exit(main())
