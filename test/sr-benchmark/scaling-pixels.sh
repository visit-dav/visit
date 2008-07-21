#!/bin/sh
# Tom Fogal, August 2007
# Revamped Mon Jul 14 14:43:17 EDT 2008

VISIT="${HOME}/visit/src/bin/tjfvisit"
TIMING="-timing -withhold-timing-output"
SCRIPT="-cli -s run.py"

function cleanUp()
{
    # remove stale files from previous runs
    rm -f ~/*\.timings
    rm -f visitlog.py
    rm -f result*.png viewer.timings ~/mdserver.timings
}

function doRun
{
    host=`hostname`
    ${VISIT} -host ${host} ${TIMING} $@ ${SCRIPT}
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
          "1280x1024+0+0"
          "1300x975+0+0"
          "1400x1050+0+0"
          "1500x1125+0+0"
          "1600x1200+0+0")

rm -f ./*\.timings

for geom in ${geometry[@]} ; do
    for procs in $(seq 8 8 32) ; do
        for mode in "-icet" "" ; do
            echo "mode is: ${mode}"
            args="-np ${procs} -geometry ${geom} ${mode}"
            doRun "${args}"
            extract_data engine_par.000.timings
            rm -f engine_par*timings
        done
    done
done
