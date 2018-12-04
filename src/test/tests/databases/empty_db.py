# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  empty_db.py 
#
#  Programmer: Mark C. Miller 
#  Date:       28Oct10
#
#  Modifications:
#    Mark C. Miller, Mon Nov  1 12:24:23 PDT 2010
#    I added specification of the Silo format to the open call so that we
#    can be assured of having the real exception (DBYieldedNoData) returned
#    in the error message. Otherwise, that exception is caught and then folded
#    into whatever other possible exceptions other candidates might generate.
# ----------------------------------------------------------------------------
import re

OpenDatabase(data_path("silo_pdb_test_data/empty.silo"), 0, "Silo_1.0")

errStr = GetLastError()
tmpType = re.search(",\nno data was found in the file for VisIt to work with.", errStr)
msg = errStr
if tmpType != None:
    msg = "DBYieldedNoDataException\n"
TestText("empty_01", msg)

Exit()
