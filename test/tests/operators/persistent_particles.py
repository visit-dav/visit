# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  persistent_particles.py
#
#  Programmer: Hank Childs
#  Date:       October 15, 2010
#
# ----------------------------------------------------------------------------

OpenDatabase(data_path("synergia_test_data/vis_particles_* database"),0, "Vs")

AddPlot("Pseudocolor", "particles_6")
AddOperator("Threshold")
ThresholdAtts = ThresholdAttributes()
ThresholdAtts.outputMeshType = 0
ThresholdAtts.listedVarNames = ("default")
ThresholdAtts.zonePortions = (0)
ThresholdAtts.lowerBounds = (-1e+37)
ThresholdAtts.upperBounds = (20)
ThresholdAtts.defaultVarName = "particles_6"
ThresholdAtts.defaultVarIsScalar = 1
SetOperatorOptions(ThresholdAtts)
DrawPlots()

View3DAtts = View3DAttributes()
View3DAtts.viewNormal = (0.282153, 0.56578, 0.774779)
View3DAtts.focus = (-0.000283548, -1.60497e-05, -0.00167359)
View3DAtts.viewUp = (-0.173662, 0.824371, -0.538752)
View3DAtts.viewAngle = 30
View3DAtts.parallelScale = 0.0560412
View3DAtts.nearPlane = -0.112082
View3DAtts.farPlane = 0.112082
View3DAtts.imagePan = (0, 0)
View3DAtts.imageZoom = 3
View3DAtts.perspective = 1
View3DAtts.eyeAngle = 2
View3DAtts.centerOfRotationSet = 0
View3DAtts.centerOfRotation = (-0.000283548, -1.60497e-05, -0.00167359)
View3DAtts.axis3DScaleFlag = 0
View3DAtts.axis3DScales = (1, 1, 1)
View3DAtts.shear = (0, 0, 1)
SetView3D(View3DAtts)

Test("persistent_particles_01")

CreateNamedSelection("selection1")
a = GetAnnotationAttributes()
SetWindowLayout(2)
SetActiveWindow(2)
SetAnnotationAttributes(a)
AddPlot("Pseudocolor", "particles_6", 1, 1)
silr = SILRestriction()
silr.TurnOnAll()
SetPlotSILRestriction(silr ,1)
ApplyNamedSelection("selection1")
DrawPlots()
SetView3D(View3DAtts)
Test("persistent_particles_02")

AddOperator("PersistentParticles", 1)
PersistentParticlesAtts = PersistentParticlesAttributes()
PersistentParticlesAtts.startIndex = 0
PersistentParticlesAtts.stopIndex = 5
PersistentParticlesAtts.stride = 1
PersistentParticlesAtts.startPathType = PersistentParticlesAtts.Absolute  # Absolute, Relative
PersistentParticlesAtts.stopPathType = PersistentParticlesAtts.Absolute  # Absolute, Relative
PersistentParticlesAtts.traceVariableX = "default"
PersistentParticlesAtts.traceVariableY = "default"
PersistentParticlesAtts.traceVariableZ = "default"
PersistentParticlesAtts.connectParticles = 0
PersistentParticlesAtts.indexVariable = "default"
SetOperatorOptions(PersistentParticlesAtts, 1)
DrawPlots()
Test("persistent_particles_03")

PersistentParticlesAtts.connectParticles = 1
SetOperatorOptions(PersistentParticlesAtts, 1)
Test("persistent_particles_04")

AddOperator("Tube", 1)
TubeAtts = TubeAttributes()
TubeAtts.scaleByVarFlag = 0
TubeAtts.tubeRadiusType = TubeAtts.Absolute
TubeAtts.radiusAbsolute = 0.0005
TubeAtts.scaleVariable = "default"
TubeAtts.fineness = 10
TubeAtts.capping = 0
SetOperatorOptions(TubeAtts, 1)
DrawPlots()
Test("persistent_particles_05")


