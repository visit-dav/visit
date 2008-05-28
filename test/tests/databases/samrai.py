# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  samrai.py 
#
#  Tests:      presence of HDF5
#              default plot 
#              sil changing with time
#              nested ghosting
#              zbuffer contention
#              2d ale data
#              operators - slice
#              selection - by level
#
#  Programmer: Mark C. Miller 
#  Date:       November 12, 2003 
#
#  Modifications:
#    Brad Whitlock, Thu Mar 25 14:05:32 PST 2004
#    Added a check to see if plots at later time steps get the right metadata.
#
#    Jeremy Meredith, Thu Jun 24 12:58:06 PDT 2004
#    Set the vector origin explicitly for some cases because I changed the
#    default to Tail.
#
#    Mark C. Miller, Thu Aug 19 16:22:49 PDT 2004
#    Turned off database info flag so future changes to samrai db plugin
#    are less likely to result in a need to update the baselines
#
#    Hank Childs, Wed Feb 16 07:34:07 PST 2005
#    Rename variables that have unsupported characters.
#
#    Mark C. Miller, Thu Feb 17 09:00:41 PST 2005
#    Added tests for variables defined on only some patches
#
#    Brad Whitlock, Wed Mar 9 09:15:30 PDT 2005
#    Removed deprecated functions.
#
#    Jeremy Meredith, Wed Sep  7 12:06:04 PDT 2005
#    Allowed spaces in variable names.
#
#    Mark C. Miller, Mon Nov  5 20:15:05 PST 2007
#    Added tests for sparse materials, expressions
#
# ----------------------------------------------------------------------------

LevelZero = 4 # set id for level 0

# Turn off all annotation
TurnOffAllAnnotations() # defines global object 'a'

OpenDatabase("../data/samrai_test_data/sil_changes/dumps.visit")

#
# just tests that default plot comes up automatically 
#
Test("samrai_01")

#
# test exterenal face overlap
#
AddPlot("Pseudocolor","Primitive Var _number_0")
DrawPlots()

v=GetView3D()
v.viewNormal=(0,0,-1)
SetView3D(v)
Test("samrai_02")

v.viewNormal=(0.34202, 0, -0.939693)
SetView3D(v)
Test("samrai_03")

#
# test level selection
#
silr=SILRestriction()
silr.TurnOffSet(LevelZero)
SetPlotSILRestriction(silr)
v.viewNormal=(0,0,1)
SetView3D(v)
Test("samrai_04")

#
# test changing time steps (and sil)
#
TimeSliderNextState()
Test("samrai_05")

# go back to begnning of time
SetTimeSliderState(0)

#
# turn on level 0 so we can check that
# nested ghosting is working correctly when we slice
#
silr.TurnOnSet(LevelZero)
SetPlotSILRestriction(silr)

#
# this test is here to remind us we have many internal faces 
#
v.viewNormal=(-0.537631, 0.372794, -0.756292)
v.focus=(15, 10, 10)
v.viewUp=(0.262569, 0.926375, 0.269976)
v.parallelScale=30
v.nearPlane=-10
v.farPlane=90
SetView3D(v)
Test("samrai_06")

#
# add a mesh plot and slice operator
#
AddPlot("Mesh","amr_mesh")
sliceAtts=SliceAttributes()
sliceAtts.originType=sliceAtts.Percent
sliceAtts.originPercent=20
sliceAtts.axisType=sliceAtts.ZAxis
sliceAtts.project2d=1
SetDefaultOperatorOptions(sliceAtts)
AddOperator("Slice",1)
DrawPlots()
Test("samrai_07")

#
# Animate forward 1 frame
#
TimeSliderNextState()
Test("samrai_08")

#
# Now, go back 1 frame
#
SetTimeSliderState(0)
Test("samrai_09")

#
# Tests for data from a 2d ale calculation.
#
DeleteAllPlots()

OpenDatabase("../data/samrai_test_data/ale2d/dumps.visit")
ResetView()

#
# Test the default plot.
#
Test("samrai_10")

#
# Check the last frame.
#
TimeSliderNextState()
Test("samrai_11")

