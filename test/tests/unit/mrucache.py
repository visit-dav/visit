# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  mrucache.py
#
#  Tests:      MRUCache_test unit test
#
#  Mark C. Miller, Tue Jan 11 10:19:23 PST 2011
# ----------------------------------------------------------------------------
import os, subprocess

# TODO: NEEDS TO BE FIXED FOR OUT OF SOURCE BUILD

tapp = pjoin(visitTopDir,"src","exe","MRUCache_test")
subp = subprocess.Popen(tapp, stderr=subprocess.PIPE, stdout=subprocess.PIPE)
(outdata,errdata) = subp.communicate()
subp.wait()
TestText("mrucache", outdata)
Exit()
