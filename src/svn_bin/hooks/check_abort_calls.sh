#!/bin/sh
##############################################################################
#
# Purpose: Check for existence of abort and the like in source files
#
# Programmer: Mark C. Miller
# Created:    Wed Jun 22 09:57:42 PDT 2011
#
# Modifications:
#   Mark C. Miller, Wed Jun 22 17:20:09 PDT 2011
#   Check -DPARALLEL code too
#  
#   Kathleen Biagas, Fri Oct 21 18:03:33 PDT 2011
#   Skip windowsbuild directory.
#
#   Eric Brugger, Tue Mar  6 07:57:28 PST 2012
#   Skip the data directory.
#
#   Satheesh Maheswaran, Wed May 15 15:19:21 BST 2013
#   Skip customlauncher files
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
    # Note to developers: Before adding skip cases to this case statement,
    # which have the effect of eliminating the whole file from consideration,
    # consider qualifying calls my modifying the source code with
    # '// HOOKS_IGNORE' comment immediately following a call you think should
    # be permitted. The grep, below, eliminates those cases from consideration.
    #
    case $fname in
        */data/*|\
        */src/tools/*|\
        */src/cqscore/*|\
        */src/third_party_builtin/*|\
        */src/bin/internallauncher|\
        */src/resources/hosts/*/customlauncher|\
        */src/bin/*.py|\
        */src/svn_bin/*|\
        */windowsbuild/*)
            # Skip these files and directories
            continue
            ;;
        branches/*RC/*|\
        trunk/*)
            # Do not skip (continue) these
            ;;
        *)
            # Skip anything not on trunk
            continue
            ;;
    esac

    svnlook cat -t $TXN $REPOS $fname | grep -v '^#[[:space:]]*include' | grep -v HOOKS_IGNORE | cpp - 2>&1 | grep -q '[[:space:]]\(abort\|exit\)[[:space:]]*(' 1>/dev/null 2>&1
    commitFileAbortCalls=$?
    if test $commitFileAbortCalls -ne 0; then
        svnlook cat -t $TXN $REPOS $fname | grep -v '^#[[:space:]]*include' | grep -v HOOKS_IGNORE | cpp -DPARALLEL - 2>&1 | grep -q '[[:space:]]\(abort\|exit\)[[:space:]]*(' 1>/dev/null 2>&1
        commitFileAbortCalls=$?
    fi

    # If the file we're committing has #warnings, reject it
    if test $commitFileAbortCalls -eq 0; then
        log "File \"$fname\" appears to contain abort()/exit() calls"
        log "    outside of a conditional #ifdef DEBUG block."
        log "Please remove them before committing."
        exit 1
    fi

done < $FLIST

# all is well!
exit 0
