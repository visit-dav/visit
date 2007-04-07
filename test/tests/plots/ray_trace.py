# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  ray_trace.py
#
#  Tests:      ray tracing with the volume plot
#
#  Defect ID:  '1949, '5699, '6916
#
#  Programmer: Hank Childs
#  Date:       December 3, 2004
#
#  Modifications:
#    Brad Whitlock, Wed Dec 15 09:36:51 PDT 2004
#    I changed the flag that's used to make it do software rendering.
#
#    Hank Childs, Wed Feb 16 07:34:07 PST 2005
#    Rename variables that have unsupported characters.
#
#    Jeremy Meredith, Wed Sep  7 12:06:04 PDT 2005
#    Allowed spaces in variable names.
#
#    Hank Childs, Fri Mar  3 09:09:04 PST 2006
#    Add testing for ray-tracing in multiple windows.
#
# ----------------------------------------------------------------------------

# Turn off all annotation
a = AnnotationAttributes()
a.axesFlag2D = 0
a.axesFlag = 0
a.triadFlag = 0
a.userInfoFlag = 0
a.databaseInfoFlag = 0
a.legendInfoFlag = 0
a.backgroundMode = 0
a.foregroundColor = (0, 0, 0, 255)
a.backgroundColor = (255, 255, 255, 255)

# Turn bbox on.  This tests:
# - interaction with geometry
# - that image based plots still get added to the vis window, causing a 
#   bounding box to get generated.
a.bboxFlag = 1
SetAnnotationAttributes(a)


#
# Start off by testing that we can bring up a normal volume plot and smooth
# the data.  Also test that it can interact with the bounding box correctly.
#
OpenDatabase("../data/rect3d.silo")
AddPlot("Volume", "d")
vol_atts = VolumeAttributes()
vol_atts.rendererType = vol_atts.RayCasting
vol_atts.smoothData = 1
SetPlotOptions(vol_atts)
DrawPlots()

v = GetView3D()
v.viewNormal = (-0.369824, 0.535308, 0.759391)
v.focus = (0.5, 0.5, 0.5)
v.viewUp = (-0.022009, 0.812062, -0.583155)
v.viewAngle = 30
v.parallelScale = 0.866025
v.nearPlane = -1.73205
v.farPlane = 1.73205
v.imagePan = (0, 0)
v.imageZoom = 1
v.perspective = 1
v.eyeAngle = 2
v.centerOfRotationSet = 0
v.centerOfRotation = (0, 0, 0)
SetView3D(v)

Test("ray_trace_01")

#
# Now test that it can play with other plots.
#
AddPlot("Pseudocolor", "d")
pc_atts = PseudocolorAttributes()
pc_atts.colorTableName = "gray"
SetPlotOptions(pc_atts)
AddOperator("Slice")
slice_atts = SliceAttributes()
slice_atts.originPercent = 90
slice_atts.axisType = slice_atts.ZAxis
slice_atts.originType = slice_atts.Percent
slice_atts.project2d = 0
SetOperatorOptions(slice_atts)
DrawPlots()

Test("ray_trace_02")

#
# Test that it can play well with other plots when doin orthographic 
# projection.
#
v.perspective = 0
SetView3D(v)

Test("ray_trace_03")

#
# Now test that we handle it well when the near and far clipping planes
# intersect the dataset.
#
v.perspective = 1
v.nearPlane = -0.5
v.farPlane = 0.5
SetView3D(v)

Test("ray_trace_04")


#
# Test that we can do curvilinear/unstructured meshes, which do an entirely
# different sort of sampling.
#
DeleteAllPlots()
OpenDatabase("../data/multi_ucd3d.silo")
AddPlot("Volume", "d")
vol_atts.rendererType = vol_atts.RayCasting
vol_atts.smoothData = 0
SetPlotOptions(vol_atts)
DrawPlots()
ResetView()
v = GetView3D()
v.viewNormal = (-0.369824, 0.535308, 0.759391)
v.viewUp = (-0.022009, 0.812062, -0.583155)
SetView3D(v)

Test("ray_trace_05")

#
# Test a multi-block rectilinear problem with ghost zones.  Use an AMR problem,
# because that will test the best if we are removing ghost zones correctly
# (ghost zone values in AMR meshes don't necessarily agree with the values of
#  the zones that refine them).
#
DeleteAllPlots()
OpenDatabase("../data/samrai_test_data/sil_changes/dumps.visit")
AddPlot("Volume", "Primitive Var _number_0")
vol_atts.rendererType = vol_atts.RayCasting
vol_atts.smoothData = 0
vol_atts.useColorVarMin = 1
vol_atts.colorVarMin = 22
SetPlotOptions(vol_atts)
DrawPlots()
ResetView()
v = GetView3D()
v.viewNormal = (-0.369824, 0.535308, 0.759391)
v.viewUp = (-0.022009, 0.812062, -0.583155)
SetView3D(v)

Test("ray_trace_06")

#
# Now do it again for another timestep.
#
SetTimeSliderState(7)
Test("ray_trace_07")

# 
# Now go to window #2 and do some more raytracing.
#
AddWindow()
SetActiveWindow(2)
DeleteAllPlots()
SetAnnotationAttributes(a)
OpenDatabase("../data/rect3d.silo")
AddPlot("Volume", "d")
vol_atts = VolumeAttributes()
vol_atts.rendererType = vol_atts.RayCasting
SetPlotOptions(vol_atts)
DrawPlots()
Test("ray_trace_08")

# And make sure everything is okay in window #1.
SetActiveWindow(1)
SetActivePlots(0)
DeleteActivePlots()
Test("ray_trace_09")

Exit()
