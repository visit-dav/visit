#!/bin/bash

SVNLOOK="/usr/bin/svnlook"
AWK="/usr/bin/awk"
PROPERTY="VisIt:VersionSpecificLogic"

REPOS="$1"
TXN="$2"

function log()
{
    echo "$@" 1>&2
}

if [ -z "${REPOS}" ]; then
    log "Repository path not given, bailing out."
    exit 1
fi
if [ -z "${TXN}" ]; then
    log "Transaction ID not given, bailing out."
    exit 1
fi

files=`${SVNLOOK} changed -t $TXN $REPOS | ${AWK} '{print $2}'`
for f in ${files} ; do
    # It ends up with some WS chars if we don't sed those out, thus making it
    # a non-zero string..
    trunk=`echo "${f}" | grep "trunk" | /bin/sed s/\ //g`
    if [ -n "${trunk}" ]; then
        # we're in the trunk.. who cares, let them do what they want.
        continue
    fi
    visit_prop=`${SVNLOOK} -t $TXN propget $REPOS ${PROPERTY} "${f}"`
    if [ -n "${visit_prop}" ]; then
        log "You cannot change the file ${f}"
        log "You must:"
        log "1. save your modifications to ${f}"
        log "2. svn propdel ${PROPERTY} ${f}"
        log "3. svn commit -m 'need to modify' ${f}"
        log "4. restore your modifications to ${f}"
        log "5. redo this commit, which just FAILED"
        log "6. svn propset ${PROPERTY} true ${f}"
        log "7. svn commit -m restoring property ${f}"
        exit 1
    fi
done
# all is well!
exit 0
