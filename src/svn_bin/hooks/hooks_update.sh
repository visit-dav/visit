#!/bin/sh
##############################################################################
#
# Purpose: A post-commit hook to automatically 'install' an update to a hook
#          script.
#
# Programmer: Mark C. Miller
# Created:    Mon Apr  7 18:16:51 PDT 2008
#
# Modifications:
#
#   Mark C. Miller, Tue Dec  9 23:13:19 PST 2008
#   Changed name of hook_vars file to hook_common as it includes more than
#   variable definitions now. Replaced refs to commands via ${<VARNAME>}
#   method to just the commands themselves.
#
#   Mark C. Miller, Fri Dec  4 09:33:50 PST 2009
#   Modified chmod command to make user/group read,write,eXecute and other
#   read,eXecute.
#
#   Mark C. Miller, Fri Dec  4 09:37:39 PST 2009
#   Fixed typo in chmod command
##############################################################################

REPOS="$1"
REV=$2

hookCommonFile=""
preCommitFile=""
postCommitFile=""
hookFiles=""
files=`svnlook changed -r ${REV} ${REPOS} | awk '{print $2}'`
for f in ${files} ; do
    case ${f} in
        *src/svn_bin/hooks/hook_common.sh)
	    hookCommonFile=$f
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
# Handle the hook_common file specially. It can effect everything
# else, so do it first.
#
if test -n "$hookCommonFile"; then
    log "Installing updated hook_common file"
    svnlook cat -r $REV $REPOS $hookCommonFile > $REPOS/hooks/hook_common.sh
    chgrp $VISIT_GROUP_NAME $REPOS/hooks/hook_common.sh
    chmod u+rwX,g+rwX,o+rX $REPOS/hooks/hook_common.sh
fi

#
# Re-Install any committed (or UN-install any removed) hooks 
#
for f in $preCommitFile $postCommitFile ${hookFiles} ; do
    bf=`basename $f`

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
    svnlook cat -r $REV $REPOS $f > $REPOS/hooks/$bf

    #
    # If the file exists and is non-zero size, it has been added/modified
    # in the repo. Furthermore, the above cat command installed it. So,
    # just ensure permissions and group access are set correctly.
    # Otherwise, it has been deleted. So, un-install it by removing it.
    #
    if test -s $REPOS/hooks/$bf; then
        log "Installing hook script $bf to $REPOS/hooks/$bf"
        chgrp $VISIT_GROUP_NAME $REPOS/hooks/$bf 1>/dev/null 2>&1
        chmod u+rwX,g+rwX,o+rX $REPOS/hooks/$bf 1>/dev/null 2>&1
    else
	log "UN-installing hook script $bf from $REPOS/hooks/$bf"
        rm -f $REPOS/hooks/$bf 1>/dev/null 2>&1
    fi

done

exit $hadError 
