#!/bin/sh
##############################################################################
#
# Purpose: Automatically 'install' an update to a hook script 
#
# Programmer: Mark C. Miller
# Created:    Mon Apr  7 18:16:51 PDT 2008
#
##############################################################################

SVNLOOK="/usr/bin/svnlook"
AWK="/usr/bin/awk"

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
hookVarsFile=""
preCommitFile=""
postCommitFile=""
hookFiles=""
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
        *src/svn_bin/hooks)
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
# else.
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
    bf=`basename $f`

    #
    # If we don't already have this hook installed, make sure
    # the user knows to update the pre- and/or post-commit meta scripts
    #
    if test ! -e $REPOS/hooks/$bf; then
        if test -z "$preCommitFile" -a -z "$postCommitFile"; then
	    log "Committed a hook script without also updating pre- or post-commit meta scripts"
	    exit 1
	fi
    fi

    ${SVNLOOK} cat -t $TXN $REPOS $f > $REPOS/hooks/$bf
    ${CHGRP} $VISIT_GROUP_NAME $REPOS/hooks/$bf
    ${CHMOD} 770 $REPOS/hooks/$bf
done

# all is well!
exit 0
