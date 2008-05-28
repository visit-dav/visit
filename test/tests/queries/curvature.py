# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  curvature.py
#  Tests:      queries     - curvature 
#
#  Defect ID:  VisIt00006746
#
#  Programmer: Hank Childs
#  Date:       May 16, 2006
#
# ----------------------------------------------------------------------------

# Turn off all annotation
TurnOffAllAnnotations() # defines global object 'a'

OpenDatabase("../data/rect3d.silo")
DefineScalarExpression("curvature", "mean_curvature(quadmesh3d)") 
AddPlot("Pseudocolor", "curvature")
AddOperator("Isosurface")
i = IsosurfaceAttributes()
i.contourMethod = i.Value
i.contourValue = 0.5
i.variable = "d"
SetOperatorOptions(i)
DrawPlots()

error = GetLastError()
TestText("curvature_01", error)

d = DeferExpressionAttributes()
d.exprs = ("curvature")
AddOperator("DeferExpression")
SetOperatorOptions(d)
DrawPlots()
Test("curvature_02")

Query("Weighted Variable Sum")
s = GetQueryOutputValue()

Query("3D surface area")
a = GetQueryOutputValue()

text = "The average mean curvature (manual) is %f\n" %(s/a)
TestText("curvature_03", text)

DeleteAllPlots()
AddPlot("Contour", "d")
c = ContourAttributes()
c.contourMethod = c.Value
c.contourValue = 0.5
SetPlotOptions(c)
DrawPlots()

Query("Average Mean Curvature")
t = GetQueryOutputString()
TestText("curvature_04", t)

Exit()
