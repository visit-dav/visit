# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  bigdata.py
#
#  Programmer: Hank Childs
#  Date:       August 27, 2010
#
# ----------------------------------------------------------------------------


OpenDatabase(silo_data_path("noise.silo"))

AddPlot("Pseudocolor", "hardyglobal")
AddOperator("Transform")
t = TransformAttributes()
t.doScale = 1
t.scaleX= 1e+20
t.scaleY= 1e+20
t.scaleZ= 1e+20
SetOperatorOptions(t)
DrawPlots()
Test("bigdata_01")

AddOperator("Isosurface")
DrawPlots()
Test("bigdata_02")

v = GetView3D()
v.axis3DScaleFlag = 1
v.axis3DScales = (2,1,1)
SetView3D(v)
Test("bigdata_03")

Exit()
