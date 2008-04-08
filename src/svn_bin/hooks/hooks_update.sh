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

hooksUpdateFile=""
hookVarsFile=""
preCommitFile=""
postCommitFile=""
hookFiles=""
files=`${SVNLOOK} changed -r ${REV} ${REPOS} | ${AWK} '{print $2}'`
for f in ${files} ; do
echo $f >> foobar
    case ${f} in
        *src/svn_bin/hooks/hooks_update.sh)
	    hooksUpdateFile=$f
            ;;
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
# Handle the hook variables file speciall. We can't install it
# automatically as it is the current script that is installing
# everything else.
if test -n "$hooksUpdateFile"; then
    echo "You are updating hooks_update.sh." 1>&2
    echo "It cannot be installed automatically." 1>&2
    echo "Remember to install hooks_update.sh manually." 1>&2
fi

#
# Handle the hook variables file specially. It can effect everything
# else, so do it first.
#
if test -n "$hookVarsFile"; then
    ${SVNLOOK} cat -t $TXN $REPOS $hookVarsFile > $REPOS/hooks/hook_vars.sh
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
	    error "Committed a hook script without also updating pre- or post-commit meta scripts"
	fi
    fi

    ${SVNLOOK} cat -t $TXN $REPOS $f > $REPOS/hooks/$bf
    ${CHGRP} $VISIT_GROUP_NAME $REPOS/hooks/$bf
    ${CHMOD} 770 $REPOS/hooks/$bf
done

# all is well!
exit 0
