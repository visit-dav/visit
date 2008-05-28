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
# ----------------------------------------------------------------------------

# Turn off all annotation
TurnOffAllAnnotations() # defines global object 'a'

#START SECTION TO BE MOVED

DeleteAllPlots()
OpenDatabase("../data/samrai_test_data/sil_changes/dumps.visit")

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

OpenDatabase("../data/globe.silo")
AddPlot("Volume", "t")
silr=SILRestriction()
silr.TurnOffSet(4)
SetPlotSILRestriction(silr)

DrawPlots()

v=GetView3D()
v.viewNormal=(0.507832, -0.301407, -0.807007)
v.viewUp=(-0.831783, -0.415313, -0.368309)
SetView3D(v)

# test fast volume render
Test("volume_01")

# test software volume render
volAtts=VolumeAttributes()
volAtts.rendererType = volAtts.RayCasting
SetPlotOptions(volAtts)
Test("volume_02")

Exit()
