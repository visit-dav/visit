#!/bin/sh
##############################################################################
#
# Purpose: Automatically 'install' an update to a hook script 
#
# Programmer: Mark C. Miller
# Created:    Mon Apr  7 18:16:51 PDT 2008
#
##############################################################################

REPOS="$1"
REV=$2

function error()
{
    echo $@ 1>&2
    exit 1
}

if test -z "${REPOS}"; then
    error "Repository path not set in $0."
fi
if test -z "${REV}"; then
    error "Revision number not set in $0."
fi

hookVarsFile=""
preCommitFile=""
postCommitFile=""
hookFiles=""
files=`${SVNLOOK} changed -r ${REV} ${REPOS} | ${AWK} '{print $2}'`
for f in ${files} ; do
    case ${f} in
        *src/svn_bin/hooks/hook_vars.sh)
	    hookVarsFile=$f
            ;;
        *src/svn_bin/hooks/pre-commit)
	    preCommitFile=$f
            ;;
        *src/svn_bin/hooks/post-commit)
	    postCommitFile=$f
            ;;
        *src/svn_bin/hooks/*)
            hookFiles="$hookFiles $f"
            ;;
    esac
done

#
# Turn off noclobber if it is on
#
set +o noclobber

#
# Handle the hook variables file specially. It can effect everything
# else, so do it first.
#
if test -n "$hookVarsFile"; then
    ${SVNLOOK} cat -r $REV $REPOS $hookVarsFile > $REPOS/hooks/hook_vars.sh
    ${CHGRP} $VISIT_GROUP_NAME $REPOS/hooks/hook_vars.sh
    ${CHMOD} 770 $REPOS/hooks/hook_vars.sh
fi

#
# Re-Install any committed hooks 
#
for f in $preCommitFile $postCommitFile ${hookFiles} ; do
    bf=`${BASENAME} $f`

    #
    # If we don't already have this hook installed, make sure
    # the user knows to update the pre- and/or post-commit meta scripts
    #
    if test ! -e $REPOS/hooks/$bf; then
        if test -z "$preCommitFile" -a -z "$postCommitFile"; then
	    echo "Committed a hook script without also updating pre- or post-commit meta scripts" 1>&2
	fi
    fi

    ${SH} ${SVNLOOK} cat -r $REV $REPOS $f > $REPOS/hooks/$bf
    ${CHGRP} $VISIT_GROUP_NAME $REPOS/hooks/$bf
    ${CHMOD} 770 $REPOS/hooks/$bf
done

# all is well!
exit 0
