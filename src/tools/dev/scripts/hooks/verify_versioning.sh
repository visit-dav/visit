#!/bin/sh

# Modifications:
#   Mark C. Miller, Tue Dec  9 00:19:04 PST 2008
#   Obtain list of changed files via FLIST ($3) argument and loop
#   over them via 'read' sh builtin method.

PROPERTY="VisIt:VersionSpecificLogic"
REPOS="$1"
TXN="$2"
FLIST="$3"

while read fline; do

    #
    # Get file 'svnlook' status and name
    #
    fstat=`echo $fline | tr -s ' ' | cut -d' ' -f1`
    fname=`echo $fline | tr -s ' ' | cut -d' ' -f2`

    # It ends up with some WS chars if we don't sed those out, thus making it
    # a non-zero string..
    trunk=`echo "${fname}" | grep "trunk" | sed s/\ //g`
    if [ -n "${trunk}" ]; then
        # we're in the trunk.. who cares, let them do what they want.
        continue
    fi
    visit_prop=`svnlook -t $TXN propget $REPOS ${PROPERTY} "${fname}"`
    if [ -n "${visit_prop}" ]; then
        log "You cannot change the file ${fname}"
        log "You must:"
        log "1. save your modifications to ${fname}"
        log "2. svn propdel ${PROPERTY} ${fname}"
        log "3. svn commit -m 'need to modify' ${fname}"
        log "4. restore your modifications to ${fname}"
        log "5. redo this commit, which just FAILED"
        log "6. svn propset ${PROPERTY} true ${fname}"
        log "7. svn commit -m restoring property ${fname}"
        exit 1
    fi

done < $FLIST

# all is well!
exit 0
