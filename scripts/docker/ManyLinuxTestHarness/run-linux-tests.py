#!/usr/bin/env python3
"""Build and run one Linux VisIt test profile with Docker Desktop."""

from __future__ import annotations

import argparse
from dataclasses import dataclass
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import time
import uuid


MODES = ("install", "layout", "audit", "cli", "engine", "gui", "core", "all")
ARCHIVE_SUFFIXES = (".tar.gz", ".tgz", ".tar.xz", ".tar.bz2")
ARCHIVE_VERSION_RE = re.compile(r"^visit(?P<version>[0-9]+(?:_[0-9]+)+)(?=[^0-9_]|$)")


@dataclass(frozen=True)
class Profile:
    title: str
    runtime_dockerfile: str
    display_dockerfile: str
    runtime_image: str
    display_image: str
    build_args: tuple[tuple[str, str], ...] = ()


PROFILES = {
    "rocky8": Profile(
        "Rocky Linux 8",
        "runtime/Dockerfile.rocky8",
        "display/Dockerfile.xvfb-rocky8",
        "visit-manylinux-test:rocky8-v12",
        "visit-manylinux-test:xvfb-rocky8-v4",
    ),
    "rocky9": Profile(
        "Rocky Linux 9",
        "runtime/Dockerfile.rocky9",
        "display/Dockerfile.xvfb-rocky9",
        "visit-manylinux-test:rocky9-v4",
        "visit-manylinux-test:xvfb-rocky9-v1",
    ),
    "fedora": Profile(
        "Fedora 44",
        "runtime/Dockerfile.fedora",
        "display/Dockerfile.xvfb-fedora",
        "visit-manylinux-test:fedora44-v3",
        "visit-manylinux-test:xvfb-fedora44-v1",
        (("FEDORA_VERSION", "44"),),
    ),
    "debian": Profile(
        "Debian 13",
        "runtime/Dockerfile.debian",
        "display/Dockerfile.xvfb-debian",
        "visit-manylinux-test:debian13-v3",
        "visit-manylinux-test:xvfb-debian13-v1",
        (("DEBIAN_VERSION", "13"),),
    ),
    "ubuntu20": Profile(
        "Ubuntu 20.04",
        "runtime/Dockerfile.ubuntu",
        "display/Dockerfile.xvfb-ubuntu",
        "visit-manylinux-test:ubuntu20-v5",
        "visit-manylinux-test:xvfb-ubuntu20-v1",
        (("UBUNTU_VERSION", "20.04"),),
    ),
    "ubuntu22": Profile(
        "Ubuntu 22.04",
        "runtime/Dockerfile.ubuntu",
        "display/Dockerfile.xvfb-ubuntu",
        "visit-manylinux-test:ubuntu22-v5",
        "visit-manylinux-test:xvfb-ubuntu22-v1",
        (("UBUNTU_VERSION", "22.04"),),
    ),
    "ubuntu24": Profile(
        "Ubuntu 24.04",
        "runtime/Dockerfile.ubuntu",
        "display/Dockerfile.xvfb-ubuntu",
        "visit-manylinux-test:ubuntu24-v5",
        "visit-manylinux-test:xvfb-ubuntu24-v1",
        (("UBUNTU_VERSION", "24.04"),),
    ),
    "ubuntu26": Profile(
        "Ubuntu 26.04",
        "runtime/Dockerfile.ubuntu",
        "display/Dockerfile.xvfb-ubuntu",
        "visit-manylinux-test:ubuntu26-v3",
        "visit-manylinux-test:xvfb-ubuntu26-v1",
        (("UBUNTU_VERSION", "26.04"),),
    ),
}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Test a versioned VisIt many-Linux distribution in one Linux profile."
    )
    parser.add_argument(
        "--profile",
        required=True,
        choices=PROFILES,
        help="runtime Linux profile",
    )
    parser.add_argument(
        "--artifact-directory",
        required=True,
        type=Path,
        help="directory containing the VisIt archive and visit-install script",
    )
    parser.add_argument(
        "--mode",
        choices=MODES,
        default="core",
        help="test layer or suite to run (default: core)",
    )
    parser.add_argument(
        "--results-directory",
        type=Path,
        help="directory that receives reports and logs (default: cwd/results/PROFILE)",
    )
    parser.add_argument(
        "--glibc-baseline",
        default="2.28",
        help="highest permitted GLIBC symbol version (default: 2.28)",
    )
    parser.add_argument(
        "--timeout",
        type=int,
        default=120,
        help="timeout in seconds for each VisIt command (default: 120)",
    )
    parser.add_argument(
        "--runtime-image",
        help="override the profile's runtime image tag",
    )
    parser.add_argument(
        "--xvfb-image",
        help="override the profile's Xvfb image tag",
    )
    parser.add_argument(
        "--no-build",
        action="store_true",
        help="reuse existing Docker images instead of rebuilding them",
    )
    parser.add_argument(
        "--strict-host-deps",
        action="store_true",
        help="fail when an unapproved library resolves from the host image",
    )
    parser.add_argument(
        "--strict-rpaths",
        action="store_true",
        help="fail if packaged ELF files retain external absolute RPATHs",
    )
    parser.add_argument(
        "--platform",
        default="linux/amd64",
        help="Docker platform for the x86_64 VisIt artifact (default: linux/amd64)",
    )
    return parser.parse_args()


