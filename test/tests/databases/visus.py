# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  visus.py 
#
#  Tests:      Visus data 
#              Data selections in parallel
#
#  Programmer: Mark C. Miller 
#  Date:       June 8, 2005 
#
# ----------------------------------------------------------------------------
import os, string

TurnOffAllAnnotations() # defines global object 'a'

atts = IndexSelectAttributes()
# Dim is an enumerated type.  0 -> 1D, 1 -> 2D, 2 -> 3D.
atts.dim = 2
atts.xMin = 0
atts.xMax = -1 
atts.xIncr = 4 
atts.yMin = 0
atts.yMax = -1 
atts.yIncr = 4 
atts.zMin = 0 
atts.zMax = -1
atts.zIncr = 4

OpenDatabase("../data/visus_test_data/idx/streaming.idx")
SetTimeSliderState(4)

#
# Test index select (selection done by database) 
#
AddPlot("Pseudocolor","field00")
AddOperator("IndexSelect")
SetOperatorOptions(atts);
DrawPlots()

v=GetView3D()
v.viewNormal=(-0.707107, -0.707107, 0)
v.viewUp=(0, 0, 1)
SetView3D(v)
Test("visus_01")

#
# Test advancing to non-existent time step
#
SetTimeSliderState(5)
Test("visus_02")

#
# Now, advance to the last existent state 
#
SetTimeSliderState(23)
Test("visus_03")

#
# Change the index selection
#
atts.xIncr = 6 
atts.yIncr = 6 
atts.zIncr = 1 
SetOperatorOptions(atts);
Test("visus_04")

DeleteAllPlots()

#
# Test Box (box done by database) 
#
boxAtts = BoxAttributes()
boxAtts.amount = 0
boxAtts.minx = 0 
boxAtts.maxx = 140 
boxAtts.miny = 0 
boxAtts.maxy = 250 
boxAtts.minz = 110 
boxAtts.maxz = 160 

AddPlot("Pseudocolor","field00")
AddOperator("Box")
SetOperatorOptions(boxAtts);
DrawPlots()
Test("visus_05")

DeleteAllPlots()

#
# Test index select followed by box (all handled by database)
#
AddPlot("Pseudocolor","field00")
AddOperator("IndexSelect")
atts.xIncr = 4 
atts.yIncr = 4 
atts.zIncr = 1 
SetOperatorOptions(atts);
AddOperator("Box")
SetOperatorOptions(boxAtts);
DrawPlots()
Test("visus_06")


#
# Test the other visus format--it uses Peter Lindstrom's floating
# point compression library.
#
DeleteAllPlots()
ReplaceDatabase("../data/visus_test_data/TGvortex_C_lin/Visus/viz.idx")
AddPlot("Pseudocolor","x_velocity")
DrawPlots()
Test("visus_07")

DeleteAllPlots()
AddPlot("Pseudocolor","energy")
DrawPlots()
SetTimeSliderState(3)
Test("visus_08")

DeleteAllPlots()
AddPlot("Pseudocolor","conductivity")
DrawPlots()
v=GetView3D()
v.viewNormal=(-0.707107, -0.707107, 0)
v.viewUp=(0, 0, 1)
SetView3D(v)
Test("visus_09")



Exit()
