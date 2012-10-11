# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Tests:      smoke check of unv file reader.
#
#  Programmer: Olivier Cessenat
#  Date: Thu Oct 11 08:42:44 PDT 2012
# ----------------------------------------------------------------------------

datapath = data_path("unv_test_data")

OpenDatabase(pjoin(datapath, "small_sur.unv"))
AddPlot("FilledBoundary", "boundaries(freemesh)")
# Begin spontaneous state
View3DAtts = View3DAttributes()
View3DAtts.viewNormal = (0.446545, 0.0881432, 0.890409)
View3DAtts.focus = (0, 0, 2.5)
View3DAtts.viewUp = (-0.122457, 0.991793, -0.0367666)
View3DAtts.viewAngle = 30
View3DAtts.parallelScale = 3.4641
View3DAtts.nearPlane = -6.9282
View3DAtts.farPlane = 6.9282
View3DAtts.imagePan = (0, 0)
View3DAtts.imageZoom = 1
View3DAtts.perspective = 1
View3DAtts.eyeAngle = 2
View3DAtts.centerOfRotationSet = 0
View3DAtts.centerOfRotation = (0, 0, 2.5)
View3DAtts.axis3DScaleFlag = 0
View3DAtts.axis3DScales = (1, 1, 1)
View3DAtts.shear = (0, 0, 1)
SetView3D(View3DAtts)
DrawPlots()
Test("unv_01")
DeleteAllPlots()
CloseDatabase(pjoin(datapath,"small_sur.unv"))

OpenDatabase(pjoin(datapath,"small_mix.unv"))
AddPlot("FilledBoundary", "surfmats(surfmesh)")
AddPlot("Mesh", "surfmesh", 1, 0)
DrawPlots()
Test("unv_02")
DeleteAllPlots()
CloseDatabase(pjoin(datapath,"small_mix.unv"))

OpenDatabase(pjoin(datapath,"small_vol_pri_bor.unv"),0)
AddPlot("FilledBoundary", "boundaries(freemesh)")
AddPlot("Mesh", "freemesh")
DrawPlots()
Test("unv_03")
DeleteAllPlots()
CloseDatabase(pjoin(datapath,"small_vol_pri_bor.unv"))

OpenDatabase(pjoin(datapath,"small_vol_tet_bor.unv"),0)
AddPlot("FilledBoundary", "boundaries(freemesh)")
AddPlot("Mesh", "facemesh")
DrawPlots()
Test("unv_04")
DeleteAllPlots()
CloseDatabase(pjoin(datapath,"small_vol_tet_bor.unv"))

OpenDatabase(pjoin(datapath,"small_vol_pyr_bor.unv"), 0)
AddPlot("FilledBoundary", "load_sets(facemesh)")
AddPlot("Mesh", "facemesh")
AddPlot("Mesh", "mesh")
SetActivePlots(2)
AddOperator("ThreeSlice", 0)
SetActivePlots(2)
ThreeSliceAtts = ThreeSliceAttributes()
ThreeSliceAtts.x = 0.1
ThreeSliceAtts.y = 0.2
ThreeSliceAtts.z = 0.24
ThreeSliceAtts.interactive = 1
SetOperatorOptions(ThreeSliceAtts, 0)
SetActivePlots(0)
FilledBoundaryAtts = FilledBoundaryAttributes()
FilledBoundaryAtts.colorType = FilledBoundaryAtts.ColorByMultipleColors
FilledBoundaryAtts.colorTableName = "Default"
FilledBoundaryAtts.invertColorTable = 0
FilledBoundaryAtts.filledFlag = 1
FilledBoundaryAtts.legendFlag = 1
FilledBoundaryAtts.lineStyle = FilledBoundaryAtts.SOLID
FilledBoundaryAtts.lineWidth = 0
FilledBoundaryAtts.singleColor = (0, 0, 0, 255)
FilledBoundaryAtts.SetMultiColor(0, (255, 0, 0, 255))
FilledBoundaryAtts.SetMultiColor(1, (0, 255, 0, 255))
FilledBoundaryAtts.boundaryType = FilledBoundaryAtts.Material
FilledBoundaryAtts.opacity = 0.258824
FilledBoundaryAtts.wireframe = 0
FilledBoundaryAtts.drawInternal = 0
FilledBoundaryAtts.smoothingLevel = 0
FilledBoundaryAtts.cleanZonesOnly = 0
FilledBoundaryAtts.mixedColor = (255, 255, 255, 255)
FilledBoundaryAtts.pointSize = 0.05
FilledBoundaryAtts.pointType = FilledBoundaryAtts.Point
FilledBoundaryAtts.pointSizeVarEnabled = 0
FilledBoundaryAtts.pointSizeVar = "default"
FilledBoundaryAtts.pointSizePixels = 2
SetPlotOptions(FilledBoundaryAtts)
View3DAtts = View3DAttributes()
View3DAtts.viewNormal = (0.440531, 0.546377, 0.712323)
View3DAtts.focus = (0, 0, 2.5)
View3DAtts.viewUp = (-0.429096, 0.825111, -0.367517)
View3DAtts.viewAngle = 30
View3DAtts.parallelScale = 3.4641
View3DAtts.nearPlane = -6.9282
View3DAtts.farPlane = 6.9282
View3DAtts.imagePan = (0, 0)
View3DAtts.imageZoom = 1
View3DAtts.perspective = 1
View3DAtts.eyeAngle = 2
View3DAtts.centerOfRotationSet = 0
View3DAtts.centerOfRotation = (0, 0, 2.5)
View3DAtts.axis3DScaleFlag = 0
View3DAtts.axis3DScales = (1, 1, 1)
View3DAtts.shear = (0, 0, 1)
SetView3D(View3DAtts)
DrawPlots()
Test("unv_05")

Exit()
