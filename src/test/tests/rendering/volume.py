# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  volume.py
#
#  Tests:      mesh      - 3D unstructured, one domain 
#              plots     - volume 
#              operators - none
#              selection - yes 
#
#  Programmer: Mark C. Miller 
#  Date:       01Jul03
#
#  Modifications:
#    Brad Whitlock, Wed Dec 15 09:36:51 PDT 2004
#    I changed the flag that's used to make it do software rendering.
#
#    Hank Childs, Mon Jul 11 14:07:16 PDT 2005
#    Added test for rectilinear grids with ghost zones and hardware rendering
#    ['5712].
#
#    Hank Childs, Wed Jul 13 10:31:08 PDT 2005
#    Delete wireframe plot, since that exposes '6380.
#
#    Jeremy Meredith, Wed Sep  7 12:06:04 PDT 2005
#    Allowed spaces in variable names.
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
#
#    Cyrus Harrison, Wed Aug 25 14:25:13 PDT 2010
#    SIL id shift (from 4 to 5) due to changes in SIL generation.
#
#    Brad Whitlock, Wed Sep 28 11:48:16 PDT 2011
#    Zoom in a little so we have more pixels covered.
#
# ----------------------------------------------------------------------------


#START SECTION TO BE MOVED

DeleteAllPlots()
OpenDatabase(data_path("samrai_test_data/sil_changes/dumps.visit"))


# '6380.  The wireframe is not composited correctly when in SR mode.
# So delete the wireframe and we should get the same picture in SR and
# non-SR.  When '6380 is fixed, the DeleteAllPlots() call below should be
# removed and the baseline should be reset.
DeleteAllPlots()

AddPlot("Volume", "Primitive Var _number_0")
volAtts = VolumeAttributes()
volAtts.rendererType =  volAtts.Serial
volAtts.smoothData = 0
volAtts.useColorVarMin = 1
volAtts.colorVarMin = 22
SetPlotOptions(volAtts)
DrawPlots()
ResetView()
v = GetView3D()
v.viewNormal = (-0.369824, 0.535308, 0.759391)
v.viewUp = (-0.022009, 0.812062, -0.583155)
SetView3D(v)

Test("volume_03")

DeleteAllPlots()

# END SECTION TO BE MOVED

OpenDatabase(silo_data_path("globe.silo"))

AddPlot("Volume", "t")
silr=SILRestriction()
silr.TurnOffSet(5)
SetPlotSILRestriction(silr)

DrawPlots()

v=GetView3D()
v.viewNormal=(0.507832, -0.301407, -0.807007)
v.viewUp=(-0.831783, -0.415313, -0.368309)
v.imageZoom = 2.
SetView3D(v)

# test the serial volume render
Test("volume_01")

# test software volume render
volAtts = VolumeAttributes()
volAtts.rendererType = volAtts.Composite
SetPlotOptions(volAtts)
Test("volume_02")

DeleteAllPlots()

# Single domain checks, plots 10-16
OpenDatabase(silo_data_path("rect3d.silo"))
AddPlot("Volume", "d")
DrawPlots()

View3DAtts = View3DAttributes()
View3DAtts.viewNormal = (0.628533, 0.511363, 0.58605)
View3DAtts.focus = (0.5, 0.5, 0.5)
View3DAtts.viewUp = (-0.169088, 0.825303, -0.538779)
View3DAtts.viewAngle = 30
View3DAtts.parallelScale = 0.866025
View3DAtts.nearPlane = -1.73205
View3DAtts.farPlane = 1.73205
View3DAtts.imagePan = (0, 0)
View3DAtts.imageZoom = 1
View3DAtts.perspective = 0
View3DAtts.eyeAngle = 2
View3DAtts.centerOfRotationSet = 0
View3DAtts.centerOfRotation = (0.5, 0.5, 0.5)
View3DAtts.axis3DScaleFlag = 0
View3DAtts.axis3DScales = (1, 1, 1)
View3DAtts.shear = (0, 0, 1)
View3DAtts.windowValid = 1
SetView3D(View3DAtts)

volAtts = VolumeAttributes()
volAtts.rendererType = volAtts.Serial
volAtts.resampleType = volAtts.NoResampling
volAtts.OSPRayEnabledFlag = 0
SetPlotOptions(volAtts)
Test("volume_10")

volAtts = VolumeAttributes()
volAtts.rendererType = volAtts.Serial
volAtts.resampleType = volAtts.OnlyIfRequired
volAtts.OSPRayEnabledFlag = 0
SetPlotOptions(volAtts)
Test("volume_11")

volAtts = VolumeAttributes()
volAtts.rendererType = volAtts.Serial
volAtts.resampleType = volAtts.OnlyIfRequired
volAtts.OSPRayEnabledFlag = 1
SetPlotOptions(volAtts)
Test("volume_12")

volAtts = VolumeAttributes()
volAtts.rendererType = volAtts.Serial
volAtts.resampleType = volAtts.SingleDomain
volAtts.OSPRayEnabledFlag = 0
SetPlotOptions(volAtts)
Test("volume_13")

volAtts = VolumeAttributes()
volAtts.rendererType = volAtts.Serial
volAtts.resampleType = volAtts.SingleDomain
volAtts.OSPRayEnabledFlag = 1
SetPlotOptions(volAtts)
Test("volume_14")

