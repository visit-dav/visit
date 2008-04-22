#!/bin/sh
##############################################################################
#
# Purpose: Count tabs in committed files and make sure their number does not
#          increase.
#
# Programmer: Mark C. Miller
# Created:    Mon Apr 21 18:09:43 PDT 2008
#
# Modifications:
#   Mark C. Miller, Mon Apr 21 20:01:43 PDT 2008
#   Added src/configure to files to be skipped by this hook
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
    # Filter out cases of files we permit tabs in.
    #
    case $f in
        *.in|*.html|*/third_party_builtin/*|*/common/icons/*)
            continue
            ;;
        src/configure) 
            continue
            ;;
    esac

    # check if the file we're trying to commit has tabs
    ${SVNLOOK} cat -t $TXN $REPOS $f | grep -q '	' 1>/dev/null 2>&1
    commitFileHasTabs=$?

    # If the file we're committing has tabs, next check its predecessor
    # on the trunk.
    if test $commitFileHasTabs -eq 0; then

        ${SVNLOOK} cat $REPOS $f | grep -q '	'
        repoFileHasTabs=$?
        if test $repoFileHasTabs -eq 0; then
            commitFileWordCount1=`${SVNLOOK} cat -t $TXN $REPOS $f | wc -l`
            commitFileWordCount2=`${SVNLOOK} cat -t $TXN $REPOS $f | tr '\t' '\n' | wc -l`
            commitFileTabCount=`expr $commitFileWordCount2 - $commitFileWordCount1`
            repoFileWordCount1=`${SVNLOOK} cat $REPOS $f | wc -l`
            repoFileWordCount2=`${SVNLOOK} cat $REPOS $f | tr '\t' '\n' | wc -l`
            repoFileTabCount=`expr $repoFileWordCount2 - $repoFileWordCount1`
            if test $commitFileTabCount -gt $repoFileTabCount; then
                log "In a file you are commiting, \"$f\", you have increased "
                log "the number of tabs from $repoFileTabCount to $commitFileTabCount."
                exit 1
            fi
        else
            log "A file you are commiting, \"$f\", has tabs"
            exit 1
        fi
    fi
done

# all is well!
exit 0
