# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  protocolo.py
#
#  Tests:      vistprotocol unit test
#
#  Mark C. Miller, Tue Jan 11 10:19:23 PST 2011
# ----------------------------------------------------------------------------
import os, subprocess

tapp = os.path.join(visitTopDir,"src","bin","visitprotocol")
subp = subprocess.Popen(tapp, stderr=subprocess.PIPE, stdout=subprocess.PIPE)
(outdata,errdata) = subp.communicate()
subp.wait()
TestText("protocol", outdata)
Exit()
