#!/bin/sh
#####
# This is a subversion hook script meant to notice when a user changes a state
# object, and warn them to update the corresponding objects.  It takes two
# arguments: the path to the repository, and a transaction identifier -- in
# that order.
# The script is intended to be installed as `post-commit' hook.  In that
# situation, it will print a message to the user's terminal (if appropriate).
# If you install it as a `pre-commit' hook, it would abort the transaction.
#####
SVNLOOK=/usr/bin/svnlook
AWK=/usr/bin/awk
RM=/bin/rm
BASH=/bin/bash

###########################################################
# You should not need to modify anything below this line. #
###########################################################
REPOS=$1
REV=$2

# Error.  Really an error is the same as a message (since this is post-commit,
# we really can't give an error), but this documents it more clearly.
function error()
{
    echo $@ 1>&2
    exit 1
}

if test "x${REPOS}" = "x"; then
    error "Repository path not set in $0."
fi
if test "x${REV}" = "x"; then
    error "Revision number not set in $0."
fi

files=`${SVNLOOK} changed -r ${REV} ${REPOS} | ${AWK} '{print $2}'`
state_files=""
for fn in ${files} ; do
    case ${fn} in
        *common/state/*xml)
            state_files="${state_files} ${fn}"
            ;;
    esac
done
if test -n "${state_files}" ; then
    echo "You are changing the files:" 1>&2
    echo "${state_files}" 1>&2
    echo "" 1>&2
    echo "Did you remember to update the Python and Java bindings?" 1>&2
    exit 1
fi
exit 0
