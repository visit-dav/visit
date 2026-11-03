#!/usr/bin/env python3
"""Build a VisIt container and copy its release artifacts to the host."""

import argparse
import os
import shlex
import shutil
import subprocess
import sys
import tempfile
from contextlib import contextmanager
from pathlib import Path

from visit_version import validate_inputs


PLATFORMS = {
    "ubuntu": {
        "dockerfile": "Dockerfile-ubuntu",
        "build_arg": "UBUNTU_VERSION",
        "default_version": "24.04",
    },
    "rocky": {
        "dockerfile": "Dockerfile-rocky",
        "build_arg": "ROCKY_VERSION",
        "default_version": "9.3",
    },
    "debian": {
        "dockerfile": "Dockerfile-debian",
        "build_arg": "DEBIAN_VERSION",
        "default_version": "12",
    },
    "fedora": {
        "dockerfile": "Dockerfile-fedora",
        "build_arg": "FEDORA_VERSION",
        "default_version": "40",
    },
}

BUILD_HELPERS = (
    "build_test_visit.py",
    "build_visit_docker_cleanup.py",
    "run_build_visit.py",
    "test_visit.py",
    "visit_version.py",
)


def run(command):
    print("+ {}".format(" ".join(shlex.quote(part) for part in command)), flush=True)
    subprocess.check_call(command)


def platform_label(platform, os_version):
    if os_version is None:
        return platform
    return "{}{}".format(platform, os_version.split(".")[0])


def stage_file(source, destination):
    """Hard-link a build input when possible, otherwise copy it."""
    try:
        os.link(str(source), str(destination))
    except OSError:
        shutil.copy2(str(source), str(destination))


@contextmanager
def temporary_build_context(script_dir, dockerfile, archive, build_script, temp_dir):
    parent = None if temp_dir is None else str(temp_dir)
    with tempfile.TemporaryDirectory(prefix="visit-docker-context-", dir=parent) as name:
        context = Path(name)
        for filename in BUILD_HELPERS:
            shutil.copy2(str(script_dir / filename), str(context / filename))
        shutil.copy2(str(dockerfile), str(context / dockerfile.name))
        stage_file(archive, context / archive.name)
        stage_file(build_script, context / build_script.name)
        yield context


def parse_args(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--platform", choices=sorted(PLATFORMS), required=True)
    parser.add_argument(
        "--source",
        type=Path,
        required=True,
        help="path to visitMAJOR_MINOR_PATCH.src.tar.gz",
    )
    parser.add_argument(
        "--build-visit",
        type=Path,
        required=True,
        help="path to the matching build_visitMAJOR_MINOR_PATCH script",
    )
    parser.add_argument("--os-version", help="base image version for the selected platform")
    parser.add_argument("--engine", choices=("docker", "podman"), default="docker")
    parser.add_argument("--output-dir", type=Path, default=Path.cwd())
    parser.add_argument(
        "--temp-dir",
        type=Path,
        help="parent directory for the temporary Docker context",
    )
    parser.add_argument("--tag", help="container image tag (derived by default)")
    parser.add_argument("--label", help="suffix used for host artifact names")
    parser.add_argument(
        "--userns-uid-map",
        action="append",
        default=[],
        metavar="MAP",
        help="pass a user namespace UID map to podman build; may be repeated",
    )
    parser.add_argument(
        "--build-only", action="store_true", help="build the image without copying artifacts"
    )
    args = parser.parse_args(argv)

    config = PLATFORMS[args.platform]
    if args.userns_uid_map and args.engine != "podman":
        parser.error("--userns-uid-map requires --engine podman")
    return args


def main(argv=None):
    args = parse_args(argv)
    script_dir = Path(__file__).resolve().parent
    archive = args.source.resolve()
    build_script = args.build_visit.resolve()
    output_dir = args.output_dir.resolve()
    temp_dir = None if args.temp_dir is None else args.temp_dir.resolve()
    config = PLATFORMS[args.platform]

    try:
        archive, build_script, version = validate_inputs(archive, build_script)
    except (RuntimeError, ValueError) as error:
        print("error: {}".format(error), file=sys.stderr)
        return 2

    dockerfile = script_dir / config["dockerfile"]
    if not dockerfile.is_file():
        print("error: Dockerfile not found: {}".format(dockerfile), file=sys.stderr)
        return 2
    if temp_dir is not None and not temp_dir.is_dir():
        print("error: temporary directory not found: {}".format(temp_dir), file=sys.stderr)
        return 2

    os_version = args.os_version or config["default_version"]
    label = args.label or platform_label(args.platform, os_version)
    tag = args.tag or "visitdev:{}-{}".format(version.dotted, label)

    if not args.build_only:
        output_dir.mkdir(parents=True, exist_ok=True)
        output_package = output_dir / "visit{}.linux-x86_64-{}.tar.gz".format(
            version.underscored, label
        )
        output_image = output_dir / "visit0000.png"
        for output in (output_package, output_image):
            if output.exists():
                print("error: refusing to overwrite {}".format(output), file=sys.stderr)
                return 2

    with temporary_build_context(
        script_dir, dockerfile, archive, build_script, temp_dir
    ) as context:
        build_command = [args.engine, "build"]
        for uid_map in args.userns_uid_map:
            build_command.extend(("--userns-uid-map", uid_map))
        build_command.extend(("-f", str(context / dockerfile.name), "-t", tag))
        if config["build_arg"]:
            build_command.extend(
                ("--build-arg", "{}={}".format(config["build_arg"], os_version))
            )
        build_command.append(str(context))
        run(build_command)

    if args.build_only:
        return 0

    package_name = "visit{}.linux-x86_64.tar.gz".format(version.underscored)
    container_name = "visit-release-{}".format(os.getpid())
    created = False
    try:
        run([args.engine, "create", "--name", container_name, tag])
        created = True
        run(
            [
                args.engine,
                "cp",
                "{}:/home/visit/{}".format(container_name, package_name),
                str(output_package),
            ]
        )
        run(
            [
                args.engine,
                "cp",
                "{}:/home/visit/visit0000.png".format(container_name),
                str(output_image),
            ]
        )
    finally:
        if created:
            run([args.engine, "rm", "-f", container_name])

    print("Created {} and {}".format(output_package, output_image))
    return 0


if __name__ == "__main__":
    sys.exit(main())