AddPlot("Pseudocolor", "particles_6", 1, 0)
ApplyNamedSelection("selection1")
PseudocolorAtts = PseudocolorAttributes()
PseudocolorAtts.legendFlag = 1
PseudocolorAtts.lightingFlag = 1
PseudocolorAtts.minFlag = 0
PseudocolorAtts.maxFlag = 0
PseudocolorAtts.centering = PseudocolorAtts.Natural  # Natural, Nodal, Zonal
PseudocolorAtts.scaling = PseudocolorAtts.Linear  # Linear, Log, Skew
PseudocolorAtts.limitsMode = PseudocolorAtts.OriginalData  # OriginalData, CurrentPlot
PseudocolorAtts.min = 0
PseudocolorAtts.max = 1
PseudocolorAtts.pointSize = 0.001
PseudocolorAtts.pointType = PseudocolorAtts.Icosahedron  # Box, Axis, Icosahedron, Point, Sphere
PseudocolorAtts.skewFactor = 1
PseudocolorAtts.opacity = 1
PseudocolorAtts.colorTableName = "orangehot"
PseudocolorAtts.smoothingLevel = 0
PseudocolorAtts.pointSizeVarEnabled = 0
PseudocolorAtts.pointSizeVar = "default"
PseudocolorAtts.pointSizePixels = 2
PseudocolorAtts.lineStyle = PseudocolorAtts.SOLID  # SOLID, DASH, DOT, DOTDASH
PseudocolorAtts.lineWidth = 0
PseudocolorAtts.opacityType = PseudocolorAtts.FullyOpaque
SetPlotOptions(PseudocolorAtts)
PseudocolorAtts = PseudocolorAttributes()
PseudocolorAtts.legendFlag = 1
PseudocolorAtts.lightingFlag = 1
PseudocolorAtts.minFlag = 0
PseudocolorAtts.maxFlag = 1
PseudocolorAtts.centering = PseudocolorAtts.Natural  # Natural, Nodal, Zonal
PseudocolorAtts.scaling = PseudocolorAtts.Linear  # Linear, Log, Skew
PseudocolorAtts.limitsMode = PseudocolorAtts.OriginalData  # OriginalData, CurrentPlot
PseudocolorAtts.min = 0
PseudocolorAtts.max = 20
PseudocolorAtts.pointSize = 0.001
PseudocolorAtts.pointType = PseudocolorAtts.Icosahedron  # Box, Axis, Icosahedron, Point, Sphere
PseudocolorAtts.skewFactor = 1
PseudocolorAtts.opacity = 1
PseudocolorAtts.colorTableName = "orangehot"
PseudocolorAtts.smoothingLevel = 0
PseudocolorAtts.pointSizeVarEnabled = 0
PseudocolorAtts.pointSizeVar = "default"
PseudocolorAtts.pointSizePixels = 2
PseudocolorAtts.lineStyle = PseudocolorAtts.SOLID  # SOLID, DASH, DOT, DOTDASH
PseudocolorAtts.lineWidth = 0
PseudocolorAtts.opacityType = PseudocolorAtts.FullyOpaque
SetPlotOptions(PseudocolorAtts)
SetActivePlots((0, 1))
SetActivePlots(0)
PseudocolorAtts = PseudocolorAttributes()
PseudocolorAtts.legendFlag = 1
PseudocolorAtts.lightingFlag = 1
PseudocolorAtts.minFlag = 0
PseudocolorAtts.maxFlag = 1
PseudocolorAtts.centering = PseudocolorAtts.Natural  # Natural, Nodal, Zonal
PseudocolorAtts.scaling = PseudocolorAtts.Linear  # Linear, Log, Skew
PseudocolorAtts.limitsMode = PseudocolorAtts.OriginalData  # OriginalData, CurrentPlot
PseudocolorAtts.min = 0
PseudocolorAtts.max = 20
PseudocolorAtts.pointSize = 0.05
PseudocolorAtts.pointType = PseudocolorAtts.Point  # Box, Axis, Icosahedron, Point, Sphere
PseudocolorAtts.skewFactor = 1
PseudocolorAtts.opacity = 1
PseudocolorAtts.colorTableName = "orangehot"
PseudocolorAtts.smoothingLevel = 0
PseudocolorAtts.pointSizeVarEnabled = 0
PseudocolorAtts.pointSizeVar = "default"
PseudocolorAtts.pointSizePixels = 2
PseudocolorAtts.lineStyle = PseudocolorAtts.SOLID  # SOLID, DASH, DOT, DOTDASH
PseudocolorAtts.lineWidth = 0
PseudocolorAtts.opacityType = PseudocolorAtts.FullyOpaque
SetPlotOptions(PseudocolorAtts)
DrawPlots()
Test("persistent_particles_06")


Exit()
