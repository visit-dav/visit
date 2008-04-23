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
#   Kathleen Bonnell, Tue Apr 22 08:35:39 PDT 2008 
#   Added windows project and solution files to be skipped by this hook.
#
#   Mark C. Miller, Tue Apr 22 09:07:58 PDT 2008
#   Added property filtering to only check text files
#
#   Mark C. Miller, Tue Apr 22 11:07:00 PDT 2008
#   Fixed property filtering method to first check for existence of mime-type
#   property. Also fixed check for file in repo to first check to see if the
#   file has zero size.
#
#   Mark C. Miller, Wed Apr 23 11:01:07 PDT 2008
#   Added build_visit to list of files we permit tabs in.
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
    # Filter out cases of files we permit tabs in.
    #
    case $f in
        *.in|*.html|*/third_party_builtin/*|*/common/icons/*|*.vcproj|*.sln)
            continue
            ;;
        */src/configure)
            continue
            ;;
        */svn_bin/build_visit)
            continue
            ;;
    esac

    # check if the file we're trying to commit has tabs
    ${SVNLOOK} cat -t $TXN $REPOS $f | grep -q '	' 1>/dev/null 2>&1
    commitFileHasTabs=$?

    # If the file we're committing has tabs, next check its predecessor
    # on the trunk.
    if test $commitFileHasTabs -eq 0; then

        fileLineCount=`${SVNLOOK} cat $REPOS $f | wc -l`
        if test $fileLineCount -gt 0; then
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
    fi
done

# all is well!
exit 0