volAtts = VolumeAttributes()
volAtts.rendererType = volAtts.Parallel
volAtts.resampleType = volAtts.ParallelRedistribute
volAtts.OSPRayEnabledFlag = 0
SetPlotOptions(volAtts)
Test("volume_15")

volAtts = VolumeAttributes()
volAtts.rendererType = volAtts.Parallel
volAtts.resampleType = volAtts.ParallelRedistribute
volAtts.OSPRayEnabledFlag = 1
SetPlotOptions(volAtts)
Test("volume_16")

DeleteAllPlots()

# Multiple domain checks, plots 20-24
OpenDatabase(silo_data_path("multi_rect3d.silo"))
AddPlot("Volume", "d")
DrawPlots()

View3DAtts = View3DAttributes()
View3DAtts.viewNormal = (0.628533, 0.511363, 0.58605)
View3DAtts.focus = (0.5, 0.5, 0.5)
View3DAtts.viewUp = (-0.169088, 0.825303, -0.538779)
View3DAtts.viewAngle = 30
View3DAtts.parallelScale = 0.866025
View3DAtts.nearPlane = -1.73205
View3DAtts.farPlane = 1.73205
View3DAtts.imagePan = (0, 0)
View3DAtts.imageZoom = 1
View3DAtts.perspective = 0
View3DAtts.eyeAngle = 2
View3DAtts.centerOfRotationSet = 0
View3DAtts.centerOfRotation = (0.5, 0.5, 0.5)
View3DAtts.axis3DScaleFlag = 0
View3DAtts.axis3DScales = (1, 1, 1)
View3DAtts.shear = (0, 0, 1)
View3DAtts.windowValid = 1
SetView3D(View3DAtts)

volAtts = VolumeAttributes()
volAtts.rendererType = volAtts.Serial
volAtts.resampleType = volAtts.NoResampling
volAtts.OSPRayEnabledFlag = 0
SetPlotOptions(volAtts)
Test("volume_20")

volAtts = VolumeAttributes()
volAtts.rendererType = volAtts.Serial
volAtts.resampleType = volAtts.ParallelRedistribute
volAtts.OSPRayEnabledFlag = 0
SetPlotOptions(volAtts)
Test("volume_21")

volAtts = VolumeAttributes()
volAtts.rendererType = volAtts.Serial
volAtts.resampleType = volAtts.ParallelRedistribute
volAtts.OSPRayEnabledFlag = 1
SetPlotOptions(volAtts)
Test("volume_22")

volAtts = VolumeAttributes()
volAtts.rendererType = volAtts.Parallel
volAtts.resampleType = volAtts.ParallelPerRank
volAtts.OSPRayEnabledFlag = 0
SetPlotOptions(volAtts)
Test("volume_23")

volAtts = VolumeAttributes()
volAtts.rendererType = volAtts.Parallel
volAtts.resampleType = volAtts.ParallelPerRank
volAtts.OSPRayEnabledFlag = 1
SetPlotOptions(volAtts)
Test("volume_24")

DeleteAllPlots()

# Multiple domain checks with missing data, plots 30-34
OpenDatabase(silo_data_path("multi_ucd3d.silo"))
AddPlot("Volume", "p")
DrawPlots()

View3DAtts = View3DAttributes()
View3DAtts.viewNormal = (0.409673, 0.375032, 0.831576)
View3DAtts.focus = (0, 2.5, 10)
View3DAtts.viewUp = (-0.208428, 0.925952, -0.314914)
View3DAtts.viewAngle = 30
View3DAtts.parallelScale = 11.4564
View3DAtts.nearPlane = -22.9129
View3DAtts.farPlane = 22.9129
View3DAtts.imagePan = (0, 0)
View3DAtts.imageZoom = 1
View3DAtts.perspective = 0
View3DAtts.eyeAngle = 2
View3DAtts.centerOfRotationSet = 0
View3DAtts.centerOfRotation = (0, 2.5, 10)
View3DAtts.axis3DScaleFlag = 0
View3DAtts.axis3DScales = (1, 1, 1)
View3DAtts.shear = (0, 0, 1)
View3DAtts.windowValid = 1
SetView3D(View3DAtts)

# This test should fail and produce nothing
volAtts = VolumeAttributes()
volAtts.rendererType = volAtts.Serial
volAtts.resampleType = volAtts.NoResampling
volAtts.OSPRayEnabledFlag = 0
SetPlotOptions(volAtts)
Test("volume_30")

volAtts = VolumeAttributes()
volAtts.rendererType = volAtts.Serial
volAtts.resampleType = volAtts.ParallelRedistribute
volAtts.OSPRayEnabledFlag = 0
SetPlotOptions(volAtts)
Test("volume_31")

volAtts = VolumeAttributes()
volAtts.rendererType = volAtts.Serial
volAtts.resampleType = volAtts.ParallelRedistribute
volAtts.OSPRayEnabledFlag = 1
SetPlotOptions(volAtts)
Test("volume_32")

volAtts = VolumeAttributes()
volAtts.rendererType = volAtts.Parallel
volAtts.resampleType = volAtts.ParallelPerRank
volAtts.OSPRayEnabledFlag = 0
SetPlotOptions(volAtts)
Test("volume_33")

volAtts = VolumeAttributes()
volAtts.rendererType = volAtts.Parallel
volAtts.resampleType = volAtts.ParallelPerRank
volAtts.OSPRayEnabledFlag = 1
SetPlotOptions(volAtts)
Test("volume_34")

Exit()
