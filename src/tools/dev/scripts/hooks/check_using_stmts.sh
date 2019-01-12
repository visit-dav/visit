#!/bin/sh
##############################################################################
#
# Purpose: Check for existence of using statements in header files
#
# Programmer: Mark C. Miller
# Created:    Mon Jul 11 23:01:54 PDT 2011
#
# Modifications:
#   Kathleen Biagas, Mon Nov 21 15:53:08 PST 2011
#   Skip everything in windowsbuild directory.
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

    case $fname in
        trunk/src/third_party_builtin/*|branches/*/src/third_party_builtin/*)
            continue
            ;;
        */common/utility/visitstream.h)
            continue
            ;;
        */databases/paraDIS/RC_cpp_lib/*.h|*/databases/paraDIS_tecplot/RC_cpp_lib/*.h)
            continue
            ;;
        */windowsbuild/*)
            continue
            ;;
        trunk/src/*.h|branches/*/src/*.h)
            # Do not skip (continue) these
            ;;
        *)
            continue
            ;;
    esac

    svnlook cat -t $TXN $REPOS $fname | grep -q '^\( *using *std::\)\|\( *using *namespace\)' 1>/dev/null 2>&1
    commitHeaderFileHasUsingStatements=$?

    # If the file we're committing has #warnings, reject it
    if test $commitHeaderFileHasUsingStatements -eq 0; then
            log -e "\
The header file, \"$fname\",\n\
appears to contain C++ 'using' directives. Using directives should be avoided\n\
in header files as they effect all files in which the header file is included,\n\
directly or indirectly. Please remove 'using' directives from header files.\n\
To do this, you will likely wind up having to add additional scope resolution\n\
to all non-builtin type names. For example, for all STL classes, you will have\n\
to add 'std::'."
        exit 1
    fi

done < $FLIST

# all is well!
exit 0
