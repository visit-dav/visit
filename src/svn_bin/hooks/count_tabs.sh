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
#   Hank Childs, Wed Sep 10 15:34:53 PDT 2008
#   Allow files named "Makefile" to have tabs.
#
#   Mark C. Miller, Tue Dec  9 00:19:04 PST 2008
#   Obtain list of changed files via FLIST ($3) argument and loop
#   over them via 'read' sh builtin method.
#
#   Mark C. Miller, Tue Dec  9 23:09:46 PST 2008
#   Eliminated several svnlook cat commands and improved performance and
#   logic in counting tabs in files. Re-factored a lot skipping logic to
#   hook_common.sh
#
#   Cyrus Harrison, Tue Dec 16 14:42:52 PST 2008
#   Added *.cmake files to the list of files we permit tabs in.
#
##############################################################################
REPOS="$1"
TXN="$2"
FLIST="$3"

while read fline; do

    #
    # Get file 'svnlook' status and name
    #
    fstat=`echo $fline | tr -s ' ' | cut -d' ' -f1`
    fname=`echo $fline | tr -s ' ' | cut -d' ' -f2`

    #
    # Skip common cases of deletions, dirs, non-text files
    #
    if `HandleCommonSkipCases $fstat $fname`; then
        continue
    fi

    #
    # Filter out other cases HandleCommonSkipCases doesn't catch
    #
    case $fname in
        *.in|*.html|*/third_party_builtin/*|*/common/icons/*|*.vcproj|*.sln|*.cmake)
            continue
            ;;
        *.tcl)
            continue
            ;;
        */src/configure)
            continue
            ;;
        */svn_bin/build_visit)
            continue
            ;;
        *Makefile)
            continue
            ;;
    esac

    # Count number of tabs in file we're committing.
    # The -c -d flags on tr here cause deletion of every character
    # that is NOT a tab. What is left over is tab characters and we
    # simply count them with wc.
    commitFileTabCount=`svnlook cat -t $TXN $REPOS $fname | tr -c -d '\t' | wc -m`

    #
    # If the file we're committing has tabs, we have some additional
    # work to do to ensure we are NOT adding tabs.
    #
    if test $commitFileTabCount -gt 0; then

        #
        # If this file is being added, it is a new file. So, the existence
        # of ANY tabs is an error.
        #
        if test $fstat = A; then
            log "A file you are adding, \"$fname\", has tabs"
            exit 1
        fi

        #
        # Since the file is NOT being added, just check to make sure the
        # number of tabs in the file as it already exists in the repo is
        # not less than the number of tabs in the file being committed.
        #
        repoFileTabCount=`svnlook cat $REPOS $fname | tr -c -d '\t' | wc -m`
        if test $commitFileTabCount -gt $repoFileTabCount; then
            log "In a file you are commiting, \"$fname\", you have increased "
            log "the number of tabs from $repoFileTabCount to $commitFileTabCount."
            exit 1
        fi

    fi # if test $commitFileTabCount -gt 0

done < $FLIST

# all is well!
exit 0
