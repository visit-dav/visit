#!/usr/bin/env bash
set -euo pipefail

source "${VISIT_TEST_ROOT:-/opt/visit-test}/lib/common.sh"

start_layer elf-audit
require_visit

result_dir="${VISIT_RESULTS_DIR}/elf-audit"
needed_file="${result_dir}/direct_needed_sonames.txt"
resolved_file="${result_dir}/resolved_deps.txt"
candidate_paths_file="${result_dir}/candidate_bundle_paths.txt"
candidate_sonames_file="${result_dir}/candidate_bundle_sonames.txt"
missing_file="${result_dir}/missing_sonames.txt"
optional_missing_file="${result_dir}/optional_missing_sonames.txt"
host_violations_file="${result_dir}/host_dependency_violations.txt"
rpath_file="${result_dir}/rpaths.txt"
bad_rpath_file="${result_dir}/bad_rpaths.txt"
glibc_file="${result_dir}/required_glibc_versions.txt"
bad_glibc_file="${result_dir}/glibc_baseline_violations.txt"
missing_unique_file="${result_dir}/missing_sonames_unique.txt"
optional_missing_unique_file="${result_dir}/optional_missing_sonames_unique.txt"
host_unique_file="${result_dir}/host_sonames_unique.txt"
bad_rpath_unique_file="${result_dir}/bad_rpaths_unique.txt"
elf_list="${result_dir}/elf_files.txt"

: > "${needed_file}"
: > "${resolved_file}"
: > "${candidate_paths_file}"
: > "${candidate_sonames_file}"
: > "${missing_file}"
: > "${optional_missing_file}"
: > "${host_violations_file}"
: > "${rpath_file}"
: > "${bad_rpath_file}"
: > "${glibc_file}"
: > "${bad_glibc_file}"
: > "${missing_unique_file}"
: > "${optional_missing_unique_file}"
: > "${host_unique_file}"
: > "${bad_rpath_unique_file}"
: > "${elf_list}"

while IFS= read -r -d '' path; do
    if readelf -h "${path}" >/dev/null 2>&1; then
        printf '%s\n' "${path}" >> "${elf_list}"
    fi
done < <(find "${VISIT_ROOT}" -type f -print0)

bundle_lib_path="$(find "${VISIT_ROOT}" -type f -name '*.so*' -printf '%h\n' \
    | sort -u | paste -sd: -)"

is_allowed_host_soname()
{
    local soname="$1"
    case "${soname}" in
        linux-vdso.so.*|ld-linux*.so.*|libc.so.*|libm.so.*|libpthread.so.*|\
        librt.so.*|libdl.so.*|libutil.so.*|libresolv.so.*|libnss_*.so.*|\
        libanl.so.*)
            return 0
            ;;
    esac

    case ":${VISIT_ALLOWED_HOST_SONAMES:-}:" in
        *:"${soname}":*) return 0 ;;
    esac
    return 1
}

is_optional_missing_dependency()
{
    local relative="$1"
    local soname="$2"
    local consumer="${relative##*/}"

    # OSPRay loads these acceleration libraries only when the corresponding
    # hardware/runtime support is available. Their transitive dependencies
    # are therefore optional on hosts that do not provide that support.
    case "${consumer}" in
        libtbbbind*.so*)
            [[ "${soname}" == libhwloc.so.* ]]
            return
            ;;
        libispcrt_device_gpu.so*)
            [[ "${soname}" == libze_loader.so.* ]]
            return
            ;;
    esac
    return 1
}

