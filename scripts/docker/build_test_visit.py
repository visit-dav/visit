#!/usr/bin/env python3
"""Build, package, install, and smoke-test VisIt inside a container."""

import argparse
import shutil
import subprocess
import sys
from pathlib import Path

from visit_version import validate_build_context


def find_one(paths, description):
    paths = sorted(paths)
    if len(paths) != 1:
        names = ", ".join(str(path) for path in paths) or "none"
        raise RuntimeError(
            "expected exactly one {}; found {}: {}".format(
                description, len(paths), names
            )
        )
    return paths[0]


def package_platform(package, version):
    prefix = version.package_prefix
    suffix = ".tar.gz"
    if not package.name.startswith(prefix) or not package.name.endswith(suffix):
        raise RuntimeError("unexpected package name: {}".format(package.name))
    return package.name[len(prefix) : -len(suffix)]


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--home", default="/home/visit")
    parser.add_argument("--jobs", type=int, default=4)
    args = parser.parse_args(argv)

    home = Path(args.home).resolve()
    try:
        archive, _build_script, version = validate_build_context(home)
        cmake = find_one(
            home.glob("third_party/cmake/*/*/bin/cmake"), "third-party cmake"
        )
    except RuntimeError as error:
        parser.error(str(error))

    print("Building VisIt {} from {}".format(version.dotted, archive.name))
    subprocess.check_call(["tar", "zxf", archive.name], cwd=str(home))

    source_dir = home / version.source_dir_name
    if not source_dir.is_dir():
        parser.error(
            "{} did not contain the expected top-level directory {}".format(
                archive.name, version.source_dir_name
            )
        )
    build_dir = source_dir / "build"
    build_dir.mkdir()

    subprocess.check_call(
        [
            str(cmake),
            "-DCMAKE_BUILD_TYPE:STRING=Release",
            "-DVISIT_INSTALL_THIRD_PARTY:BOOL=ON",
            "-DVISIT_ENABLE_XDB:BOOL=OFF",
            "-DVISIT_PARADIS:BOOL=ON",
            "-DVISIT_CREATE_XMLTOOLS_GEN_TARGETS:BOOL=OFF",
            "-DVISIT_CONFIG_SITE={}".format(home / "visit-config.cmake"),
            "../src",
        ],
        cwd=str(build_dir),
    )
    subprocess.check_call(["make", "-j", str(args.jobs), "package"], cwd=str(build_dir))

    try:
        package = find_one(
            build_dir.glob("{}*.tar.gz".format(version.package_prefix)),
            "VisIt binary package",
        )
    except RuntimeError as error:
        parser.error(str(error))
    installed_package = home / package.name
    shutil.move(str(package), str(installed_package))

    platform = package_platform(installed_package, version)
    subprocess.check_call(
        [
            str(source_dir / "scripts" / "visit-install"),
            "-c",
            "none",
            version.dotted,
            platform,
            "visit",
        ],
        cwd=str(home),
    )
    subprocess.check_call(
        [str(home / "visit" / "bin" / "visit"), "-cli", "-nowin", "-s", "test_visit.py"],
        cwd=str(home),
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
