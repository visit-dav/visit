#!/bin/sh
##############################################################################
#
# Purpose: Check for existence of #warning compiler directives. 
#
# Programmer: Mark C. Miller
# Created:    May 20, 2008 
#
##############################################################################
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

    #
    # Only do this check for files svn thinks are 'text' files
    #
    hasMimeTypeProp=`${SVNLOOK} proplist -t $TXN $REPOS $f | grep mime-type`
    if test -n "$hasMimeTypeProp"; then
        mimeTypeProp=`${SVNLOOK} propget -t $TXN $REPOS svn:mime-type $f`
        if test -n "$mimeTypeProp"; then
            if test -z "`echo $mimeTypeProp | grep ^text/`"; then
                continue
            fi
        fi
    fi

    # check if the file we're trying to commit is empty (a deletion?) 
    commitFileLineCount=`${SVNLOOK} cat -t $TXN $REPOS $f | wc -l`
    if test $commitFileLineCount -le 0; then
        continue;
    fi

    ${SVNLOOK} cat -t $TXN $REPOS $f | grep -q '^#warning' 1>/dev/null 2>&1
    commitFileHasPoundWarnings=$?

    # If the file we're committing has ctrl chars, reject it
    if test $commitFileHasPoundWarnings -eq 0; then
        log "File \"$f\" appears to contain '#warning' compilation directives."
        log "Please remove them before committing."
        exit 1
    fi
done

# all is well!
exit 0
