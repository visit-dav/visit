#!/usr/bin/env python3
"""Remove downloaded tarballs after the complete VisIt build succeeds."""

import argparse
import sys
from pathlib import Path

from visit_version import find_visit_archive


TARBALL_SUFFIXES = (
    ".tar",
    ".tar.gz",
    ".tar.bz2",
    ".tar.xz",
    ".tbz",
    ".tbz2",
    ".tgz",
    ".txz",
)


def is_tarball(path):
    return path.is_file() and path.name.endswith(TARBALL_SUFFIXES)


def cleanup_tarballs(home):
    home = Path(home)
    source_archive, version = find_visit_archive(home)
    removed = []
    for path in sorted(home.iterdir()):
        if not is_tarball(path):
            continue

        is_binary_package = (
            path != source_archive and path.name.startswith(version.package_prefix)
        )
        if is_binary_package:
            continue

        print("Removing {}".format(path))
        path.unlink()
        removed.append(path)
    return removed


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--home", default="/home/visit")
    args = parser.parse_args(argv)
    try:
        cleanup_tarballs(args.home)
    except RuntimeError as error:
        parser.error(str(error))
    return 0


if __name__ == "__main__":
    sys.exit(main())
