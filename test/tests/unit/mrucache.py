# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  mrucache.py
#
#  Tests:      MRUCache_test unit test
#
#  Mark C. Miller, Tue Jan 11 10:19:23 PST 2011
# ----------------------------------------------------------------------------
import os

tapp = visit_bin_path("..","exe","MRUCache_test")
res = sexe(tapp,ret_output=True)
TestText("mrucache", res["output"])
Exit()