def run(command: list[str], *, check: bool = True) -> subprocess.CompletedProcess:
    print("+ " + subprocess.list2cmdline(command), flush=True)
    return subprocess.run(command, check=check)


def run_build(command: list[str], failure_log: Path) -> None:
    """Stream a Docker build and retain its complete output only on failure."""
    command_text = "+ " + subprocess.list2cmdline(command)
    print(command_text, flush=True)
    with failure_log.open("w", encoding="utf-8") as output:
        output.write(command_text + "\n")
        process = subprocess.Popen(
            command,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            errors="replace",
        )
        assert process.stdout is not None
        try:
            for line in process.stdout:
                print(line, end="", flush=True)
                output.write(line)
        except BaseException:
            process.terminate()
            process.wait()
            raise
        returncode = process.wait()

    if returncode != 0:
        raise subprocess.CalledProcessError(returncode, command)
    failure_log.unlink()


def docker_mount(host_path: Path, container_path: str, read_only: bool = False) -> str:
    suffix = ":ro" if read_only else ""
    return f"{host_path.resolve()}:{container_path}{suffix}"


def layer_commands(mode: str) -> list[str]:
    if mode in ("core", "all", "install"):
        return [mode]
    return ["install", mode]


def docker_build_command(
    *,
    script_dir: Path,
    dockerfile: str,
    image: str,
    platform: str,
    build_args: tuple[tuple[str, str], ...],
) -> list[str]:
    command = ["docker", "build", "--platform", platform]
    for name, value in build_args:
        command.extend(["--build-arg", f"{name}={value}"])
    command.extend(["-f", str(script_dir / dockerfile), "-t", image, str(script_dir)])
    return command


def selected_layers(mode: str) -> tuple[str, ...]:
    if mode == "all":
        return ("audit", "layout", "cli", "engine", "gui")
    if mode == "core":
        return ("audit", "layout", "cli", "engine")
    if mode == "install":
        return ()
    return (mode,)


def visit_archive_version(path: Path) -> str | None:
    name = path.name
    if not name.endswith(ARCHIVE_SUFFIXES):
        return None
    match = ARCHIVE_VERSION_RE.match(name)
    if match is None or name[match.end() :].startswith(".src."):
        return None
    return match.group("version").replace("_", ".")


def binary_archives(artifact_dir: Path) -> list[Path]:
    return sorted(
        path
        for path in artifact_dir.iterdir()
        if path.is_file() and visit_archive_version(path) is not None
    )


