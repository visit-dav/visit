#!/usr/bin/env bash
set -euo pipefail

source "${VISIT_TEST_ROOT:-/opt/visit-test}/lib/common.sh"

start_layer layout
require_visit

machine="$(uname -m)"
printf '%s\n' "${machine}" > "${VISIT_RESULTS_DIR}/layout/container_architecture.txt"
case "${machine}" in
    x86_64|amd64) ;;
    *) die "the linux-x86_64 VisIt distribution requires an amd64 container; uname -m reported ${machine}" ;;
esac

python_dir="${VISIT_ROOT}/current/linux-x86_64/bin"
{
    printf 'Expected Python directory: %s\n' "${python_dir}"
    if [[ -d "${python_dir}" ]]; then
        find "${python_dir}" -maxdepth 1 \
            \( -type f -o -type l \) -name 'python*' \
            -printf '%M %p -> %l\n' | sort
    else
        printf 'Directory does not exist. Python candidates under install root:\n'
        find "${VISIT_ROOT}" \( -type f -o -type l \) -path '*/bin/python*' -print | sort
    fi
} > "${VISIT_RESULTS_DIR}/layout/python_layout.txt"

if [[ ! -x "${python_dir}/python" && ! -x "${python_dir}/python3" ]]; then
    cat "${VISIT_RESULTS_DIR}/layout/python_layout.txt" >&2
    die "VisIt's launcher cannot find an executable bundled python or python3"
fi

broken_links="${VISIT_RESULTS_DIR}/layout/broken_symlinks.txt"
find -L "${VISIT_ROOT}" -type l -print | sort > "${broken_links}"
[[ ! -s "${broken_links}" ]] || die "the installed distribution contains broken symlinks; see ${broken_links}"

version_log="${VISIT_RESULTS_DIR}/layout/version.log"
set +e
run_visit -version >"${version_log}" 2>&1
version_status=$?
set -e
if ((version_status != 0)); then
    cat "${version_log}" >&2
    die "visit -version failed with exit code ${version_status}; see ${version_log}"
fi

version_output="$(cat "${version_log}")"
printf '%s\n' "${version_output}" | tee "${VISIT_RESULTS_DIR}/layout/version.txt"
grep -Fq "${VISIT_VERSION}" <<<"${version_output}" || \
    die "launcher did not report expected version ${VISIT_VERSION}"

elf_count=0
while IFS= read -r -d '' path; do
    if readelf -h "${path}" >/dev/null 2>&1; then
        ((elf_count += 1))
    fi
done < <(find "${VISIT_ROOT}" -type f -print0)

((elf_count > 0)) || die "no ELF files found below ${VISIT_ROOT}"
printf '%s\n' "${elf_count}" > "${VISIT_RESULTS_DIR}/layout/elf_count.txt"
finish_layer layout
