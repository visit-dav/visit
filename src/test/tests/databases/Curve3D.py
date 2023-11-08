# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  Curve3D.py
#
#  Programmer: Olivier Cessenat
#  Wed Nov  8 18:45:00 CET 2023
#
# ----------------------------------------------------------------------------
TurnOffAllAnnotations()

OpenDatabase(data_path("Curve3D_test_data/a3dcurvf.curve3d"), 0, "Curve3D_1.0")
AddPlot("FilledBoundary", "segments", 1, 1)
# Begin spontaneous state
View3DAtts = View3DAttributes()
View3DAtts.viewNormal = (0.087671, -0.95302, 0.289943)
View3DAtts.focus = (2.5, 1.5, 0)
View3DAtts.viewUp = (-0.114612, 0.279481, 0.953286)
View3DAtts.viewAngle = 30
View3DAtts.parallelScale = 3.08221
View3DAtts.nearPlane = -6.16441
View3DAtts.farPlane = 6.16441
View3DAtts.imagePan = (0, 0)
View3DAtts.imageZoom = 1
View3DAtts.perspective = 1
View3DAtts.eyeAngle = 2
View3DAtts.centerOfRotationSet = 0
View3DAtts.centerOfRotation = (2.5, 1.5, 0)
View3DAtts.axis3DScaleFlag = 0
View3DAtts.axis3DScales = (1, 1, 1)
View3DAtts.shear = (0, 0, 1)
View3DAtts.windowValid = 1
SetView3D(View3DAtts)
# End spontaneous state

FilledBoundaryAtts = FilledBoundaryAttributes()
FilledBoundaryAtts.colorType = FilledBoundaryAtts.ColorByMultipleColors  # ColorBySingleColor, ColorByMultipleColors, ColorByColorTable
FilledBoundaryAtts.colorTableName = "Default"
FilledBoundaryAtts.invertColorTable = 0
FilledBoundaryAtts.legendFlag = 1
FilledBoundaryAtts.lineWidth = 2
FilledBoundaryAtts.singleColor = (0, 0, 0, 255)
FilledBoundaryAtts.SetMultiColor(0, (255, 0, 0, 255))
FilledBoundaryAtts.SetMultiColor(1, (0, 255, 0, 255))
FilledBoundaryAtts.boundaryNames = ("mat_toto", "mat_toto_0")
FilledBoundaryAtts.opacity = 1
FilledBoundaryAtts.wireframe = 0
FilledBoundaryAtts.drawInternal = 0
FilledBoundaryAtts.smoothingLevel = 0
FilledBoundaryAtts.cleanZonesOnly = 0
FilledBoundaryAtts.mixedColor = (255, 255, 255, 255)
FilledBoundaryAtts.pointSize = 0.05
FilledBoundaryAtts.pointType = FilledBoundaryAtts.Point  # Box, Axis, Icosahedron, Octahedron, Tetrahedron, SphereGeometry, Point, Sphere
FilledBoundaryAtts.pointSizeVarEnabled = 0
FilledBoundaryAtts.pointSizeVar = "default"
FilledBoundaryAtts.pointSizePixels = 2
SetPlotOptions(FilledBoundaryAtts)

DrawPlots()
SaveWindowAtts = SaveWindowAttributes()
SaveWindowAtts.outputToCurrentDirectory = 1
SaveWindowAtts.outputDirectory = "."
SaveWindowAtts.fileName = "a3dcurvf"
SaveWindowAtts.family = 1
SaveWindowAtts.format = SaveWindowAtts.PNG  # BMP, CURVE, JPEG, OBJ, PNG, POSTSCRIPT, POVRAY, PPM, RGB, STL, TIFF, ULTRA, VTK, PLY
SaveWindowAtts.width = 1024
SaveWindowAtts.height = 1024
SaveWindowAtts.screenCapture = 0
SaveWindowAtts.saveTiled = 0
SaveWindowAtts.quality = 80
SaveWindowAtts.progressive = 0
SaveWindowAtts.binary = 0
SaveWindowAtts.stereo = 0
SaveWindowAtts.compression = SaveWindowAtts.PackBits  # None, PackBits, Jpeg, Deflate
SaveWindowAtts.forceMerge = 0
SaveWindowAtts.resConstraint = SaveWindowAtts.ScreenProportions  # NoConstraint, EqualWidthHeight, ScreenProportions
SaveWindowAtts.advancedMultiWindowSave = 0
SetSaveWindowAttributes(SaveWindowAtts)
Test("Curve3D_00", SaveWindowAtts)

