# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  watertight.py
#  Tests:      queries     - watertight 
#
#  Defect ID:  VisIt00006632
#
#  Programmer: Hank Childs
#  Date:       September 23, 2005
#
# ----------------------------------------------------------------------------

OpenDatabase("../data/rect3d.silo")
AddPlot("Pseudocolor", "d")
AddOperator("Isosurface")
i = IsosurfaceAttributes()
i.contourMethod = i.Value
i.contourValue = 0.48
SetOperatorOptions(i)
DrawPlots()

Query("Watertight")
text = GetQueryOutputString()
TestText("watertight_01", text)

i.contourValue = 0.5
SetOperatorOptions(i)
Query("Watertight")
text = GetQueryOutputString()
TestText("watertight_02", text)

Exit()
