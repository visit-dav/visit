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

doRun "-np 4 -geometry 1024x768+0+0"
#doRun "-geometry 1100x825+0+0"
#doRun "-geometry 1280x1024+0+0"
#doRun "-geometry 1300x975+0+0"
#doRun "-geometry 1400x1050+0+0"
#doRun "-geometry 1500x1125+0+0"
#doRun "-geometry 1600x1200+0+0"