DeleteAllPlots()
CloseDatabase(data_path("Curve3D_test_data/a3dcurvf.curve3d"))

OpenDatabase(data_path("Curve3D_test_data/a3dcurve.curve3d"), 0, "Curve3D_1.0")
AddPlot("FilledBoundary", "segments", 1, 1)
# Begin spontaneous state
View3DAtts = View3DAttributes()
View3DAtts.viewNormal = (-0.960174, -0.252458, 0.119713)
View3DAtts.focus = (2.5, 1.5, 0)
View3DAtts.viewUp = (-0.254446, 0.613076, -0.747927)
View3DAtts.viewAngle = 30
View3DAtts.parallelScale = 3.08221
View3DAtts.nearPlane = -6.16441
View3DAtts.farPlane = 6.16441
View3DAtts.imagePan = (-0.178886, 0.187104)
View3DAtts.imageZoom = 1
View3DAtts.perspective = 1
View3DAtts.eyeAngle = 2
View3DAtts.centerOfRotationSet = 0
View3DAtts.centerOfRotation = (2.5, 1.5, 0)
View3DAtts.axis3DScaleFlag = 0
View3DAtts.axis3DScales = (1, 1, 1)
View3DAtts.shear = (0, 0, 1)
View3DAtts.windowValid = 1
SetView3D(View3DAtts)
# End spontaneous state

FilledBoundaryAtts = FilledBoundaryAttributes()
FilledBoundaryAtts.colorType = FilledBoundaryAtts.ColorByMultipleColors  # ColorBySingleColor, ColorByMultipleColors, ColorByColorTable
FilledBoundaryAtts.colorTableName = "Default"
FilledBoundaryAtts.invertColorTable = 0
FilledBoundaryAtts.legendFlag = 1
FilledBoundaryAtts.lineWidth = 2
FilledBoundaryAtts.singleColor = (0, 0, 0, 255)
FilledBoundaryAtts.SetMultiColor(0, (255, 0, 0, 255))
FilledBoundaryAtts.SetMultiColor(1, (0, 255, 0, 255))
FilledBoundaryAtts.boundaryNames = ("mat_tata", "mat_tutu", "mat_tutu_0", "mat_tutu_1")
FilledBoundaryAtts.opacity = 1
FilledBoundaryAtts.wireframe = 0
FilledBoundaryAtts.drawInternal = 0
FilledBoundaryAtts.smoothingLevel = 0
FilledBoundaryAtts.cleanZonesOnly = 0
FilledBoundaryAtts.mixedColor = (255, 255, 255, 255)
FilledBoundaryAtts.pointSize = 0.05
FilledBoundaryAtts.pointType = FilledBoundaryAtts.Point  # Box, Axis, Icosahedron, Octahedron, Tetrahedron, SphereGeometry, Point, Sphere
FilledBoundaryAtts.pointSizeVarEnabled = 0
FilledBoundaryAtts.pointSizeVar = "default"
FilledBoundaryAtts.pointSizePixels = 2
SetPlotOptions(FilledBoundaryAtts)

DrawPlots()
SaveWindowAtts = SaveWindowAttributes()
SaveWindowAtts.outputToCurrentDirectory = 1
SaveWindowAtts.outputDirectory = "."
SaveWindowAtts.fileName = "a3dcurve"
SaveWindowAtts.family = 1
SaveWindowAtts.format = SaveWindowAtts.PNG  # BMP, CURVE, JPEG, OBJ, PNG, POSTSCRIPT, POVRAY, PPM, RGB, STL, TIFF, ULTRA, VTK, PLY
SaveWindowAtts.width = 1024
SaveWindowAtts.height = 1024
SaveWindowAtts.screenCapture = 0
SaveWindowAtts.saveTiled = 0
SaveWindowAtts.quality = 80
SaveWindowAtts.progressive = 0
SaveWindowAtts.binary = 0
SaveWindowAtts.stereo = 0
SaveWindowAtts.compression = SaveWindowAtts.PackBits  # None, PackBits, Jpeg, Deflate
SaveWindowAtts.forceMerge = 0
SaveWindowAtts.resConstraint = SaveWindowAtts.ScreenProportions  # NoConstraint, EqualWidthHeight, ScreenProportions
SaveWindowAtts.advancedMultiWindowSave = 0
SetSaveWindowAttributes(SaveWindowAtts)
Test("Curve3D_01", SaveWindowAtts)

DeleteAllPlots()
CloseDatabase(data_path("Curve3D_test_data/a3dcurve.curve3d"))

Exit()
