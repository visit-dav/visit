#!/bin/sh
##############################################################################
#
# Purpose: Check that we aren't committing any file with GPL license.
#
# Programmer: Mark C. Miller
# Created:    November 24, 2008 
#
# Modifications:
#
#   Mark C. Miller, Tue Dec  2 16:01:01 PST 2008
#   Added aclocal.m4 th permitted cases.
#   
#   Kathleen Bonnell, Fri Dec  5 08:05:54 PST 2008
#   Added qt includes in windowsbuild to permitted cases.
#
#   Mark C. Miller, Tue Dec  9 00:19:04 PST 2008
#   Obtain list of changed files via FLIST ($3) argument and loop
#   over them via 'read' sh builtin method.
#
#   Eric Brugger, Tue Dec  9 17:58:52 PST 2008
#   Added docs/WebSite/new.html to the list of permitted cases.
#
##############################################################################
REPOS="$1"
TXN="$2"
FLIST="$3"

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

while read f; do

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
    # Filter out some cases we know should be permitted now. 
    #
    case $f in
        *src/third_party_builtin/glew*/*/*)
            continue
            ;;
        *src/databases/Vis5D/*)
            continue
            ;;
        *src/common/utility/win32-regex*)
            continue
            ;;
        *src/svn_bin/build_visit) 
            continue
            ;;
        *src/aclocal.m4) 
            continue
            ;;
        *windowsbuild/include/qt/*) 
            continue
            ;;
        *src/svn_bin/hooks/check_gpl.sh) 
            continue
            ;;
        *docs/WebSite/new.html)
            continue
            ;;
    esac

    # check if the file we're trying to commit is empty (a deletion?) 
    commitFileLineCount=`${SVNLOOK} cat -t $TXN $REPOS $f | wc -l`
    if test $commitFileLineCount -le 0; then
        continue;
    fi

    #
    # If the file we're committing has GPL text, reject it.
    # We are looking for the phrase 'GNU General Public License'
    # which could exist on multiple lines. So, we look first for
    # 'GNU', take 3 lines +/- of line matching GNU, 'paste' these
    # lines together and then remove common, intervening symbols
    # that might appear between the lines and finally look for the
    # full phrase in the resulting catenation.
    #
    if test -n "`${SVNLOOK} cat -t $TXN $REPOS $f | grep -A 3 -B 3 GNU | tr '\n' ' ' | tr -s ' ' | tr -d '/*#' | grep GNU[[:space:]]*General[[:space:]]*Public[[:space:]]*License`"; then
        log "File \"$f\" appears to contain a 'GNU General Public License' statement. "
        log "If this is 'ok', then to allow it to be committed, you may need to alter "
        log "the logic in src/svn_bin/hooks/check_gpl.sh, commit that hook and then "
        log "try your commit again."
        exit 1
    fi

done < $FLIST

# all is well!
exit 0
