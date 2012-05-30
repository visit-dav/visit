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

def script_path(sname):
    return pjoin(os.path.split(TestScriptPath())[0],sname)

OpenDatabase(silo_data_path("rect2d.silo"))


AddPlot("Pseudocolor", "d")
DrawPlots()


PythonQuery(file=script_path("py_query_script_00.vpq"))
res = GetQueryOutputString()
TestText("py_queries_00",res + "\n")

script_file = pjoin(os.path.split(TestScriptPath())[0],"py_query_script_01.vpq")

PythonQuery(file=script_path("py_query_script_01.vpq"),vars=["default","p"],args=["test",0.0,[1,2.0,3]])
res = GetQueryOutputString()
TestText("py_queries_01",res + "\n")


Exit()
