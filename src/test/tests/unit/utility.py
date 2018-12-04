# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  utility.py
#
#  Tests:      Utility_test unit test
#
#  Mark C. Miller, Mon Dec  1 23:55:05 PST 2014
# ----------------------------------------------------------------------------
import os

tapp = visit_bin_path("..","exe","Utility_test")
res = sexe(tapp,ret_output=True)
if res["return_code"] == 0:
    excode = 111
else:
    excode = 113
Exit(excode)