#
# Test a vector plot.
#
AddPlot("Vector", "Velocity")
vectorAtts=VectorAttributes()
vectorAtts.scale = 0.02
vectorAtts.vectorOrigin = vectorAtts.Middle
SetPlotOptions(vectorAtts)
DrawPlots()

v = View2DAttributes()
v.windowCoords = (0.31, 0.58, 0.15, 0.36)
SetView2D(v)
Test("samrai_12")

#
# Tests for material/species data 
#
DeleteAllPlots()

OpenDatabase("../data/samrai_test_data/mats-par3/dumps.visit")
ResetView()

#
# test a material plot
#
AddPlot("FilledBoundary","materials")
DrawPlots()
Test("samrai_13")

#
# test material plot with some patches off
#
silr=SILRestriction()
silr.TurnOffSet(1)
silr.TurnOffSet(2)
silr.TurnOffSet(3)
silr.TurnOffSet(4)
SetPlotSILRestriction(silr)
Test("samrai_14")

#
# now, turn off a material, too
#
silr.TurnOnAll()
silr.TurnOffSet(20)
SetPlotSILRestriction(silr)
Test("samrai_15")

silr.TurnOnAll()
SetPlotSILRestriction(silr)
DeleteAllPlots()

#
# add a species plot
#
AddPlot("Pseudocolor","species")
DrawPlots()

#
# Selectively turn off each species
#
silr.TurnOffSet(24)
SetPlotSILRestriction(silr)
Test("samrai_16")

silr.TurnOffSet(25)
SetPlotSILRestriction(silr)
Test("samrai_17")

silr.TurnOffSet(26)
SetPlotSILRestriction(silr)
Test("samrai_18")

silr.TurnOffSet(27)
SetPlotSILRestriction(silr)
Test("samrai_19")

silr.TurnOffSet(28)
SetPlotSILRestriction(silr)
Test("samrai_20")

DeleteAllPlots()

#
# Open a new database that has levels that change over time so we can test
# that plots get the right metadata and SIL as time advances.
#
OpenDatabase("../data/samrai_test_data/front/dumps.visit")
# Get rid of the default plot
DeleteAllPlots()
AddPlot("FilledBoundary", "levels")
DrawPlots()
ResetView()
Test("samrai_21")

# Go to the next time state. This used to cause an InvalidIndexException.
TimeSliderNextState()
Test("samrai_22")

#
# Test a database where some variables are not defined on all patches
#
DeleteAllPlots()
OpenDatabase("../data/samrai_test_data/viz_2d/dumps.visit")
AddPlot("Pseudocolor", "Function_0") # only defined on some patches
AddPlot("Pseudocolor", "Function_1") # only defined on some other patches
DrawPlots()
Test("samrai_23")

#
# Test a samrai database with sparse material representation
#
DeleteAllPlots()
OpenDatabase("../data/samrai_test_data/sparse_mats/summary.samrai")
AddPlot("Pseudocolor", "energy")
DrawPlots()
Test("samrai_24")

#
#  Zoom into a region where there is mixing
#
v=GetView2D()
v.windowCoords=(-0.296975, -0.223086, 0.0437891, 0.120943)
SetView2D(v)
DrawPlots()
Test("samrai_25")

#
# Turn off different materials and see what we get for the PC plot
# and zone picks
#
silr = SILRestriction()
j = 0
for k1 in silr.SetsInCategory("materials"):
    i = 1
    for k2 in silr.SetsInCategory("materials"):
        if silr.SetName(k2) == "material_%d"%(j+1):
            silr.TurnOffSet(k2)
        else:
            silr.TurnOnSet(k2)
        i = i + 1
    SetPlotSILRestriction(silr)
    DrawPlots()
    Test("samrai_%d"%(26+2*j))
    PickByZone(247,15)
    TestText("samrai_%d"%(26+2*j+1),GetPickOutput())
    j = j + 1

#
# Test some expressions from the samrai database 
#
silr.TurnOnAll()
SetPlotSILRestriction(silr)
DeleteAllPlots()
AddPlot("Pseudocolor","von_mises_stress")
DrawPlots()
Test("samrai_33")
DeleteAllPlots()
AddPlot("Pseudocolor","speed")
DrawPlots()
Test("samrai_34")

Exit()