def print_failure_reports(results_dir: Path, mode: str) -> None:
    common_reports = (
        "install/image_environment.txt",
        "install/archive.sha256",
        "install/archive_python_entries.txt",
        "install/installed_python_entries.txt",
        "install/install.log",
    )
    reports_by_layer = {
        "layout": (
            "layout/container_architecture.txt",
            "layout/python_layout.txt",
            "layout/version.log",
        ),
        "cli": ("cli-smoke/visit.log",),
        "engine": ("engine-smoke/visit.log",),
        "gui": (
            "gui-smoke/display_environment.txt",
            "gui-smoke/xdpyinfo.txt",
            "gui-smoke/glxinfo.txt",
            "gui-smoke/viewer_process.log",
            "gui-smoke/visit.log",
            "gui-smoke/diagnostic_files.txt",
            "gui-smoke/gui_failure_report.txt",
            "gui-smoke/xvfb.log",
        ),
        "audit": (
            "elf-audit/missing_sonames_unique.txt",
            "elf-audit/optional_missing_sonames_unique.txt",
            "elf-audit/host_sonames_unique.txt",
            "elf-audit/bad_rpaths_unique.txt",
            "elf-audit/glibc_baseline_violations.txt",
        ),
    }
    layers = selected_layers(mode)
    report_names = list(common_reports)
    for layer in layers:
        report_names.extend(reports_by_layer[layer])
    if "gui" in layers:
        report_names.extend(
            str(path.relative_to(results_dir))
            for path in sorted((results_dir / "gui-smoke").glob("*.vlog"))
        )
    if mode in ("core", "all"):
        report_names.append("suite_failures.txt")

    print("\nAvailable failure reports:", file=sys.stderr)
    found = False
    for relative_name in dict.fromkeys(report_names):
        report = results_dir / relative_name
        if not report.is_file():
            continue
        try:
            contents = report.read_text(encoding="utf-8", errors="replace")
        except OSError as error:
            found = True
            print(f"\n===== {relative_name} =====", file=sys.stderr)
            print(f"Could not read report: {error}", file=sys.stderr)
            continue
        if not contents.strip():
            continue
        found = True
        print(f"\n===== {relative_name} =====", file=sys.stderr)
        lines = contents.splitlines()
        if len(lines) > 200:
            print(f"... {len(lines) - 200} earlier lines omitted ...", file=sys.stderr)
            lines = lines[-200:]
        print("\n".join(lines), file=sys.stderr)
    if not found:
        print("No report files were created.", file=sys.stderr)


