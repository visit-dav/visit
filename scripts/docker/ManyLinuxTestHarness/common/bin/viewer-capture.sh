#!/usr/bin/env bash

# Installed temporarily in place of the versioned viewer by the GUI test. VisIt's
# launcher does not reliably relay a viewer that dies during initialization,
# so capture the component's own stdout and stderr before replacing this
# wrapper process with the real executable.
set -u

: "${VISIT_VIEWER_REAL:?VISIT_VIEWER_REAL is required}"
: "${VISIT_VIEWER_CAPTURE_LOG:?VISIT_VIEWER_CAPTURE_LOG is required}"

{
    printf 'viewer executable: %s\n' "${VISIT_VIEWER_REAL}"
    printf 'viewer arguments:'
    printf ' <%s>' "$@"
    printf '\n'
} >>"${VISIT_VIEWER_CAPTURE_LOG}"

# Preserve argv[0] so VisIt's component/version discovery sees the original
# viewer name rather than the temporary ".visit-test-real" suffix.
exec -a "$0" "${VISIT_VIEWER_REAL}" "$@" \
    >>"${VISIT_VIEWER_CAPTURE_LOG}" 2>&1
