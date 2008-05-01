#!/bin/sh
##############################################################################
#
# Purpose: Count ctrl characters in committed files and make sure we don't
#          commit files with ctrl characters.
#
# Programmer: Mark C. Miller
# Created:    April 30, 2008
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

    #
    # Filter out some cases that don't make sense that
    # above logic doesn't catch.
    #
    case $f in
        *.ini)
            continue
            ;;
        */src/third_party_builtin/*|*/src/common/icons/*)
            continue
            ;;
        */src/exe/*|*/src/bin/*)
            continue
            ;;
        */src/java/images/*)
            continue
            ;;
        */src/tools/qtssh/remotecommand/*)
            continue
            ;;
    esac

    # check if the file we're trying to commit is empty (a deletion?) 
    commitFileLineCount=`${SVNLOOK} cat -t $TXN $REPOS $f | wc -l`
    if test $commitFileLineCount -le 0; then
        continue;
    fi

    ${SVNLOOK} cat -t $TXN $REPOS $f | grep -q -f ctrl_chars.txt 1>/dev/null 2>&1
    commitFileHasCtrlChars=$?

    # If the file we're committing has ctrl chars, reject it
    if test $commitFileHasCtrlChars -eq 0; then
        log "File \"$f\" appears to contain ctrl characters, maybe from dos?."
        log "Please remove them before committing."
        exit 1
    fi
done

# all is well!
exit 0