def main() -> int:
    args = parse_args()
    profile = PROFILES[args.profile]
    script_dir = Path(__file__).resolve().parent
    artifact_dir = args.artifact_directory.resolve()
    results_dir = (
        args.results_directory.resolve()
        if args.results_directory
        else (Path.cwd() / "results" / args.profile).resolve()
    )
    runtime_image = args.runtime_image or os.environ.get(
        "VISIT_RUNTIME_IMAGE", profile.runtime_image
    )
    display_image = args.xvfb_image or os.environ.get(
        "VISIT_XVFB_IMAGE", profile.display_image
    )

    if not artifact_dir.is_dir():
        print(f"error: artifact directory does not exist: {artifact_dir}", file=sys.stderr)
        return 2
    archives = binary_archives(artifact_dir)
    if not archives:
        installed_hint = ""
        if (artifact_dir / "current").exists() and (artifact_dir / "bin" / "visit").exists():
            installed_hint = (
                "\nThis appears to be an installed VisIt tree. Pass the directory "
                "containing the binary tarball; the harness tests visit-install itself."
            )
        print(
            f"error: no versioned VisIt binary archive found directly in "
            f"{artifact_dir}.{installed_hint}",
            file=sys.stderr,
        )
        return 2
    if len(archives) != 1:
        print(
            f"error: expected one versioned VisIt binary archive directly in "
            f"{artifact_dir}; found {len(archives)}: "
            + ", ".join(path.name for path in archives),
            file=sys.stderr,
        )
        return 2
    archive = archives[0]
    visit_version = visit_archive_version(archive)
    assert visit_version is not None
    if args.timeout <= 0:
        print("error: --timeout must be positive", file=sys.stderr)
        return 2
    if shutil.which("docker") is None:
        print(
            "error: docker was not found in PATH; start Docker Desktop and open a new terminal",
            file=sys.stderr,
        )
        return 2

    results_dir.mkdir(parents=True, exist_ok=True)
    needs_display = args.mode in ("gui", "all")

    if not args.no_build:
        runtime_build_log = results_dir / "runtime-image-build.log"
        display_build_log = results_dir / "xvfb-image-build.log"
        runtime_build_log.unlink(missing_ok=True)
        display_build_log.unlink(missing_ok=True)
        run_build(
            docker_build_command(
                script_dir=script_dir,
                dockerfile=profile.runtime_dockerfile,
                image=runtime_image,
                platform=args.platform,
                build_args=profile.build_args,
            ),
            runtime_build_log,
        )
        if needs_display:
            run_build(
                docker_build_command(
                    script_dir=script_dir,
                    dockerfile=profile.display_dockerfile,
                    image=display_image,
                    platform=args.platform,
                    build_args=profile.build_args,
                ),
                display_build_log,
            )

    unique = uuid.uuid4().hex[:10]
    x11_volume_name = f"visit-x11-{args.profile}-{unique}"
    xvfb_name = f"visit-xvfb-{args.profile}-{unique}"
    x11_volume_created = False
    xvfb_started = False

    def capture_xvfb_log() -> None:
        if not xvfb_started:
            return
        log_dir = results_dir / "gui-smoke"
        log_dir.mkdir(parents=True, exist_ok=True)
        completed = subprocess.run(
            ["docker", "logs", xvfb_name],
            check=False,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            errors="replace",
        )
        (log_dir / "xvfb.log").write_text(completed.stdout, encoding="utf-8")

    try:
        if needs_display:
            run(["docker", "volume", "create", x11_volume_name])
            x11_volume_created = True
            run(
                [
                    "docker",
                    "run",
                    "--platform",
                    args.platform,
                    "-d",
                    "--rm",
                    "--name",
                    xvfb_name,
                    "--network",
                    "none",
                    "-v",
                    f"{x11_volume_name}:/tmp/.X11-unix",
                    display_image,
                ]
            )
            xvfb_started = True
            time.sleep(1)

        command = ["docker", "run", "--platform", args.platform, "--rm"]
        if needs_display:
            command.extend(
                [
                    "-e",
                    "DISPLAY=:99",
                    "-v",
                    f"{x11_volume_name}:/tmp/.X11-unix",
                ]
            )
        command.extend(
            [
                "-e",
                f"VISIT_GLIBC_BASELINE={args.glibc_baseline}",
                "-e",
                f"VISIT_TEST_TIMEOUT={args.timeout}",
                "-e",
                f"VISIT_ARCHIVE=/artifacts/{archive.name}",
                "-e",
                f"VISIT_VERSION={visit_version}",
                "-e",
                f"VISIT_STRICT_HOST_DEPS={int(args.strict_host_deps)}",
                "-e",
                f"VISIT_STRICT_RPATHS={int(args.strict_rpaths)}",
                "-v",
                docker_mount(artifact_dir, "/artifacts", read_only=True),
                "-v",
                docker_mount(results_dir, "/results"),
                runtime_image,
                *layer_commands(args.mode),
            ]
        )
        try:
            run(command)
        except subprocess.CalledProcessError:
            capture_xvfb_log()
            print_failure_reports(results_dir, args.mode)
            raise
    finally:
        if xvfb_started:
            capture_xvfb_log()
            run(["docker", "rm", "-f", xvfb_name], check=False)
        if x11_volume_created:
            run(["docker", "volume", "rm", x11_volume_name], check=False)

    print(f"VisIt {args.mode} tests passed on {profile.title}. Results: {results_dir}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except subprocess.CalledProcessError as error:
        print(f"error: command failed with exit code {error.returncode}", file=sys.stderr)
        raise SystemExit(error.returncode)
    except KeyboardInterrupt:
        print("\nInterrupted.", file=sys.stderr)
        raise SystemExit(130)