while IFS= read -r elf; do
    relative="${elf#${VISIT_ROOT}/}"

    while IFS= read -r soname; do
        [[ -n "${soname}" ]] && printf '%s\t%s\n' "${relative}" "${soname}" >> "${needed_file}"
    done < <(readelf -d "${elf}" 2>/dev/null \
        | sed -n 's/.*(NEEDED).*\[\([^]]*\)\].*/\1/p')

    while IFS= read -r dynamic_path; do
        [[ -n "${dynamic_path}" ]] || continue
        printf '%s\t%s\n' "${relative}" "${dynamic_path}" >> "${rpath_file}"
        IFS=: read -r -a entries <<<"${dynamic_path}"
        for entry in "${entries[@]}"; do
            if [[ "${entry}" == /* && "${entry}" != "${VISIT_ROOT}"* ]]; then
                printf '%s\t%s\n' "${relative}" "${entry}" >> "${bad_rpath_file}"
            fi
        done
    done < <(readelf -d "${elf}" 2>/dev/null \
        | sed -n 's/.*(RPATH).*\[\([^]]*\)\].*/\1/p; s/.*(RUNPATH).*\[\([^]]*\)\].*/\1/p')

    while IFS= read -r glibc_version; do
        [[ -n "${glibc_version}" ]] || continue
        printf '%s\t%s\n' "${relative}" "${glibc_version}" >> "${glibc_file}"
        highest="$(printf '%s\n%s\n' "${VISIT_GLIBC_BASELINE:-2.28}" "${glibc_version}" | sort -V | tail -n 1)"
        if [[ "${highest}" == "${glibc_version}" && "${glibc_version}" != "${VISIT_GLIBC_BASELINE:-2.28}" ]]; then
            printf '%s\tGLIBC_%s\n' "${relative}" "${glibc_version}" >> "${bad_glibc_file}"
        fi
    done < <(readelf --version-info "${elf}" 2>/dev/null \
        | sed -n 's/.*Name: GLIBC_\([0-9][0-9.]*\).*/\1/p' | sort -Vu)

    ldd_output="$(LD_LIBRARY_PATH="${bundle_lib_path}${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}" ldd "${elf}" 2>&1 || true)"
    while IFS= read -r line; do
        printf '%s\t%s\n' "${relative}" "${line}" >> "${resolved_file}"
        if [[ "${line}" =~ ^[[:space:]]*([^[:space:]]+)[[:space:]]+\=\>[[:space:]]+not[[:space:]]+found ]]; then
            soname="${BASH_REMATCH[1]}"
            if is_optional_missing_dependency "${relative}" "${soname}"; then
                printf '%s\t%s\n' "${relative}" "${soname}" >> "${optional_missing_file}"
            else
                printf '%s\t%s\n' "${relative}" "${soname}" >> "${missing_file}"
            fi
        elif [[ "${line}" =~ ^[[:space:]]*([^[:space:]]+)[[:space:]]+\=\>[[:space:]]+(/[^[:space:]]+) ]]; then
            soname="${BASH_REMATCH[1]}"
            resolved="${BASH_REMATCH[2]}"
            if [[ "${resolved}" != "${VISIT_ROOT}"/* ]] && ! is_allowed_host_soname "${soname}"; then
                printf '%s\t%s\t%s\n' "${relative}" "${soname}" "${resolved}" >> "${host_violations_file}"
                printf '%s\n' "${resolved}" >> "${candidate_paths_file}"
                printf '%s\n' "${soname}" >> "${candidate_sonames_file}"
            fi
        fi
    done <<<"${ldd_output}"
done < "${elf_list}"

for report in \
    "${needed_file}" "${resolved_file}" "${candidate_paths_file}" \
    "${candidate_sonames_file}" "${missing_file}" "${optional_missing_file}" \
    "${host_violations_file}" \
    "${rpath_file}" "${bad_rpath_file}" "${glibc_file}" "${bad_glibc_file}"
do
    sort -u -o "${report}" "${report}"
done

awk -F '\t' 'NF >= 2 {print $2}' "${missing_file}" | sort -u > "${missing_unique_file}"
awk -F '\t' 'NF >= 2 {print $2}' "${optional_missing_file}" \
    | sort -u > "${optional_missing_unique_file}"
awk -F '\t' 'NF >= 2 {print $2}' "${host_violations_file}" | sort -u > "${host_unique_file}"
awk -F '\t' 'NF >= 2 {print $2}' "${bad_rpath_file}" | sort -u > "${bad_rpath_unique_file}"

failures=0
if [[ -s "${missing_file}" ]]; then
    log "Unresolved shared libraries found in ${missing_file}"
    failures=1
fi
if [[ -s "${optional_missing_file}" ]]; then
    log "NOTICE: optional OSPRay dependencies recorded in ${optional_missing_file}"
fi
if [[ -s "${host_violations_file}" ]]; then
    if [[ "${VISIT_STRICT_HOST_DEPS:-0}" == "1" ]]; then
        log "Unexpected host-resolved libraries found in ${host_violations_file}"
        failures=1
    else
        log "NOTICE: host-resolved libraries recorded in ${host_violations_file}"
    fi
fi
if [[ -s "${bad_rpath_file}" ]]; then
    if [[ "${VISIT_STRICT_RPATHS:-0}" == "1" ]]; then
        log "External absolute RPATH entries found in ${bad_rpath_file}"
        failures=1
    else
        log "NOTICE: external absolute RPATH entries recorded in ${bad_rpath_file}"
    fi
fi
if [[ -s "${bad_glibc_file}" ]]; then
    log "Requirements newer than GLIBC_${VISIT_GLIBC_BASELINE:-2.28} found in ${bad_glibc_file}"
    failures=1
fi

((failures == 0)) || die "ELF portability audit failed"
finish_layer elf-audit
