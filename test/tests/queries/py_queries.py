# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  py_queires.py
#
#  Tests:      python filters / python queries
#              plots     - Pseudocolor
#
#
#  Programmer: Cyrus Harrison
#  Date:       Fri May 21 09:14:07 PDT 2010
#
#  Modifications:
#
# ----------------------------------------------------------------------------

import os
from os.path import join as pjoin

OpenDatabase("../data/silo_%s_test_data/rect2d.silo"%SILO_MODE)

AddPlot("Pseudocolor", "d")
DrawPlots()

script_file = pjoin(os.path.split(TestScriptPath())[0],"py_query_script_00.vpq")

PythonQuery(file=script_file)
res = GetQueryOutputString()
TestText("py_queries_00",res + "\n")

Exit()
