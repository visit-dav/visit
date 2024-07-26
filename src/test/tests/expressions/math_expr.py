# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  math_expr.py
#
#  Tests:      Basic math expressions
#
#  Mark C. Miller, Fri Jul 26 14:13:43 PDT 2024
# ----------------------------------------------------------------------------
OpenDatabase(silo_data_path("globe.silo"))
meshName = "mesh1"

# Test simple integer valued case first
DefineScalarExpression("const31", "nodal_constant(<" + meshName + ">, 31)")
DefineScalarExpression("const5", "nodal_constant(<" + meshName + ">, 5)")
DefineScalarExpression("mod1", "const31 % const5")
AddPlot("Pseudocolor", "mod1")
DrawPlots()
Query("MinMax")
q = GetQueryOutputObject()
TestValueEQ("Minimum Field Value", q['min'], 1.0)
TestValueEQ("Maximum Field Value", q['max'], 1.0)
DeleteAllPlots()

# Test floating point case (zero remainder)
DefineScalarExpression("const7.5", "nodal_constant(<" + meshName + ">, 7.5)")
DefineScalarExpression("const2.5", "nodal_constant(<" + meshName + ">, 2.5)")
DefineScalarExpression("mod2", "const7.5 % const2.5")
AddPlot("Pseudocolor", "mod2")
DrawPlots()
Query("MinMax")
q = GetQueryOutputObject()
TestValueEQ("Minimum Field Value", q['min'], 0.0)
TestValueEQ("Maximum Field Value", q['max'], 0.0)
DeleteAllPlots()

# Test floating point case (non-zero remainder)
DefineScalarExpression("const8.5", "nodal_constant(<" + meshName + ">, 8.5)")
DefineScalarExpression("const2.5", "nodal_constant(<" + meshName + ">, 2.5)")
DefineScalarExpression("mod3", "const8.5 % const2.5")
AddPlot("Pseudocolor", "mod3")
DrawPlots()
Query("MinMax")
q = GetQueryOutputObject()
TestValueEQ("Minimum Field Value", q['min'], 1.0)
TestValueEQ("Maximum Field Value", q['max'], 1.0)
DeleteAllPlots()

Exit()
