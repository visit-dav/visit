# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  miranda.py 
#
#  Tests:      miranda raw dumps
#
#  Programmer: David Bremer
#  Date:       Feb 20, 2007
#
# ----------------------------------------------------------------------------
import os, string

TurnOffAllAnnotations()


OpenDatabase("../data/miranda_raw/TG_vortex/plot.raw")

AddPlot("Pseudocolor","density")
DrawPlots()
Test("miranda_raw_reader1")

AddPlot("Mesh", "mesh")
DrawPlots()
Test("miranda_raw_reader2")

SetTimeSliderState(1)
Test("miranda_raw_reader3")

SetTimeSliderState(2)
DeleteAllPlots()
AddPlot("Vector", "velocity")
DrawPlots()
Test("miranda_raw_reader4")

v=GetView3D()
v.viewNormal=(-0.707107, -0.707107, 0)
v.viewUp=(0, 0, 1)
SetView3D(v)
Test("miranda_raw_reader5")

DeleteAllPlots()
ReplaceDatabase("../data/miranda_raw/jet_2d/plot_mat_t83.raw")


SetTimeSliderState(0)
AddPlot("Pseudocolor","density")
DrawPlots()
Test("miranda_raw_reader6")

SetTimeSliderState(1)
DeleteAllPlots()
AddPlot("Vector", "velocity")
DrawPlots()
vv=GetView2D()
vv.viewportCoords=(0.2, 0.95, 0.15, 0.95)
vv.windowCoords=(0.00640306, 0.00963122, 0.0128936, 0.0161598)
SetView2D(vv)
Test("miranda_raw_reader7")

SetTimeSliderState(2)
Test("miranda_raw_reader8")



Exit()
