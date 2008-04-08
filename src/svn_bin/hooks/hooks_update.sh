#!/bin/sh
##############################################################################
#
# Purpose: A post-commit hook to automatically 'install' an update to a hook
#          script.
#
# Programmer: Mark C. Miller
# Created:    Mon Apr  7 18:16:51 PDT 2008
#
##############################################################################

REPOS="$1"
REV=$2

hadError=0
function log()
{
    hadError=1
    echo $@ 1>&2
}

if test -z "${REPOS}"; then
    log "Repository path not set in $0."
    exit 1
fi
if test -z "${REV}"; then
    log "Revision number not set in $0."
    exit 1
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
    log "Installing updated hook variables"
    ${SVNLOOK} cat -r $REV $REPOS $hookVarsFile > $REPOS/hooks/hook_vars.sh
    ${CHGRP} $VISIT_GROUP_NAME $REPOS/hooks/hook_vars.sh
    ${CHMOD} 770 $REPOS/hooks/hook_vars.sh
fi

#
# Re-Install any committed (or UN-install any removed) hooks 
#
for f in $preCommitFile $postCommitFile ${hookFiles} ; do
    bf=`${BASENAME} $f`

    #
    # If we don't already have this hook installed, make sure
    # the user knows to update the pre- and/or post-commit meta scripts
    #
    if test ! -e $REPOS/hooks/$bf; then
        if test -z "$preCommitFile" -a -z "$postCommitFile"; then
	    log "Committed a hook script without also updating pre- or post-commit meta scripts" 1>&2
	fi
    fi

    #
    # Install the file (or at least try to)
    #
    ${SVNLOOK} cat -r $REV $REPOS $f > $REPOS/hooks/$bf

    #
    # If the file exists and is non-zero size, it has been added/modified
    # in the repo. Furthermore, the above cat command installed it. So,
    # just ensure permissions and group access are set correctly.
    # Otherwise, it has been deleted. So, un-install it by removing it.
    #
    if test -s $REPOS/hooks/$bf; then
        log "Installing hook script $bf to $REPOS/hooks/$bf"
        ${CHGRP} $VISIT_GROUP_NAME $REPOS/hooks/$bf 1>/dev/null 2>&1
        ${CHMOD} 770 $REPOS/hooks/$bf 1>/dev/null 2>&1
    else
	log "UN-installing hook script $bf from $REPOS/hooks/$bf"
        ${RM} -f $REPOS/hooks/$bf 1>/dev/null 2>&1
    fi

done

exit $hadError 
