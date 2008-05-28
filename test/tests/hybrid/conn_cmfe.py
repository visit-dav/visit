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
# ----------------------------------------------------------------------------


OpenDatabase("../data/wave0200.silo")


# Test general capability.
DefineScalarExpression("cmfe", "conn_cmfe(<../data/wave0020.silo:pressure>, quadmesh)")
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
Test("conn_cmfe_02")

# Test using cmfe in another expression.
DefineVectorExpression("disp", "{0, 3*cmfe, 0}")
RemoveLastOperator()
d = DisplaceAttributes()
d.variable = "disp"
SetDefaultOperatorOptions(d)
AddOperator("Displace")
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

DefineScalarExpression("cmfe4", "conn_cmfe(<../data/wave0100.silo:pressure>, quadmesh, pressure)")
ChangeActivePlotsVar("cmfe4")
DrawPlots()
t = GetLastError()
TestText("conn_cmfe_07", t)

DefineScalarExpression("cmfe5", "conn_cmfe(<../data/wave0100.silo:pressure>)")
ChangeActivePlotsVar("cmfe5")
DrawPlots()
t = GetLastError()
TestText("conn_cmfe_08", t)

# And one more cool picture just for grins.
DefineScalarExpression("cmfe6", "conn_cmfe(<../data/wave0570.silo:pressure>, quadmesh)")
DefineScalarExpression("max", "if(ge(pressure, cmfe6), pressure, cmfe6)")
ChangeActivePlotsVar("max")
DrawPlots()
Test("conn_cmfe_09")

# Invalid variable in new database.
DefineScalarExpression("cmfe7", "conn_cmfe(<../data/globe.silo:pressure>, quadmesh)")
ChangeActivePlotsVar("cmfe7")
t = GetLastError()
TestText("conn_cmfe_10", t)

# Now a good variable, but connectivity doesn't match.
DefineScalarExpression("cmfe8", "conn_cmfe(<../data/globe.silo:t>, quadmesh)")
ChangeActivePlotsVar("cmfe8")
DrawPlots()
t = GetLastError()
TestText("conn_cmfe_11", t)

# Test multiple CMFEs
DefineScalarExpression("cmfe9", "conn_cmfe(<../data/wave0100.silo:pressure>, quadmesh)")
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
s.TurnOnSet(1)
SetPlotSILRestriction(s)
t = GetLastError()
TestText("conn_cmfe_14", t)

Exit()
