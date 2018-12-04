#!/bin/sh
# Tom Fogal, August 2007
# Revamped Mon Jul 14 14:43:17 EDT 2008

## Modify these to suit your particular machine.
# absolute path to visit root dir:
V_DIR="${HOME}/visit"
# VisIt executable;
VISIT="${V_DIR}/src/bin/tjfvisit"
# -l, -p, etc. options:
PAR=

## You should not need to modify anything below this line.
host=`hostname`
TIMING="-timing -withhold-timing-output"
SCRIPT="-cli -s run.py"
VISIT_OPT="-noconfig -nowin -host ${host} ${V_DIR} ${TIMING} ${PAR}"
DB="${host}.db"

export V_DIR

# remove stale files from previous runs
function cleanUp()
{
    rm -f ~/*\.timings ./*.timings ./*.png
    rm -f visitlog.py
}

function doRun
{
    ${VISIT} ${VISIT_OPT} $@ ${SCRIPT}
}

extract_data()
{
    if ! test -f ${DB} ; then
        echo "'${DB}' does not exist, creating ..."
        sqlite3 ${DB} < create.sql
    fi
    awk --assign pat=NM::Render -f timing.awk "$@" | sqlite3 ${DB}
}

geometry=("1024x768+0+0"
          "1600x1200+0+0"
          "3200x3200+0+0"
          "4096x4096+0+0")

cleanUp
for geom in ${geometry[@]} ; do
    for procs in $(seq 8 8 32) ; do
        for mode in "-icet" "" ; do
            echo "mode is: ${mode}"
            args="-np ${procs} -geometry ${geom} ${mode}"
            doRun "${args}"
            extract_data engine_par.000.timings
            rm -f engine_par*timings
            cleanUp
        done
    done
done
