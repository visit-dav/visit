# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  stringhelpers.py
#
#  Tests:      StringHelpers_test unit test
#
#  Mark C. Miller, Tue Jan 11 10:19:23 PST 2011
#
#  Modifications:
#    Kathleen Biagas, Thu Nov 12, 2020
#    Retrieve the correct path on Windows.
# ----------------------------------------------------------------------------
import os, sys

if sys.platform.startswith("win"):
    tapp = visit_bin_path("StringHelpers_test.exe")
else:
    tapp = visit_bin_path("..","exe","StringHelpers_test")
res = sexe(tapp,ret_output=True)
if res["return_code"] == 0:
    excode = 111
else:
    excode = 113
Exit(excode)
