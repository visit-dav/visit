# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  ghost_zoneid_expr.py
#
#  Tests:      Ghost Zone Id Expression
#
#  Programmer: Justin Privitera
#  Date:       Wed Apr 6 15:03:47 PDT 2022
#
# ----------------------------------------------------------------------------
OpenDatabase(silo_data_path("multi_ucd3d.silo"))
meshName = "mesh1"

#
# Set precision for rounding operations
#
Prec = 5

TestSection("Ghost Zone ID Expression")

AddPlot("Pseudocolor", "d")
DefineScalarExpression("ghostzonevar", "ghost_zoneid(" + meshName + ")")
AddPlot("Pseudocolor", "ghostzonevar")
DrawPlots()
Query("MinMax")
q = GetQueryOutputObject()
TestValueEQ("Minimum Field Value", q['min'], 0.0, Prec)
TestValueEQ("Maximum Field Value", q['max'], 0.0, Prec)
DeleteAllPlots()

TestSection("Ghost Zone ID Expression w/ Inverse Ghost Zones Debugging")

AddPlot("Pseudocolor", "d")
AddOperator("InverseGhostZone", 1)
DefineScalarExpression("ghostzonevar", "ghost_zoneid(" + meshName + ")")
ChangeActivePlotsVar("ghostzonevar")
DrawPlots()
Query("MinMax")
q = GetQueryOutputObject()
TestValueEQ("Minimum Field Value", q['min'], 1.0, Prec)
TestValueEQ("Maximum Field Value", q['max'], 1.0, Prec)
DeleteAllPlots()

Exit()
