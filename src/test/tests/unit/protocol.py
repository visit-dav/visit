# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  protocolo.py
#
#  Tests:      vistprotocol unit test
#
#  Mark C. Miller, Tue Jan 11 10:19:23 PST 2011
# ----------------------------------------------------------------------------
import os

tapp = visit_bin_path("visitprotocol")
res = sexe(tapp,ret_output=True)
if res["return_code"] == 0:
    excode = 111
else:
    excode = 113
Exit(excode)
