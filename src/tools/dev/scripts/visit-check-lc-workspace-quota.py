#
# visit-check-lc-workspace-quota.py
#
# Shows inode (file count) quota info for /usr/workspace/wsa
#

import sys
import os
import datetime
import subprocess


def timestamp(t=None,sep="_"):
    """ Creates a timestamp that can easily be included in a filename. """
    if t is None:
        t = datetime.datetime.now()
    sargs = (t.year,t.month,t.day,t.hour,t.minute,t.second)
    sbase = "".join(["%04d",sep,"%02d",sep,"%02d",sep,"%02d",sep,"%02d",sep,"%02d"])
    return  sbase % sargs

def sexe(cmd,ret_output=False,echo = False):
    """ Helper for executing shell commands. """
    if echo:
        print("[exe: %s]" % cmd)
    if ret_output:
        p = subprocess.Popen(cmd,
                             shell=True,
                             universal_newlines=True,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.STDOUT)
        res =p.communicate()[0]
        return p.returncode,res
    else:
        return subprocess.call(cmd,shell=True)

rcode, rout = sexe("quota -v",ret_output=True)

quota_txt = ""
lines = rout.split("\n")
for idx,l in enumerate(lines):
    if l.startswith("/usr/workspace/wsa"):
        quota_txt = lines[idx+1]

if quota_txt != "":
    qtok = quota_txt.split()
    # usage is qtok[3]
    # limit is qtok[4]
    print("{0} {1} {2} of {3} files".format(timestamp(),"/usr/workspace/wsa",qtok[3],qtok[4]))
