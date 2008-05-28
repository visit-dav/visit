# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  nek3d.py 
#
#  Tests:      nek3d dumps
#
#  Programmer: David Bremer
#  Date:       June 12, 2007
#
# ----------------------------------------------------------------------------
import os, string

OpenDatabase("../data/nek3d_test_data/7Pin/7Pin.100blocks.nek3d")

AddPlot("Mesh", "mesh")
DrawPlots()
Test("nek3d_reader1")


v=GetView3D()
v.viewNormal=(-0.58339, -0.40012, -0.706796)
v.viewUp=(-0.664278, -0.265658, 0.698685)
SetView3D(v)
Test("nek3d_reader2")


DeleteAllPlots()
AddPlot("Vector", "velocity")
v = VectorAttributes()
v.nVectors = 10
SetPlotOptions(v)
DrawPlots()
Test("nek3d_reader3")


SetTimeSliderState(1)
DrawPlots()
Test("nek3d_reader4")


SetTimeSliderState(2)
DeleteAllPlots()
AddPlot("Pseudocolor", "pressure")
DrawPlots()
Test("nek3d_reader5")

# -----------------------------
DefineScalarExpression("vx", "velocity[0]")
DefineScalarExpression("vy", "velocity[1]")
DefineScalarExpression("vz", "velocity[2]")

DeleteAllPlots()
OpenDatabase("../data/nek3d_test_data/3dascii/test_ascii.nek3d")
AddPlot("Pseudocolor", "temperature")
DrawPlots()
Test("nek3d_reader6")

DeleteAllPlots()
AddPlot("Mesh", "mesh");
AddPlot("Pseudocolor", "vz")
DrawPlots()
Test("nek3d_reader7")

# -----------------------------
DeleteAllPlots()
OpenDatabase("../data/nek3d_test_data/2dascii/couette.nek2d")
ResetView()
AddPlot("Vector", "velocity")
DrawPlots()
Test("nek3d_reader8")

DeleteAllPlots()
AddPlot("Pseudocolor", "pressure")
DrawPlots()
Test("nek3d_reader9")

# -----------------------------
DeleteAllPlots()
OpenDatabase("../data/nek3d_test_data/2dbinary/couette.nek")
ResetView()
AddPlot("Mesh", "mesh")
DrawPlots()
Test("nek3d_reader10")

# -----------------------------
DeleteAllPlots()
OpenDatabase("../data/nek3d_test_data/parallel_nek_output_example/par_test.nek3d")
ResetView()
AddPlot("Vector", "velocity")
DrawPlots()
Test("nek3d_reader11")

DeleteAllPlots()
AddPlot("Pseudocolor", "vy")
DrawPlots()
Test("nek3d_reader12")

Exit()




