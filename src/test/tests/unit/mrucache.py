# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  mrucache.py
#
#  Tests:      MRUCache_test unit test
#
#  Mark C. Miller, Tue Jan 11 10:19:23 PST 2011
#
#  Modifications:
#    Kathleen Biagas, Mon Oct 26, 2020
#    Retrieve the correct path on Windows.
#
# ----------------------------------------------------------------------------
import sys

if sys.platform.startswith("win"):
    tapp = visit_bin_path("MRUCache_test.exe")
else:
    tapp = visit_bin_path("..","exe","MRUCache_test")
res = sexe(tapp,ret_output=True)
TestText("mrucache", res["output"])
Exit()
