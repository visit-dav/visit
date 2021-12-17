# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  default_methods.py
#
#  Tests:      Tests default method interface of the visit module
#
#  Cyrus Harrison, Mon Jul 20 11:32:42 PDT 2020
#
#  Modifications
#    Mark C. Miller, Thu Dec 16 16:43:39 PST 2021
#    Wrap in TestValueXX calls so that we really do test behavior and not
#    just that something doesn't fail due to exception. Add tests for
#    WriteScript.
# ----------------------------------------------------------------------------
import os, sys, filecmp

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

# Test two ways of using WriteScript
f = open('foo.py', 'wt')
WriteScript(f)
f.close()
WriteScript('bar.py')
TestValueEQ("WriteScript('foo.py') == WriteScript(f)", filecmp.cmp('foo.py','bar.py'), True)
TestValueGT("WriteScript() file size", os.path.getsize('foo.py'), 3000)

Exit()
