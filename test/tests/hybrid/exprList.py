# ---------------------------------------------------------------------------- 
#  CLASSES: nightly
#
#  Test Case:  exprList.py
#  Tests:      Expression list contents as windows are added and we switch
#              between databases that have expressions.
#
#  Defect ID:  VisIt00003955
#
#  Programmer: Brad Whitlock
#  Date:       Fri Oct 24 18:06:01 PST 2003
#
#  Modifications:
#    
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
# ----------------------------------------------------------------------------

def ExprStrings():
    exprList = Expressions()
    str = "Expressions:\n"
    index = 1
    for expr in exprList:
        str = str + "expression %d: %s = %s\n" % (index, expr[0], expr[1])
        index = index + 1
    return str

TurnOnAllAnnotations()

# Define some expressions just so we have some in the list.
DefineScalarExpression("var1", "var2 + var3")
DefineScalarExpression("var4", "var5 * var6")
DefineScalarExpression("var7", "var8 / var9")

# Open the first database, which has some expressions.
OpenDatabase(data_path("silo_%s_test_data/globe.silo")%SILO_MODE)

AddPlot("Pseudocolor", "speed")
DrawPlots()

# This test should show our scalar expressions + globe's expressions.
TestText("exprList00", ExprStrings())

# Add a new window and open a different database that has no expressions of
# its own.
AddWindow()
SetActiveWindow(2)
DeleteAllPlots()
OpenDatabase(data_path("silo_%s_test_data/wave*.silo database")%SILO_MODE)

TestText("exprList01", ExprStrings())

# Going back to window 1, where globe is open. This should make the expression
# list contain globe's expressions.
SetActiveWindow(1)
TestText("exprList02", ExprStrings())

# Open a new database. This should make the expression list contain the
# expressions for rect3d and our scalar expressions.
OpenDatabase(data_path("silo_%s_test_data/rect3d.silo")%SILO_MODE)

TestText("exprList03", ExprStrings())

# Add a plot
AddPlot("Pseudocolor", "u")
DrawPlots()

# Make the active plot be the plot of globe. The expression list should 
# contain globe's expressions.
SetActivePlots(0)
TestText("exprList04", ExprStrings())

Exit()
