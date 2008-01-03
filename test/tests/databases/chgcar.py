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
# ----------------------------------------------------------------------------

# Turn off all annotations

a = AnnotationAttributes()
a.axesFlag2D = 0
a.axesFlag = 0
a.triadFlag = 0
a.bboxFlag = 0
a.userInfoFlag = 0
a.databaseInfoFlag = 0
a.legendInfoFlag = 0
a.backgroundMode = 0
a.foregroundColor = (0, 0, 0, 255)
a.backgroundColor = (255, 255, 255, 255)
SetAnnotationAttributes(a)

OpenDatabase("../data/CHGCAR", 0, "VASP_1.0")
AddPlot("Pseudocolor", "charge")
DrawPlots()

View3DAtts = GetView3D()
View3DAtts.viewNormal = (-0.85796, 0.216484, -0.465874)
View3DAtts.focus = (4.5, 0.5, 4)
View3DAtts.viewUp = (0.135878, 0.970204, 0.200602)
View3DAtts.viewAngle = 30
SetView3D(View3DAtts)

Test("chgcar_01")
ReplaceDatabase("../data/CHGCAR.vtk")
Test("chgcar_02")
ReplaceDatabase("../data/CHGCAR")

AddOperator("Resample")
ResamplePluginAtts = ResamplePluginAttributes()
ResamplePluginAtts.useExtents = 0
ResamplePluginAtts.startX = 1
ResamplePluginAtts.endX = 6
ResamplePluginAtts.samplesX = 80
ResamplePluginAtts.startY = -2
ResamplePluginAtts.endY = 4
ResamplePluginAtts.samplesY = 80
ResamplePluginAtts.is3D = 1
ResamplePluginAtts.startZ = 1
ResamplePluginAtts.endZ = 6
ResamplePluginAtts.samplesZ = 80
SetOperatorOptions(ResamplePluginAtts)

Test("chgcar_03")
ReplaceDatabase("../data/CHGCAR.vtk")
Test("chgcar_04")
ReplaceDatabase("../data/CHGCAR")

DeleteActivePlots()

AddPlot("Mesh", "mesh")
AddOperator("IndexSelect",1)
IndexSelectAtts = IndexSelectAttributes()
IndexSelectAtts.dim = IndexSelectAtts.ThreeD  # OneD, TwoD, ThreeD
IndexSelectAtts.xMin = 2
SetOperatorOptions(IndexSelectAtts)

AddPlot("Label", "charge")
LabelAtts = LabelAttributes()
LabelAtts.textHeight1 = 0.05
SetPlotOptions(LabelAtts)
DrawPlots()

Test("chgcar_11")
ReplaceDatabase("../data/CHGCAR.vtk")
Test("chgcar_12")
ReplaceDatabase("../data/CHGCAR")

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
ReplaceDatabase("../data/CHGCAR.vtk")
Test("chgcar_14")
ReplaceDatabase("../data/CHGCAR")

DeleteAllPlots()

AddPlot("Volume", "charge")
DrawPlots()
VolumeAtts = VolumeAttributes()
VolumeAtts.lightingFlag = 0
VolumeAtts.opacityAttenuation = .1
VolumeAtts.resampleTarget = 450000
VolumeAtts.rendererType = VolumeAtts.Splatting
SetPlotOptions(VolumeAtts)

Test("chgcar_05")
ReplaceDatabase("../data/CHGCAR.vtk")
Test("chgcar_06")
ReplaceDatabase("../data/CHGCAR")

VolumeAtts.opacityAttenuation = .2
VolumeAtts.rendererType = VolumeAtts.Texture3D
SetPlotOptions(VolumeAtts)

Test("chgcar_07")
ReplaceDatabase("../data/CHGCAR.vtk")
Test("chgcar_08")
ReplaceDatabase("../data/CHGCAR")

VolumeAtts.opacityAttenuation = .4
VolumeAtts.rendererType = VolumeAtts.RayCasting
SetPlotOptions(VolumeAtts)

Test("chgcar_09")
ReplaceDatabase("../data/CHGCAR.vtk")
Test("chgcar_10")
ReplaceDatabase("../data/CHGCAR")

Exit()
