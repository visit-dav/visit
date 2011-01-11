# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  stringhelpers.py
#
#  Tests:      StringHelpers_test unit test
#
#  Mark C. Miller, Tue Jan 11 10:19:23 PST 2011
# ----------------------------------------------------------------------------
import os, subprocess

tapp = os.path.join(visitTopDir,"src","exe","StringHelpers_test")
subp = subprocess.Popen(tapp, stderr=subprocess.PIPE, stdout=subprocess.PIPE)
subp.wait()
if subp.returncode == 0:
    excode = 111
else:
    excode = 113
Exit(excode)
