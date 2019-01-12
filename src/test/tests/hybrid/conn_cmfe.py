# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  conn_cmfe.py
#
#  Defect ID:  '6616
#
#  Programmer: Hank Childs
#  Date:       August 30, 2005
#
#  Modifications:
#
#    Hank Childs, Tue Sep 20 13:15:11 PDT 2005
#    Test for non-existent databases, especially those with short names ['6616]
#
#    Hank Childs, Fri Oct  7 17:14:07 PDT 2005
#    Test error message for Eulerian material selection.
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
#
#    Cyrus Harrison, Thu Mar 25 09:57:34 PDT 2010
#    Added call(s) to DrawPlots() b/c of changes to the default plot state
#    behavior when an operator is added.
#
#    Cyrus Harrison, Wed Aug 25 14:25:13 PDT 2010
#    SIL id shift (from 1 to 2) due to changes in SIL generation.
#
# ----------------------------------------------------------------------------


OpenDatabase(silo_data_path("wave0200.silo"))



# Test general capability.
DefineScalarExpression("cmfe", "conn_cmfe(<%s:pressure>, quadmesh)" % cmfe_silo_data_path("wave0020.silo"))
AddPlot("Pseudocolor", "cmfe")
DrawPlots()
Test("conn_cmfe_01")

# Test using the expression as a secondary variable.
ChangeActivePlotsVar("pressure")
AddOperator("Threshold")
t = ThresholdAttributes()
t.lowerBounds = (-1.0)
t.upperBounds = (0.1)
t.listedVarNames = ("cmfe")
SetOperatorOptions(t)
DrawPlots()
Test("conn_cmfe_02")

# Test using cmfe in another expression.
DefineVectorExpression("disp", "{0, 3*cmfe, 0}")
RemoveLastOperator()
d = DisplaceAttributes()
d.variable = "disp"
SetDefaultOperatorOptions(d)
AddOperator("Displace")
DrawPlots()
Test("conn_cmfe_03")

RemoveLastOperator()
ChangeActivePlotsVar("cmfe")
DrawPlots()

Query("MinMax")
t = GetQueryOutputString()
TestText("conn_cmfe_04", t)

# Now test error conditions.
DefineScalarExpression("cmfe2", "conn_cmfe(pressure, quadmesh)")
ChangeActivePlotsVar("cmfe2")
DrawPlots()
t = GetLastError()
TestText("conn_cmfe_05", t)

DefineScalarExpression("cmfe3", "conn_cmfe(<bad_file.silo:a_var>, quadmesh)")
ChangeActivePlotsVar("cmfe3")
DrawPlots()
t = GetLastError()
TestText("conn_cmfe_06", t)

DefineScalarExpression("cmfe4", "conn_cmfe(<%s:pressure>, quadmesh, pressure)" %  cmfe_silo_data_path("wave0100.silo"))
ChangeActivePlotsVar("cmfe4")
DrawPlots()
t = GetLastError()
TestText("conn_cmfe_07", t)

DefineScalarExpression("cmfe5", "conn_cmfe(<%s:pressure>)" % cmfe_silo_data_path("wave0100.silo"))
ChangeActivePlotsVar("cmfe5")
DrawPlots()
t = GetLastError()
TestText("conn_cmfe_08", t)

# And one more cool picture just for grins.
DefineScalarExpression("cmfe6", "conn_cmfe(<%s:pressure>, quadmesh)" % cmfe_silo_data_path("wave0570.silo"))
DefineScalarExpression("max", "if(ge(pressure, cmfe6), pressure, cmfe6)")
ChangeActivePlotsVar("max")
DrawPlots()
Test("conn_cmfe_09")

# Invalid variable in new database.
DefineScalarExpression("cmfe7", "conn_cmfe(<%s:pressure>, quadmesh)" % cmfe_silo_data_path("globe.silo"))
ChangeActivePlotsVar("cmfe7")
t = GetLastError()
TestText("conn_cmfe_10", t)

# Now a good variable, but connectivity doesn't match.
DefineScalarExpression("cmfe8", "conn_cmfe(<%s:t>, quadmesh)" % cmfe_silo_data_path("globe.silo"))
ChangeActivePlotsVar("cmfe8")
DrawPlots()
t = GetLastError()
TestText("conn_cmfe_11", t)

# Test multiple CMFEs
DefineScalarExpression("cmfe9", "conn_cmfe(<%s:pressure>, quadmesh)" % cmfe_silo_data_path("wave0100.silo"))
DefineScalarExpression("max2", "if(ge(max, cmfe9), max, cmfe9)")
ChangeActivePlotsVar("max2")
DrawPlots()
Test("conn_cmfe_12")

DefineScalarExpression("cmfe10", "conn_cmfe(<30:d>, quadmesh)")
ChangeActivePlotsVar("cmfe10")
t = GetLastError()
TestText("conn_cmfe_13", t)

ChangeActivePlotsVar("cmfe")
DrawPlots()
s = SILRestriction()
s.TurnOffAll()
s.TurnOnSet(2)
SetPlotSILRestriction(s)
t = GetLastError()
TestText("conn_cmfe_14", t)

Exit()
