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

tapp = pjoin(TestEnv.params["visit_top_dir"],"src","exe","MRUCache_test")
rcode, outdata = sexe(tapp,ret_output=True)
TestText("mrucache", outdata)
Exit()
