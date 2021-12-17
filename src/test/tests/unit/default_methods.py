# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  default_methods.py
#
#  Tests:      Tests default method interface of the visit module
#
#  Cyrus Harrison, Mon Jul 20 11:32:42 PDT 2020
# ----------------------------------------------------------------------------
import sys

#
# Simply Call These
#
TestValueEQ("GetDebugLevel()", GetDebugLevel(), 0)

# Do something to create an error (not a python exception) so that we
# can get something other than the empty string from GetLastError()
DeleteWindow()
TestValueEQ("GetLastError()", GetLastError(), "Can't delete the last window.")

# This version number test requires always 3, single digits
TestValueGE("Version()", int(Version().replace('.','')), 321)

TestValueEQ("LocalNameSpace()", LocalNameSpace(), None)

# The long file name test is relevant only on Windows
if sys.platform.startswith("win"):
    TestValueEQ("LongFileName()", LongFileName("myfile.txt"))

Exit()
