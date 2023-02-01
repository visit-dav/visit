# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  chgcar.py
#
#  Tests:      mesh      - transformed rectilinear
#
#  Defect ID:  none
#
#  Programmer: Jeremy Meredith
#  Date:       February 20, 2007
#
#    Jeremy Meredith, Wed Aug 13 14:39:11 EDT 2008
#    Unrestricted the number of labels plotted.  Since I now
#    have automatic decomposition of chgcar files, the actual
#    labels chosen was varying based on serial vs parallel
#    (which is fine, but doesn't help for regression testing).
#
#    Hank Childs, Wed Dec 31 14:32:34 PST 2008
#    Rename ResamplePluginAtts to ResampleAtts.
#
#    Cyrus Harrison, Thu Mar 25 09:57:34 PDT 2010
#    Added call(s) to DrawPlots() b/c of changes to the default plot state
#    behavior when an operator is added.
#
#    Kathleen Biagas, Mon Nov 28, 2022
#    Replace obsolete Label text attributes with new versions.
#
# ----------------------------------------------------------------------------


OpenDatabase(data_path("chgcar_test_data/CHGCAR"),0, "VASP_1.0")

AddPlot("Pseudocolor", "charge")
DrawPlots()

View3DAtts = GetView3D()
View3DAtts.viewNormal = (-0.85796, 0.216484, -0.465874)
View3DAtts.focus = (4.5, 0.5, 4)
View3DAtts.viewUp = (0.135878, 0.970204, 0.200602)
View3DAtts.viewAngle = 30
SetView3D(View3DAtts)

Test("chgcar_01")
ReplaceDatabase(data_path("chgcar_test_data/CHGCAR.vtk"))

Test("chgcar_02")
ReplaceDatabase(data_path("chgcar_test_data/CHGCAR"))


AddOperator("Resample")
ResampleAtts = ResampleAttributes()
ResampleAtts.useExtents = 0
ResampleAtts.startX = 1
ResampleAtts.endX = 6
ResampleAtts.samplesX = 80
ResampleAtts.startY = -2
ResampleAtts.endY = 4
ResampleAtts.samplesY = 80
ResampleAtts.is3D = 1
ResampleAtts.startZ = 1
ResampleAtts.endZ = 6
ResampleAtts.samplesZ = 80
SetOperatorOptions(ResampleAtts)
DrawPlots()

Test("chgcar_03")
ReplaceDatabase(data_path("chgcar_test_data/CHGCAR.vtk"))

Test("chgcar_04")
ReplaceDatabase(data_path("chgcar_test_data/CHGCAR"))


DeleteActivePlots()

AddPlot("Mesh", "mesh")
AddOperator("IndexSelect",1)
IndexSelectAtts = IndexSelectAttributes()
IndexSelectAtts.dim = IndexSelectAtts.ThreeD  # OneD, TwoD, ThreeD
IndexSelectAtts.xMin = 2
SetOperatorOptions(IndexSelectAtts)

AddPlot("Label", "charge")
LabelAtts = LabelAttributes()
LabelAtts.textFont1.scale = 7
LabelAtts.restrictNumberOfLabels = 0
SetPlotOptions(LabelAtts)
DrawPlots()

Test("chgcar_11")
ReplaceDatabase(data_path("chgcar_test_data/CHGCAR.vtk"))

Test("chgcar_12")
ReplaceDatabase(data_path("chgcar_test_data/CHGCAR"))


DeleteAllPlots()



AddPlot("Pseudocolor", "charge")
AddOperator("ThreeSlice")
ThreeSliceAtts = ThreeSliceAttributes()
ThreeSliceAtts.x = 3
ThreeSliceAtts.y = 0
ThreeSliceAtts.z = 4
SetOperatorOptions(ThreeSliceAtts)
DrawPlots()

Test("chgcar_13")
ReplaceDatabase(data_path("chgcar_test_data/CHGCAR.vtk"))

Test("chgcar_14")
ReplaceDatabase(data_path("chgcar_test_data/CHGCAR"))


DeleteAllPlots()

AddPlot("Volume", "charge")
DrawPlots()
VolumeAtts = VolumeAttributes()
VolumeAtts.lightingFlag = 0
VolumeAtts.opacityAttenuation = .6
VolumeAtts.resampleTarget = 450000
SetPlotOptions(VolumeAtts)

Test("chgcar_05")
ReplaceDatabase(data_path("chgcar_test_data/CHGCAR.vtk"))

Test("chgcar_06")
ReplaceDatabase(data_path("chgcar_test_data/CHGCAR"))


VolumeAtts.opacityAttenuation = .9
SetPlotOptions(VolumeAtts)

Test("chgcar_07")
ReplaceDatabase(data_path("chgcar_test_data/CHGCAR.vtk"))

Test("chgcar_08")
ReplaceDatabase(data_path("chgcar_test_data/CHGCAR"))


VolumeAtts.opacityAttenuation = .4
VolumeAtts.rendererType = VolumeAtts.RayCasting
SetPlotOptions(VolumeAtts)

Test("chgcar_09")
ReplaceDatabase(data_path("chgcar_test_data/CHGCAR.vtk"))

Test("chgcar_10")
ReplaceDatabase(data_path("chgcar_test_data/CHGCAR"))


Exit()
