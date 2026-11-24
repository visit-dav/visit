#!/usr/bin/env bash
set -euo pipefail

source "${VISIT_TEST_ROOT:-/opt/visit-test}/lib/common.sh"

start_layer gui-smoke
[[ -n "${DISPLAY:-}" ]] || die "DISPLAY is required; run the Xvfb sidecar and set DISPLAY=:99"

if [[ -r "${VISIT_ROOT}/data/curv2d.silo" ]]; then
    export VISIT_TEST_DATA="${VISIT_ROOT}/data/curv2d.silo"
    export VISIT_TEST_VARIABLE="d"
else
    export VISIT_TEST_DATA="${VISIT_TEST_ROOT}/data/smoke.vtk"
    export VISIT_TEST_VARIABLE="value"
fi
export VISIT_RENDER_DIR="${VISIT_RESULTS_DIR}/gui-smoke"
export LIBGL_ALWAYS_SOFTWARE="${LIBGL_ALWAYS_SOFTWARE:-1}"
unset LIBGL_ALWAYS_INDIRECT
unset QT_XCB_GL_INTEGRATION
unset QT_DEBUG_PLUGINS
export QT_X11_NO_MITSHM="${QT_X11_NO_MITSHM:-1}"
export LIBGL_DEBUG="${LIBGL_DEBUG:-verbose}"

