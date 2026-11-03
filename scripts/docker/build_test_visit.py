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


def prepare_config_site(home):
    destination = home / "visit-config.cmake"
    if destination.is_file():
        return destination

    source = find_one(
        (path for path in home.glob("*.cmake") if path != destination),
        "VisIt config-site file",
    )
    print("Copying {} to {}".format(source, destination))
    shutil.copyfile(str(source), str(destination))
    return destination


def mark_completed(state_dir, step, value="complete"):
    (state_dir / step).write_text("{}\n".format(value))


def completed(state_dir, step):
    return (state_dir / step).is_file()


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
        config_site = prepare_config_site(home)
    except RuntimeError as error:
        parser.error(str(error))

    state_dir = home / ".visit-docker-build-state"
    state_dir.mkdir(exist_ok=True)
    print("Building VisIt {} from {}".format(version.dotted, archive.name))
    if not completed(state_dir, "extracted"):
        subprocess.check_call(["tar", "zxf", archive.name], cwd=str(home))
        mark_completed(state_dir, "extracted")
    else:
        print("Reusing extracted VisIt source")

    source_dir = home / version.source_dir_name
    if not source_dir.is_dir():
        parser.error(
            "{} did not contain the expected top-level directory {}".format(
                archive.name, version.source_dir_name
            )
        )
    build_dir = source_dir / "build"
    build_dir.mkdir(exist_ok=True)

    if not completed(state_dir, "configured"):
        subprocess.check_call(
            [
                str(cmake),
                "-DCMAKE_BUILD_TYPE:STRING=Release",
                "-DVISIT_INSTALL_THIRD_PARTY:BOOL=ON",
                "-DVISIT_ENABLE_XDB:BOOL=OFF",
                "-DVISIT_PARADIS:BOOL=ON",
                "-DVISIT_CREATE_XMLTOOLS_GEN_TARGETS:BOOL=OFF",
                "-DVISIT_CONFIG_SITE={}".format(config_site),
                "../src",
            ],
            cwd=str(build_dir),
        )
        mark_completed(state_dir, "configured")
    else:
        print("Reusing configured VisIt build directory")

    package_state = state_dir / "packaged"
    if not package_state.is_file():
        subprocess.check_call(
            ["make", "-j", str(args.jobs), "package"], cwd=str(build_dir)
        )
        try:
            package = find_one(
                build_dir.glob("{}*.tar.gz".format(version.package_prefix)),
                "VisIt binary package",
            )
        except RuntimeError as error:
            parser.error(str(error))
        installed_package = home / package.name
        shutil.move(str(package), str(installed_package))
        mark_completed(state_dir, "packaged", installed_package.name)
    else:
        installed_package = home / package_state.read_text().strip()
        if not installed_package.is_file():
            parser.error(
                "recorded VisIt package is missing: {}".format(installed_package)
            )
        print("Reusing package {}".format(installed_package))

    platform = package_platform(installed_package, version)
    if not completed(state_dir, "installed"):
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
        mark_completed(state_dir, "installed")
    else:
        print("Reusing installed VisIt package")

    if not completed(state_dir, "tested"):
        subprocess.check_call(
            [
                str(home / "visit" / "bin" / "visit"),
                "-cli",
                "-nowin",
                "-s",
                "test_visit.py",
            ],
            cwd=str(home),
        )
        mark_completed(state_dir, "tested")
    else:
        print("VisIt smoke test already completed")
    return 0


if __name__ == "__main__":
    sys.exit(main())
