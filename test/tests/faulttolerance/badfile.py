# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Tests: The quality of the error message being produced when opening a bad
#         file.
#
#  Programmer: Hank Childs
#  Date:       January 12, 2007
#
# ----------------------------------------------------------------------------

import os

# Make zero-length Silo file.  The Silo reader is smart enough to not
# crash, which means we can test the normal error message.
f = open("junk.silo", "w")
f.close()
OpenDatabase("junk.silo")
e = GetLastError()
TestText("badfile_01", e)

# Make zero-length VTK file.  The VTK reader crashes in this case.  If the
# VTK reader is ever improved, then we'll have to locate a reader that
# crashes successfully.  (Maybe we'll write one.)
f = open("junk.vtk", "w")
f.close()
OpenDatabase("junk.vtk")
e = GetLastError()
TestText("badfile_02", e)

Exit()
