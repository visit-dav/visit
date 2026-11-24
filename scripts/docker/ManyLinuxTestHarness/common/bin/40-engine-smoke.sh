#!/usr/bin/env bash
set -euo pipefail

source "${VISIT_TEST_ROOT:-/opt/visit-test}/lib/common.sh"

start_layer engine-smoke
if [[ -r "${VISIT_ROOT}/data/curv2d.silo" ]]; then
    export VISIT_TEST_DATA="${VISIT_ROOT}/data/curv2d.silo"
    export VISIT_TEST_VARIABLE="d"
else
    export VISIT_TEST_DATA="${VISIT_TEST_ROOT}/data/smoke.vtk"
    export VISIT_TEST_VARIABLE="value"
fi
export VISIT_RENDER_DIR="${VISIT_RESULTS_DIR}/engine-smoke"

run_visit -cli -nowin -ni -s "${VISIT_TEST_ROOT}/tests/engine_smoke.py" \
    >"${VISIT_RESULTS_DIR}/engine-smoke/visit.log" 2>&1 || {
        cat "${VISIT_RESULTS_DIR}/engine-smoke/visit.log" >&2
        die "VisIt engine smoke test failed"
    }

if grep -Fq 'Traceback (most recent call last):' \
    "${VISIT_RESULTS_DIR}/engine-smoke/visit.log"; then
    cat "${VISIT_RESULTS_DIR}/engine-smoke/visit.log" >&2
    die "VisIt engine smoke test reported a Python traceback"
fi

grep -Fq 'VISIT_ENGINE_SMOKE_OK' "${VISIT_RESULTS_DIR}/engine-smoke/visit.log" || \
    die "engine success marker was not written"
[[ -s "${VISIT_RESULTS_DIR}/engine-smoke/visit_engine_smoke.png" ]] || \
    die "headless engine test did not produce visit_engine_smoke.png"
finish_layer engine-smoke
