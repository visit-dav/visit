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
vol_atts = VolumeAttributes()
vol_atts.rendererType = vol_atts.Splatting
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

# test fast volume render
Test("volume_01")

# test software volume render
volAtts=VolumeAttributes()
volAtts.rendererType = volAtts.RayCasting
SetPlotOptions(volAtts)
Test("volume_02")

Exit()
