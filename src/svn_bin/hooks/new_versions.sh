#!/bin/sh
#####
# This is NOT a subversion hook script!
#####
# svn.nersc.gov defaults to giving us vrsh with a useless PATH setting;
# furthermore, no program which tries to utilize stdin is runnable (they hang
# forever).  This makes it a real pain to play with hooks.
#
# To make things more manageable, one can edit hooks locally and then scp them
# to NERSC.  I tend to do this to my ${HOME}, and then from a shell logged into
# NERSC I copy them to the hooks directory.  When working this way, the
# permissions can sometimes get mucked up -- a pain to fix when you don't have
# a real shell.  This script ensures files get copied and work appropriately.
#####

HOOKSDIR="/svn/visit/hooks"

files="verify_versioning.sh"
files="${files} pre-commit"
files="${files} post-commit"
files="${files} exceptions.py"
files="${files} output_email.sh"
files="${files} state_update.sh"
for f in ${files} ; do
        if test -f ${f} ; then
            /bin/echo "copying/fixing ${f}..."
            /bin/cp ${HOME}/${f} ${HOOKSDIR}/
            /bin/chmod g+rwx ./${f}
            /bin/chmod a+rx ./${f}
            /bin/chown fogal1:visitdev ${f}
        fi
done
