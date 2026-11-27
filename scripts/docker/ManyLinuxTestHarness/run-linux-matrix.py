#!/usr/bin/env python3
"""Run the VisIt test suite across the requested Linux profiles."""

from __future__ import annotations

import argparse
from dataclasses import dataclass
from pathlib import Path
import subprocess
import sys


DEFAULT_PROFILES = (
    "rocky8",
    "rocky9",
    "ubuntu20",
    "ubuntu22",
    "ubuntu24",
    "ubuntu26",
    "fedora",
    "debian",
)
ALL_PROFILES = DEFAULT_PROFILES
MODES = ("install", "layout", "audit", "cli", "engine", "gui", "core", "all")


@dataclass(frozen=True)
class ProfileResult:
    profile: str
    returncode: int
    suite_failures: tuple[str, ...]
    suite_report_found: bool

    @property
    def passed(self) -> bool:
        return self.returncode == 0 and not self.suite_failures


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Run VisIt many-Linux tests across a Docker image matrix."
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
        default="all",
        help="test layer or suite to run (default: all)",
    )
    parser.add_argument(
        "--results-root",
        type=Path,
        default=Path.cwd() / "results",
        help="root directory for profile results (default: cwd/results)",
    )
    parser.add_argument(
        "--profiles",
        nargs="+",
        choices=ALL_PROFILES,
        default=DEFAULT_PROFILES,
        help="profiles to run (default: " + " ".join(DEFAULT_PROFILES) + ")",
    )
    parser.add_argument("--glibc-baseline", default="2.28")
    parser.add_argument("--timeout", type=int, default=120)
    parser.add_argument("--no-build", action="store_true")
    parser.add_argument("--strict-host-deps", action="store_true")
    parser.add_argument("--strict-rpaths", action="store_true")
    parser.add_argument("--platform", default="linux/amd64")
    return parser.parse_args()


def read_suite_failures(path: Path) -> tuple[bool, tuple[str, ...]]:
    try:
        contents = path.read_text(encoding="utf-8", errors="replace")
    except FileNotFoundError:
        return False, ()
    except OSError as error:
        print(f"warning: cannot read {path}: {error}", file=sys.stderr)
        return False, ()

    failures = tuple(line.strip() for line in contents.splitlines() if line.strip())
    return True, failures


def print_matrix_summary(results: list[ProfileResult]) -> None:
    profile_width = max(len("PROFILE"), *(len(result.profile) for result in results))

    print("\nMatrix suite summary:")
    print(f"{'PROFILE':<{profile_width}}  RESULT  SUITE FAILURES")
    for result in results:
        status = "PASS" if result.passed else "FAIL"
        if result.suite_failures:
            details = ", ".join(result.suite_failures)
        elif result.suite_report_found:
            details = "none"
        elif result.returncode != 0:
            details = f"unavailable (driver exit {result.returncode})"
        else:
            details = "not produced"
        print(f"{result.profile:<{profile_width}}  {status:<6}  {details}")
    sys.stdout.flush()


def main() -> int:
    args = parse_args()
    script_dir = Path(__file__).resolve().parent
    driver = script_dir / "run-linux-tests.py"
    artifact_dir = args.artifact_directory.resolve()
    results_root = args.results_root.resolve()
    results_root.mkdir(parents=True, exist_ok=True)
    results: list[ProfileResult] = []

    for profile in args.profiles:
        print(f"\n===== {profile} =====", flush=True)
        profile_results = results_root / profile
        suite_failures_path = profile_results / "suite_failures.txt"
        # A build or startup failure happens before run-tests.sh can clear its
        # suite report. Remove an older report here so the summary cannot
        # attribute stale failed layers to the current run.
        suite_failures_path.unlink(missing_ok=True)
        command = [
            sys.executable,
            str(driver),
            "--profile",
            profile,
            "--artifact-directory",
            str(artifact_dir),
            "--mode",
            args.mode,
            "--results-directory",
            str(profile_results),
            "--glibc-baseline",
            args.glibc_baseline,
            "--timeout",
            str(args.timeout),
            "--platform",
            args.platform,
        ]
        if args.no_build:
            command.append("--no-build")
        if args.strict_host_deps:
            command.append("--strict-host-deps")
        if args.strict_rpaths:
            command.append("--strict-rpaths")
        completed = subprocess.run(command, check=False)
        report_found, suite_failures = read_suite_failures(suite_failures_path)
        results.append(
            ProfileResult(
                profile=profile,
                returncode=completed.returncode,
                suite_failures=suite_failures,
                suite_report_found=report_found,
            )
        )

    print_matrix_summary(results)
    failures = [result.profile for result in results if not result.passed]

    if failures:
        print("\nFailed profiles: " + ", ".join(failures), file=sys.stderr)
        print(f"Results: {results_root}", file=sys.stderr)
        return 1

    print("\nAll profiles passed: " + ", ".join(args.profiles))
    print(f"Results: {results_root}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except KeyboardInterrupt:
        print("\nInterrupted.", file=sys.stderr)
        raise SystemExit(130)
