#!/usr/bin/env bash
set -euo pipefail

test_root="${VISIT_TEST_ROOT:-/opt/visit-test}"
source "${test_root}/lib/common.sh"

usage()
{
    cat <<'EOF'
Usage: run-tests.sh COMMAND [COMMAND ...]

Commands:
  install  Install the mounted versioned VisIt distribution
  layout   Check installation layout, version, links, and ELF presence
  audit    Audit ELF dependencies, RPATHs, and the glibc baseline
  cli      Run the non-interactive CLI smoke test
  engine   Open data and execute a plot through the compute engine
  gui      Render through an external X display and software Mesa
  core     Run install, layout, audit, CLI, and engine tests
  all      Run every layer, including the GUI test
EOF
}

run_suite()
{
    local include_gui="$1"
    local failures=0
    local layer
    local layers=(audit layout cli engine)
    local failures_file="${VISIT_RESULTS_DIR}/suite_failures.txt"

    : > "${failures_file}"

    # Installation is a prerequisite for every other layer, so stop the suite
    # immediately if it fails.
    if ! run_command install; then
        log "FAIL install; the remaining layers require an installed distribution"
        printf '%s\n' install > "${failures_file}"
        die "installation failed; see ${failures_file} and the install reports"
    fi

    if [[ "${include_gui}" == "yes" ]]; then
        layers+=(gui)
    fi

    for layer in "${layers[@]}"; do
        if ! run_command "${layer}"; then
            log "FAIL ${layer}; continuing with the remaining independent layers"
            printf '%s\n' "${layer}" >> "${failures_file}"
            ((failures += 1))
        fi
    done

    if ((failures != 0)); then
        log "FAILED LAYERS: $(paste -sd, "${failures_file}")"
        die "${failures} test layer(s) failed; see ${failures_file}"
    fi
}

run_command()
{
    case "$1" in
        install) resolve_visit_archive_and_version; "${test_root}/bin/00-install.sh" ;;
        layout)  resolve_visit_archive_and_version; "${test_root}/bin/10-layout.sh" ;;
        audit)   resolve_visit_archive_and_version; "${test_root}/bin/20-elf-audit.sh" ;;
        cli)     resolve_visit_archive_and_version; "${test_root}/bin/30-cli-smoke.sh" ;;
        engine)  resolve_visit_archive_and_version; "${test_root}/bin/40-engine-smoke.sh" ;;
        gui)     resolve_visit_archive_and_version; "${test_root}/bin/50-gui-smoke.sh" ;;
        core)    run_suite no ;;
        all)     run_suite yes ;;
        -h|--help|help) usage ;;
        *) usage >&2; die "unknown test layer: $1" ;;
    esac
}

(($# > 0)) || set -- core
for command in "$@"; do
    run_command "${command}"
done

log "Requested test layers completed successfully"
