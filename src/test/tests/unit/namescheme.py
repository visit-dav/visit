# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  namescheme.py
#
#  Tests:      Namescheme_test unit test
#
#  Mark C. Miller, Tue Jan 11 10:19:23 PST 2011
#
#  Modifications:
#    Kathleen Biagas, Mon Oct 26, 2020
#    Retrieve correct path on Windows.
#
# ----------------------------------------------------------------------------
import sys

if sys.platform.startswith("win"):
    tapp = visit_bin_path("Namescheme_test.exe")
else:
    tapp = visit_bin_path("..","exe","Namescheme_test")
res = sexe(tapp,ret_output=True)
if res["return_code"] == 0:
    excode = 111
else:
    excode = 113
Exit(excode)
