# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  namescheme.py
#
#  Tests:      Namescheme_test unit test
#
#  Mark C. Miller, Tue Jan 11 10:19:23 PST 2011
# ----------------------------------------------------------------------------
import os

tapp = visit_bin_path("..","exe","Namescheme_test")
rcode, outdata = sexe(tapp,ret_output=True)
if rcode == 0:
    excode = 111
else:
    excode = 113
Exit(excode)
