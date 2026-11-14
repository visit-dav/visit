#!/usr/bin/env python3
"""Discover and represent the VisIt version in a Docker build context."""

import argparse
import re
import sys
from pathlib import Path


ARCHIVE_PATTERN = re.compile(
    r"^visit(?P<major>[0-9]+)_(?P<minor>[0-9]+)_(?P<patch>[0-9]+)\.src\.tar\.gz$"
)
BUILD_SCRIPT_PATTERN = re.compile(
    r"^build_visit(?P<major>[0-9]+)_(?P<minor>[0-9]+)_(?P<patch>[0-9]+)$"
)


class VisitVersion:
    def __init__(self, major, minor, patch):
        self.parts = (major, minor, patch)

    @property
    def dotted(self):
        return ".".join(self.parts)

    @property
    def underscored(self):
        return "_".join(self.parts)

    @property
    def archive_name(self):
        return "visit{}.src.tar.gz".format(self.underscored)

    @property
    def source_dir_name(self):
        return "visit{}".format(self.underscored)

    @property
    def build_script_name(self):
        return "build_visit{}".format(self.underscored)

    @property
    def package_prefix(self):
        return "visit{}.".format(self.underscored)


def version_from_archive(path):
    path = Path(path)
    match = ARCHIVE_PATTERN.match(path.name)
    if match is None:
        raise ValueError(
            "expected an archive named visitMAJOR_MINOR_PATCH.src.tar.gz; got {!r}".format(
                path.name
            )
        )
    return VisitVersion(match.group("major"), match.group("minor"), match.group("patch"))


def version_from_build_script(path):
    path = Path(path)
    match = BUILD_SCRIPT_PATTERN.match(path.name)
    if match is None:
        raise ValueError(
            "expected a script named build_visitMAJOR_MINOR_PATCH; got {!r}".format(
                path.name
            )
        )
    return VisitVersion(match.group("major"), match.group("minor"), match.group("patch"))


def find_visit_archive(directory):
    directory = Path(directory)
    archives = sorted(
        path
        for path in directory.glob("visit*.src.tar.gz")
        if ARCHIVE_PATTERN.match(path.name)
    )
    if len(archives) != 1:
        names = ", ".join(path.name for path in archives) or "none"
        raise RuntimeError(
            "expected exactly one visitMAJOR_MINOR_PATCH.src.tar.gz in {}; found {}: {}".format(
                directory, len(archives), names
            )
        )
    return archives[0], version_from_archive(archives[0])


def find_build_script(directory):
    directory = Path(directory)
    scripts = sorted(
        path
        for path in directory.glob("build_visit[0-9]*")
        if BUILD_SCRIPT_PATTERN.match(path.name)
    )
    if len(scripts) != 1:
        names = ", ".join(path.name for path in scripts) or "none"
        raise RuntimeError(
            "expected exactly one build_visitMAJOR_MINOR_PATCH in {}; found {}: {}".format(
                directory, len(scripts), names
            )
        )
    return scripts[0], version_from_build_script(scripts[0])


def validate_inputs(archive, build_script):
    archive = Path(archive)
    build_script = Path(build_script)
    if not archive.is_file():
        raise RuntimeError("source archive not found: {}".format(archive))
    if not build_script.is_file():
        raise RuntimeError("third-party build script not found: {}".format(build_script))

    version = version_from_archive(archive)
    build_version = version_from_build_script(build_script)
    if build_version.parts != version.parts:
        raise RuntimeError(
            "{} does not match third-party build script {}".format(
                archive.name, build_script.name
            )
        )
    return archive, build_script, version


def validate_build_context(directory):
    directory = Path(directory)
    archive, _version = find_visit_archive(directory)
    build_script, _build_version = find_build_script(directory)
    return validate_inputs(archive, build_script)


def main(argv=None):
    parser = argparse.ArgumentParser(
        description="Print the VisIt version inferred from a source archive."
    )
    parser.add_argument("directory", nargs="?", default=".")
    parser.add_argument(
        "--format",
        choices=("dotted", "underscored", "archive", "source-dir", "build-script"),
        default="dotted",
    )
    args = parser.parse_args(argv)

    try:
        archive, version = find_visit_archive(args.directory)
    except (RuntimeError, ValueError) as error:
        parser.error(str(error))

    values = {
        "dotted": version.dotted,
        "underscored": version.underscored,
        "archive": archive.name,
        "source-dir": version.source_dir_name,
        "build-script": version.build_script_name,
    }
    print(values[args.format])
    return 0


if __name__ == "__main__":
    sys.exit(main())
