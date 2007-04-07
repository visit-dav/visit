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
a = AnnotationAttributes()
a.axesFlag2D = 0
a.axesFlag = 0
a.triadFlag = 0
a.bboxFlag = 0
a.userInfoFlag = 0
a.databaseInfoFlag = 0
a.legendInfoFlag = 0
a.backgroundMode = 0
a.foregroundColor = (0, 0, 0, 255)
a.backgroundColor = (255, 255, 255, 255)
SetAnnotationAttributes(a)

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
