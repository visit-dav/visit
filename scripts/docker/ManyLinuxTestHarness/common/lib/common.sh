#!/usr/bin/env bash

set -euo pipefail

VISIT_VERSION="${VISIT_VERSION:-}"
VISIT_ARCHIVE="${VISIT_ARCHIVE:-}"
VISIT_ROOT="${VISIT_ROOT:-/opt/visit}"
VISIT_ARTIFACT_DIR="${VISIT_ARTIFACT_DIR:-/artifacts}"
VISIT_RESULTS_DIR="${VISIT_RESULTS_DIR:-/results}"
VISIT_TEST_ROOT="${VISIT_TEST_ROOT:-/opt/visit-test}"
VISIT_TEST_TIMEOUT="${VISIT_TEST_TIMEOUT:-120}"
VISIT_TEST_HOME="${VISIT_TEST_HOME:-/work/home}"

export VISIT_VERSION VISIT_ARCHIVE VISIT_ROOT VISIT_ARTIFACT_DIR VISIT_RESULTS_DIR
export VISIT_TEST_ROOT VISIT_TEST_TIMEOUT VISIT_TEST_HOME
export HOME="${VISIT_TEST_HOME}"
export TMPDIR="${TMPDIR:-/work/tmp}"

mkdir -p "${VISIT_RESULTS_DIR}" "${VISIT_TEST_HOME}" "${TMPDIR}"

log()
{
    printf '[visit-test] %s\n' "$*"
}

die()
{
    printf '[visit-test] ERROR: %s\n' "$*" >&2
    exit 1
}

visit_version_from_archive_name()
{
    local archive_name

    archive_name="$(basename "$1")"
    case "${archive_name}" in
        *.tar.gz|*.tgz|*.tar.xz|*.tar.bz2) ;;
        *) return 1 ;;
    esac
    [[ "${archive_name}" != visit*.src.* ]] || return 1
    if [[ "${archive_name}" =~ ^visit([0-9]+(_[0-9]+)+)([^0-9_]|$) ]]; then
        printf '%s\n' "${BASH_REMATCH[1]//_/.}"
        return 0
    fi
    return 1
}

find_visit_binary_archives()
{
    local candidate

    while IFS= read -r candidate; do
        if visit_version_from_archive_name "${candidate}" >/dev/null; then
            printf '%s\n' "${candidate}"
        fi
    done < <(find "${VISIT_ARTIFACT_DIR}" -maxdepth 1 -type f \
        \( -name 'visit*.tar.gz' -o -name 'visit*.tgz' \
           -o -name 'visit*.tar.xz' -o -name 'visit*.tar.bz2' \) \
        -print | sort)
}

resolve_visit_archive_and_version()
{
    local archive_version
    local -a archives

    if [[ -z "${VISIT_ARCHIVE}" ]]; then
        mapfile -t archives < <(find_visit_binary_archives)
        ((${#archives[@]} == 1)) || \
            die "expected exactly one versioned VisIt binary archive in ${VISIT_ARTIFACT_DIR}; found ${#archives[@]} after ignoring source archives (or set VISIT_ARCHIVE)"
        VISIT_ARCHIVE="${archives[0]}"
    fi
    [[ -r "${VISIT_ARCHIVE}" ]] || die "VisIt archive is not readable: ${VISIT_ARCHIVE}"

    archive_version="$(visit_version_from_archive_name "${VISIT_ARCHIVE}" || true)"
    [[ -n "${archive_version}" ]] || \
        die "cannot determine the VisIt version from archive name: $(basename "${VISIT_ARCHIVE}")"
    if [[ -n "${VISIT_VERSION}" && "${VISIT_VERSION}" != "${archive_version}" ]]; then
        die "VISIT_VERSION=${VISIT_VERSION} does not match version ${archive_version} parsed from $(basename "${VISIT_ARCHIVE}")"
    fi

    VISIT_VERSION="${archive_version}"
    export VISIT_ARCHIVE VISIT_VERSION
}

find_visit_binary()
{
    local candidate

    if [[ -n "${VISIT_BIN:-}" ]]; then
        [[ -x "${VISIT_BIN}" ]] || die "VISIT_BIN is not executable: ${VISIT_BIN}"
        printf '%s\n' "${VISIT_BIN}"
        return
    fi

    for candidate in \
        "${VISIT_ROOT}/bin/visit" \
        "${VISIT_ROOT}/${VISIT_VERSION}/bin/visit"
    do
        if [[ -x "${candidate}" ]]; then
            printf '%s\n' "${candidate}"
            return
        fi
    done

    candidate="$(find "${VISIT_ROOT}" -type f -path '*/bin/visit' -perm -u+x -print -quit 2>/dev/null || true)"
    [[ -n "${candidate}" ]] || die "cannot find an installed VisIt launcher below ${VISIT_ROOT}"
    printf '%s\n' "${candidate}"
}

require_visit()
{
    if [[ -z "${VISIT_VERSION}" ]]; then
        resolve_visit_archive_and_version
    fi
    VISIT_BIN="$(find_visit_binary)"
    export VISIT_BIN
}

run_visit()
{
    require_visit
    timeout --signal=TERM --kill-after=15s "${VISIT_TEST_TIMEOUT}" \
        "${VISIT_BIN}" "$@"
}

start_layer()
{
    local name="$1"
    local layer_dir="${VISIT_RESULTS_DIR}/${name}"
    log "BEGIN ${name}"
    mkdir -p "${layer_dir}"
    # A results directory may be reused across runs. Remove reports from an
    # older invocation so a failure early in this layer cannot expose stale
    # files as if they came from the current artifact.
    find "${layer_dir}" -mindepth 1 -delete
}

finish_layer()
{
    log "PASS $1"
}
