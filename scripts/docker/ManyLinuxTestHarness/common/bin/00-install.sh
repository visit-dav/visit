#!/usr/bin/env bash
set -euo pipefail

source "${VISIT_TEST_ROOT:-/opt/visit-test}/lib/common.sh"

start_layer install
resolve_visit_archive_and_version

[[ "${VISIT_TEST_IMAGE_REVISION:-}" =~ ^[0-9]+$ ]] || \
    die "runtime test image has no valid revision marker"
command -v cpio >/dev/null 2>&1 || \
    die "cpio is missing from the test image; rebuild it without --no-build"
{
    printf 'image_revision=%s\n' "${VISIT_TEST_IMAGE_REVISION}"
    printf 'test_profile=%s\n' "${VISIT_TEST_PROFILE:-unknown}"
    printf 'visit_version=%s\n' "${VISIT_VERSION}"
    printf 'visit_archive=%s\n' "$(basename "${VISIT_ARCHIVE}")"
    printf 'architecture=%s\n' "$(uname -m)"
    printf 'cpio=%s\n' "$(command -v cpio)"
    if command -v rpm >/dev/null 2>&1; then
        rpm -q cpio || true
    elif command -v dpkg-query >/dev/null 2>&1; then
        dpkg-query -W -f='cpio_package=${Package} ${Version}\n' cpio || true
    fi
} > "${VISIT_RESULTS_DIR}/install/image_environment.txt"

if [[ -x "${VISIT_ROOT}/bin/visit" || -x "${VISIT_ROOT}/${VISIT_VERSION}/bin/visit" ]]; then
    log "VisIt is already installed below ${VISIT_ROOT}; installation skipped"
    finish_layer install
    exit 0
fi

archive="${VISIT_ARCHIVE}"

archive_manifest="${VISIT_RESULTS_DIR}/install/archive_contents.txt"
archive_python_entries="${VISIT_RESULTS_DIR}/install/archive_python_entries.txt"
sha256sum "${archive}" > "${VISIT_RESULTS_DIR}/install/archive.sha256"
tar -tf "${archive}" > "${archive_manifest}" || die "cannot read archive contents: ${archive}"
grep -E '(^|/)bin/python([^/]*)$' "${archive_manifest}" \
    > "${archive_python_entries}" || true

if [[ ! -s "${archive_python_entries}" ]]; then
    die "the VisIt archive contains no bin/python* entries; see ${archive_manifest} and archive.sha256"
fi

installer="${VISIT_INSTALLER:-}"
if [[ -z "${installer}" ]]; then
    encoded_version="${VISIT_VERSION//./_}"
    installer="$(find "${VISIT_ARTIFACT_DIR}" -maxdepth 3 -type f \
        \( -name 'visit-install' -o -name "visit-install${encoded_version}" \
           -o -name "visit-install${VISIT_VERSION}" \) \
        -print -quit)"
fi

# Developer-created archives may contain scripts/visit-install. Extract just
# that script when no separate release installer was mounted. If the binary
# package does not contain it, a source archive beside the binary package may
# provide the installer, but must never be selected as the distribution.
if [[ -z "${installer}" ]]; then
    installer_archive="${archive}"
    installer_member="$(tar -tf "${archive}" | grep -E '(^|/)scripts/visit-install$' | head -n 1 || true)"
    if [[ -z "${installer_member}" ]]; then
        while IFS= read -r source_archive; do
            installer_member="$(tar -tf "${source_archive}" 2>/dev/null \
                | grep -E '(^|/)scripts/visit-install$' | head -n 1 || true)"
            if [[ -n "${installer_member}" ]]; then
                installer_archive="${source_archive}"
                break
            fi
        done < <(find "${VISIT_ARTIFACT_DIR}" -maxdepth 1 -type f \
            -name "visit${encoded_version}.src.tar*" -print | sort)
    fi
    if [[ -n "${installer_member}" ]]; then
        installer="/work/visit-install"
        tar -xOf "${installer_archive}" "${installer_member}" > "${installer}"
        chmod 0755 "${installer}"
    fi
fi

[[ -n "${installer}" && -r "${installer}" ]] || \
    die "no visit-install script found; mount it in ${VISIT_ARTIFACT_DIR} or set VISIT_INSTALLER"

installer_copy="/work/visit-install-driver"
cp "${installer}" "${installer_copy}"
chmod 0755 "${installer_copy}"

log "Installing $(basename "${archive}") into ${VISIT_ROOT}"
"${installer_copy}" -c none -f "${archive}" "${VISIT_ROOT}" \
    >"${VISIT_RESULTS_DIR}/install/install.log" 2>&1 || {
        cat "${VISIT_RESULTS_DIR}/install/install.log" >&2
        die "visit-install failed"
    }

require_visit
find "${VISIT_ROOT}" -maxdepth 5 -printf '%M %p -> %l\n' | sort \
    > "${VISIT_RESULTS_DIR}/install/installed_layout.txt"
find "${VISIT_ROOT}" \( -type f -o -type l \) -path '*/bin/python*' \
    -printf '%M %p -> %l\n' | sort \
    > "${VISIT_RESULTS_DIR}/install/installed_python_entries.txt"

if [[ ! -s "${VISIT_RESULTS_DIR}/install/installed_python_entries.txt" ]]; then
    die "Python is present in the archive but visit-install did not install it; see install reports"
fi
log "Installed launcher: ${VISIT_BIN}"
finish_layer install
