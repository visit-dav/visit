# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  empty_db.py 
#
#  Programmer: Mark C. Miller 
#  Date:       28Oct10
#
# ----------------------------------------------------------------------------
import re

OpenDatabase("../data/silo_pdb_test_data/empty.silo")
errStr = GetLastError()
tmpType = re.search(",\nno data was found in the file for VisIt to work with.", errStr)
msg = "UnknownException\n"
if tmpType != None:
    msg = "DBYieldedNoDataException\n"
TestText("empty_01", msg)

Exit()
