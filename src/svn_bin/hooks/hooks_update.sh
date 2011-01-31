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
#
#   Mark C. Miller, Mon Jan 31 08:48:59 PST 2011
#   Prevent installing hooks from anything other than trunk commits.
##############################################################################

REPOS="$1"
REV=$2

hookCommonFile=""
preCommitFile=""
postCommitFile=""
hookFiles=""
files=`svnlook changed -r ${REV} ${REPOS} | awk '{print $2}'`
for f in ${files} ; do

    # This logic prevents any of the hooks_update magic acting on
    # hooks that are changed on something other than the trunk.
    if test "`echo $f | rev | cut -d'/' -f2-4 | rev`" = "src/svn_bin/hooks" \
         -a "`echo $f | cut -d'/' -f1-4`" != "trunk/src/svn_bin/hooks"; then
        log -e "\
In the file, \"$f\",\n\
you are committing a change to hooks on something other than the trunk.\n\
If this is the result of a merge of the trunk into a branch, then you\n\
may IGNORE this message. However, if this is the result of changes you\n\
have explicitly made to hooks on your branch, your changes to your hook\n\
have been COMMITTED only. They will NOT be INSTALLED and will, therefore,\n\
not effect behavior of any subsequent commit on ANY branch. All changes\n\
to hooks that you want INSTALLED must be done ONLY ON THE TRUNK. If you\n\
require hook behavior that is specific to your branch -- which can involve\n\
some tricky coding in the hooks -- you may want to have your changes reviewed\n\
by another developer before committing."
        continue
    fi

    case ${f} in
        trunk/src/svn_bin/hooks/hook_common.sh)
	    hookCommonFile=$f
            ;;
        trunk/src/svn_bin/hooks/pre-commit)
	    preCommitFile=$f
            ;;
        trunk/src/svn_bin/hooks/post-commit)
	    postCommitFile=$f
            ;;
        trunk/src/svn_bin/hooks/*)
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
