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

TurnOffAllAnnotations()
OpenDatabase("../data/nek3d_test_data/7Pin.100blocks.nek3d")

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

Exit()