write_failure_report()
{
    local report="${VISIT_RENDER_DIR}/gui_failure_report.txt"
    local name

    : >"${report}"
    for name in \
        display_environment.txt \
        xdpyinfo.txt \
        glxinfo.txt \
        viewer_process.log \
        visit.log \
        diagnostic_files.txt
    do
        [[ -s "${VISIT_RENDER_DIR}/${name}" ]] || continue
        printf '\n===== %s =====\n' "${name}" >>"${report}"
        tail -n 200 "${VISIT_RENDER_DIR}/${name}" >>"${report}"
    done
    for name in "${VISIT_RENDER_DIR}"/*.vlog; do
        [[ -s "${name}" ]] || continue
        printf '\n===== %s =====\n' "$(basename "${name}")" >>"${report}"
        tail -n 200 "${name}" >>"${report}"
    done
}

# Prove that the runtime container can reach and interrogate the sidecar
# display before starting VisIt.  Viewer stderr is not reliably relayed by
# the VisIt launcher, so retain enough context to diagnose display failures.
{
    printf 'DISPLAY=%s\n' "${DISPLAY}"
    printf 'LIBGL_ALWAYS_SOFTWARE=%s\n' "${LIBGL_ALWAYS_SOFTWARE}"
    printf 'LIBGL_ALWAYS_INDIRECT=<unset>\n'
    printf 'LIBGL_DEBUG=%s\n' "${LIBGL_DEBUG}"
    printf 'QT_DEBUG_PLUGINS=<unset>\n'
    printf 'QT_XCB_GL_INTEGRATION=<unset>\n'
    printf 'QT_X11_NO_MITSHM=%s\n' "${QT_X11_NO_MITSHM}"
    printf '\nMesa/GLX libraries visible to the dynamic loader:\n'
    ldconfig -p 2>/dev/null | grep -E 'lib(EGL|GL|GLX|OpenGL|gbm|drm)' || true
} >"${VISIT_RENDER_DIR}/display_environment.txt" 2>&1

if ! xdpyinfo -display "${DISPLAY}" \
    >"${VISIT_RENDER_DIR}/xdpyinfo.txt" 2>&1; then
    cat "${VISIT_RENDER_DIR}/xdpyinfo.txt" >&2
    die "cannot connect to the Xvfb display ${DISPLAY}"
fi

if ! glxinfo -display "${DISPLAY}" -B \
    >"${VISIT_RENDER_DIR}/glxinfo.txt" 2>&1; then
    cat "${VISIT_RENDER_DIR}/glxinfo.txt" >&2
    die "cannot create a software GLX context on ${DISPLAY}"
fi

# DebugStream flushes every log record.  A Windows bind mount makes level-5
# plugin and GLX diagnostics slow enough to trip the test timeout, so write
# logs to the container layer and copy them out when the command completes.
debug_dir="$(mktemp -d "${TMPDIR}/visit-gui-debug.XXXXXX")"

preserve_debug_files()
{
    find "${debug_dir}" -maxdepth 1 -type f \
        -exec cp -f '{}' "${VISIT_RENDER_DIR}/" ';'
}

cd "${debug_dir}"

# Interpose a transparent wrapper around the viewer in this disposable
# installation.  It preserves the real process's environment, arguments, and
# exit status while bypassing the launcher's suppression of early stderr.
viewer_binary="$(find -L "${VISIT_ROOT}" -type f \
    -path '*/linux-x86_64/bin/viewer' -perm -u+x -print -quit \
    2>/dev/null || true)"
if [[ -n "${viewer_binary}" ]]; then
    export VISIT_VIEWER_REAL="${viewer_binary}.visit-test-real"
    export VISIT_VIEWER_CAPTURE_LOG="${debug_dir}/viewer_process.log"
    : >"${VISIT_VIEWER_CAPTURE_LOG}"
    mv "${viewer_binary}" "${VISIT_VIEWER_REAL}"
    cp "${VISIT_TEST_ROOT}/bin/viewer-capture.sh" "${viewer_binary}"
    chmod 0755 "${viewer_binary}"
else
    # Capturing stderr is diagnostic instrumentation, not a test
    # prerequisite.  Continue with the unmodified installation if an unusual
    # package layout prevents interposition.
    log "WARNING: cannot interpose the architecture-specific viewer; continuing without viewer stderr capture"
fi
ulimit -c unlimited 2>/dev/null || true

run_visit -debug 1 -cli -ni -s "${VISIT_TEST_ROOT}/tests/gui_render.py" \
    >"${VISIT_RESULTS_DIR}/gui-smoke/visit.log" 2>&1 || {
        preserve_debug_files
        find "${VISIT_RENDER_DIR}" -maxdepth 1 -type f -printf '%f\n' \
            | sort >"${VISIT_RENDER_DIR}/diagnostic_files.txt"
        write_failure_report
        cat "${VISIT_RENDER_DIR}/gui_failure_report.txt" >&2
        die "VisIt GUI/rendering smoke test failed"
    }

preserve_debug_files

if grep -Fq 'Traceback (most recent call last):' \
    "${VISIT_RESULTS_DIR}/gui-smoke/visit.log"; then
    find "${VISIT_RENDER_DIR}" -maxdepth 1 -type f -printf '%f\n' \
        | sort >"${VISIT_RENDER_DIR}/diagnostic_files.txt"
    write_failure_report
    cat "${VISIT_RENDER_DIR}/gui_failure_report.txt" >&2
    die "VisIt GUI/rendering smoke test reported a Python traceback"
fi

grep -Fq 'VISIT_GUI_SMOKE_OK' "${VISIT_RESULTS_DIR}/gui-smoke/visit.log" || \
    die "GUI success marker was not written"
[[ -s "${VISIT_RESULTS_DIR}/gui-smoke/visit_gui_smoke.png" ]] || \
    die "GUI smoke test did not produce visit_gui_smoke.png"

# VisIt's signal handler can convert a shutdown-time segmentation fault into
# a successful process exit status. Inspect the retained component logs so a
# rendered image and success marker cannot hide a crash during shutdown.
crash_log=""
for component_log in "${VISIT_RENDER_DIR}"/*.vlog; do
    [[ -f "${component_log}" ]] || continue
    if grep -Fq 'signalhandler_core: SIGSEGV!' "${component_log}"; then
        crash_log="${component_log}"
        break
    fi
done
if [[ -n "${crash_log}" ]]; then
    find "${VISIT_RENDER_DIR}" -maxdepth 1 -type f -printf '%f\n' \
        | sort >"${VISIT_RENDER_DIR}/diagnostic_files.txt"
    write_failure_report
    cat "${VISIT_RENDER_DIR}/gui_failure_report.txt" >&2
    die "VisIt component crashed during GUI test shutdown: $(basename "${crash_log}")"
fi
finish_layer gui-smoke
