#!/usr/bin/env bash
set -euo pipefail

source "${VISIT_TEST_ROOT:-/opt/visit-test}/lib/common.sh"

start_layer cli-smoke

run_visit -cli -nowin -ni -s "${VISIT_TEST_ROOT}/tests/cli_smoke.py" \
    >"${VISIT_RESULTS_DIR}/cli-smoke/visit.log" 2>&1 || {
        cat "${VISIT_RESULTS_DIR}/cli-smoke/visit.log" >&2
        die "VisIt CLI smoke test failed"
    }

if grep -Fq 'Traceback (most recent call last):' \
    "${VISIT_RESULTS_DIR}/cli-smoke/visit.log"; then
    cat "${VISIT_RESULTS_DIR}/cli-smoke/visit.log" >&2
    die "VisIt CLI smoke test reported a Python traceback"
fi

grep -Fq 'VISIT_CLI_SMOKE_OK' "${VISIT_RESULTS_DIR}/cli-smoke/visit.log" || \
    die "CLI success marker was not written"
finish_layer cli-smoke
