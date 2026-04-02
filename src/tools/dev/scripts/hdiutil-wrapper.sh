#!/bin/bash -x

REAL_HDIUTIL="/usr/bin/hdiutil"
LOGFILE="hdiutil-$(date +%Y%m%d-%H%M%S)-$$.log"

# Send everything to the log, including shell trace from -x.
exec >>"$LOGFILE" 2>&1

echo "=================================================="
echo "time: $(date)"
echo "log:  $LOGFILE"
echo "cwd:  $(pwd)"
echo "argv: $0 $*"
echo "pid:  $$"
echo "ppid: $PPID"
echo "===== mount ====="
mount
echo "===== hdiutil ====="
hdiutil info
echo "===== diskutil ====="
diskutil list
echo "=================================================="

out="${!#}" # last argument is output file
dout=$(dirname $out)
bout1=$(basename $out)
bout2=$(basename $out .dmg)
if [[ "$bout1" = "$bout2" ]]; then
    uout=${out}
else
    uout=${dout}/${bout2}.$$.dmg
fi

newargs=()
for arg in "$@"; do
    if [ "$arg" = "create" ]; then
        newargs+=("$arg")
        newargs+=("-verbose")
        newargs+=("-debug")
        newargs+=("-plist")
        continue
    fi
    if [ "$arg" = "$out" ]; then
        newargs+=("$uout")
        continue
    fi
    newargs+=("$arg")
done
set -- "${newargs[@]}"

max_tries=6
sleep_secs=2
try=1

while true
do
    echo
    echo "----- attempt hdiutil $try of $max_tries -----"
    date
    hdiutil info
    diskutil list
    mount
    ps aux | egrep 'hdiutil|diskimagesd|diskarbitrationd|Finder|QuickLook|mds|mdworker'
    lsof | grep -i 'dmg\|_CPack_Packages\|/Volumes/'

    # Capture stdout/stderr so we can inspect it for Resource busy.
    output="$("$REAL_HDIUTIL" "$@" 2>&1)"
    rc=$?

    printf '%s\n' "$output"
    echo "exit code: $rc"

    if [ $rc -eq 0 ]; then
        echo "success"
        if [[ "$uout" != "$out" ]]; then
            mv ${uout} ${out}
        fi
        exit 0
    fi

    if [ $try -eq $max_tries ]; then
        echo "failed after $max_tries"
        exit $rc
    fi

    sleep $sleep_secs
    try=$((try + 1))
    sleep_secs=$((sleep_secs * 2))

done
