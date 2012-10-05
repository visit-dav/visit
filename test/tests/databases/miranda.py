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
#  Modifications:
#    Jeremy Meredith, Tue Jul 15 10:43:58 EDT 2008
#    Changed number of vectors in vector plot to match the old behavior.
#    (We now account for how many domains there are.)
#
#    Jeremy Meredith, Thu Jan 14 11:19:12 EST 2010
#    Changed the path to the test data.
#
# ----------------------------------------------------------------------------
import os, string



OpenDatabase(data_path("miranda_test_data/TG_vortex/plot.raw"))


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
vec = VectorAttributes()
vec.nVectors = 400*64
SetPlotOptions(vec)
DrawPlots()
Test("miranda_raw_reader4")

v=GetView3D()
v.viewNormal=(-0.707107, -0.707107, 0)
v.viewUp=(0, 0, 1)
SetView3D(v)
Test("miranda_raw_reader5")

DeleteAllPlots()
ReplaceDatabase(data_path("miranda_test_data/jet_2d/plot_mat_t83.raw"))


SetTimeSliderState(0)
AddPlot("Pseudocolor","density")
DrawPlots()
Test("miranda_raw_reader6")

SetTimeSliderState(1)
DeleteAllPlots()
AddPlot("Vector", "velocity")
vec = VectorAttributes()
vec.nVectors = 400*128
SetPlotOptions(vec)
DrawPlots()
vv=GetView2D()
vv.viewportCoords=(0.2, 0.95, 0.15, 0.95)
vv.windowCoords=(0.00640306, 0.00963122, 0.0128936, 0.0161598)
SetView2D(vv)
Test("miranda_raw_reader7")

SetTimeSliderState(2)
Test("miranda_raw_reader8")

DeleteAllPlots()
ReplaceDatabase(data_path("miranda_test_data/stem_2d/plot.mir"))

vv = View2DAttributes()
vv.windowCoords = (0, 2.23563, -0.28, 0.835625)
vv.viewportCoords = (0.05, 0.95, 0.05, 0.95)
vv.fullFrameActivationMode = vv.On  # On, Off, Auto
vv.fullFrameAutoThreshold = 100
SetView2D(vv)

SetTimeSliderState(0)
AddPlot("Pseudocolor","density")
DrawPlots()
Test("miranda_raw_reader9")

SetTimeSliderState(3)
DrawPlots()
Test("miranda_raw_reader10")



Exit()
