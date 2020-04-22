# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  ic_streamlines.py
#
#  Tests:      operator      - integralcurve
#
#  Defect ID:  -
#
#  Programmer: Allen Sanderson
#  Date:       March 10, 2016
#
# ----------------------------------------------------------------------------

OpenDatabase(silo_data_path("noise.silo"))

AddPlot("Pseudocolor", "operators/IntegralCurve/grad", 1, 0)

View3DAtts = View3DAttributes()
View3DAtts.viewNormal = (0.270729, 0.624198, 0.732859)
View3DAtts.focus = (0.496062, 0.99603, 0.496062)
View3DAtts.viewUp = (-0.0922782, 0.774611, -0.62567)
View3DAtts.viewAngle = 30
View3DAtts.parallelScale = 12.1829
View3DAtts.nearPlane = -24.3658
View3DAtts.farPlane = 24.3658
View3DAtts.imagePan = (0, 0)
View3DAtts.imageZoom = 1
View3DAtts.perspective = 1
View3DAtts.eyeAngle = 2
View3DAtts.axis3DScaleFlag = 0
View3DAtts.axis3DScales = (1, 1, 1)
SetView3D(View3DAtts)

PseudocolorAtts = PseudocolorAttributes()
PseudocolorAtts.scaling = PseudocolorAtts.Linear  # Linear, Log, Skew
PseudocolorAtts.skewFactor = 1
PseudocolorAtts.limitsMode = PseudocolorAtts.OriginalData  # OriginalData, CurrentPlot
PseudocolorAtts.minFlag = 0
PseudocolorAtts.min = 0
PseudocolorAtts.maxFlag = 0
PseudocolorAtts.max = 1
PseudocolorAtts.centering = PseudocolorAtts.Natural  # Natural, Nodal, Zonal
PseudocolorAtts.colorTableName = "hot"
PseudocolorAtts.invertColorTable = 0
PseudocolorAtts.opacityType = PseudocolorAtts.FullyOpaque  # ColorTable, FullyOpaque, Constant, Ramp, VariableRange
PseudocolorAtts.opacityVariable = ""
PseudocolorAtts.opacity = 1
PseudocolorAtts.opacityVarMin = 0
PseudocolorAtts.opacityVarMax = 1
PseudocolorAtts.opacityVarMinFlag = 0
PseudocolorAtts.opacityVarMaxFlag = 0
PseudocolorAtts.pointSize = 0.05
PseudocolorAtts.pointType = PseudocolorAtts.Point  # Box, Axis, Icosahedron, Octahedron, Tetrahedron, SphereGeometry, Point, Sphere
PseudocolorAtts.pointSizeVarEnabled = 0
PseudocolorAtts.pointSizeVar = "default"
PseudocolorAtts.pointSizePixels = 2
PseudocolorAtts.lineType = PseudocolorAtts.Tube  # Line, Tube, Ribbon
PseudocolorAtts.lineWidth = 0
PseudocolorAtts.tubeResolution = 100
PseudocolorAtts.tubeRadiusSizeType = PseudocolorAtts.Absolute  # Absolute, FractionOfBBox
PseudocolorAtts.tubeRadiusAbsolute = 0.25
PseudocolorAtts.tubeRadiusBBox = 0.005
PseudocolorAtts.tubeRadiusVarEnabled = 0
PseudocolorAtts.tubeRadiusVar = ""
PseudocolorAtts.tubeRadiusVarRatio = 10
PseudocolorAtts.tailStyle = PseudocolorAtts.Spheres  # None, Spheres, Cones
PseudocolorAtts.headStyle = PseudocolorAtts.None  # None, Spheres, Cones
PseudocolorAtts.endPointRadiusSizeType = PseudocolorAtts.Absolute  # Absolute, FractionOfBBox
PseudocolorAtts.endPointRadiusAbsolute = 0.8
PseudocolorAtts.endPointRadiusBBox = 0.005
PseudocolorAtts.endPointResolution = 100
PseudocolorAtts.endPointRatio = 2
PseudocolorAtts.endPointRadiusVarEnabled = 0
PseudocolorAtts.endPointRadiusVar = ""
PseudocolorAtts.endPointRadiusVarRatio = 10
PseudocolorAtts.renderSurfaces = 1
PseudocolorAtts.renderWireframe = 0
PseudocolorAtts.renderPoints = 0
PseudocolorAtts.smoothingLevel = 0
PseudocolorAtts.legendFlag = 1
PseudocolorAtts.lightingFlag = 1
PseudocolorAtts.wireframeColor = (0, 0, 0, 0)
PseudocolorAtts.pointColor = (0, 0, 0, 0)
SetPlotOptions(PseudocolorAtts)

IntegralCurveAtts = IntegralCurveAttributes()
IntegralCurveAtts.sourceType = IntegralCurveAtts.SpecifiedPlane  # SpecifiedPoint, PointList, SpecifiedLine, Circle, SpecifiedPlane, SpecifiedSphere, SpecifiedBox, Selection, FieldData
IntegralCurveAtts.pointSource = (0, 0, 0)
IntegralCurveAtts.lineStart = (0, 0, 0)
IntegralCurveAtts.lineEnd = (1, 0, 0)
IntegralCurveAtts.planeOrigin = (0.5, 1, 0.5)
IntegralCurveAtts.planeNormal = (0, 1, 0)
IntegralCurveAtts.planeUpAxis = (1, 0, 0)
IntegralCurveAtts.radius = 1
IntegralCurveAtts.sphereOrigin = (0, 0, 0)
IntegralCurveAtts.boxExtents = (0, 1, 0, 1, 0, 1)
IntegralCurveAtts.useWholeBox = 1
IntegralCurveAtts.pointList = (0, 0, 0, 1, 0, 0, 0, 1, 0)
IntegralCurveAtts.fieldData = ()
IntegralCurveAtts.sampleDensity0 = 5
IntegralCurveAtts.sampleDensity1 = 5
IntegralCurveAtts.sampleDensity2 = 2
IntegralCurveAtts.dataValue = IntegralCurveAtts.Speed  # Solid, SeedPointID, Speed, Vorticity, ArcLength, TimeAbsolute, TimeRelative, AverageDistanceFromSeed, CorrelationDistance, Difference, Variable
IntegralCurveAtts.dataVariable = ""
IntegralCurveAtts.integrationDirection = IntegralCurveAtts.Both  # Forward, Backward, Both, ForwardDirectionless, BackwardDirectionless, BothDirectionless
IntegralCurveAtts.maxSteps = 100000
IntegralCurveAtts.terminateByDistance = 0
IntegralCurveAtts.termDistance = 10
IntegralCurveAtts.terminateByTime = 1
IntegralCurveAtts.termTime = 200
IntegralCurveAtts.maxStepLength = 0.1
IntegralCurveAtts.limitMaximumTimestep = 1
IntegralCurveAtts.maxTimeStep = 0.02
IntegralCurveAtts.relTol = 1e-06
IntegralCurveAtts.absTolSizeType = IntegralCurveAtts.FractionOfBBox  # Absolute, FractionOfBBox
IntegralCurveAtts.absTolAbsolute = 1e-06
IntegralCurveAtts.absTolBBox = 1e-07
IntegralCurveAtts.fieldType = IntegralCurveAtts.Default  # Default, FlashField, M3DC12DField, M3DC13DField, Nek5000Field, NektarPPField
IntegralCurveAtts.fieldConstant = 1
IntegralCurveAtts.velocitySource = (0, 0, 0)
IntegralCurveAtts.integrationType = IntegralCurveAtts.DormandPrince  # Euler, Leapfrog, DormandPrince, AdamsBashforth, RK4, M3DC12DIntegrator
IntegralCurveAtts.parallelizationAlgorithmType = IntegralCurveAtts.VisItSelects  # LoadOnDemand, ParallelStaticDomains, MasterSlave, VisItSelects
IntegralCurveAtts.maxProcessCount = 10
IntegralCurveAtts.maxDomainCacheSize = 3
IntegralCurveAtts.workGroupSize = 32
IntegralCurveAtts.pathlines = 0
IntegralCurveAtts.pathlinesOverrideStartingTimeFlag = 0
IntegralCurveAtts.pathlinesOverrideStartingTime = 0
IntegralCurveAtts.pathlinesPeriod = 0
IntegralCurveAtts.pathlinesCMFE = IntegralCurveAtts.POS_CMFE  # CONN_CMFE, POS_CMFE
IntegralCurveAtts.displayGeometry = IntegralCurveAtts.Lines  # Lines, Tubes, Ribbons
IntegralCurveAtts.cropBeginFlag = 0
IntegralCurveAtts.cropBegin = 0
IntegralCurveAtts.cropEndFlag = 0
IntegralCurveAtts.cropEnd = 0
IntegralCurveAtts.cropValue = IntegralCurveAtts.Time  # Distance, Time, StepNumber
IntegralCurveAtts.sampleDistance0 = 18
IntegralCurveAtts.sampleDistance1 = 18
IntegralCurveAtts.sampleDistance2 = 10
IntegralCurveAtts.fillInterior = 1
IntegralCurveAtts.randomSamples = 0
IntegralCurveAtts.randomSeed = 0
IntegralCurveAtts.numberOfRandomSamples = 1
IntegralCurveAtts.issueAdvectionWarnings = 1
IntegralCurveAtts.issueBoundaryWarnings = 1
IntegralCurveAtts.issueTerminationWarnings = 1
IntegralCurveAtts.issueStepsizeWarnings = 1
IntegralCurveAtts.issueStiffnessWarnings = 1
IntegralCurveAtts.issueCriticalPointsWarnings = 1
IntegralCurveAtts.criticalPointThreshold = 0.001
IntegralCurveAtts.correlationDistanceAngTol = 5
IntegralCurveAtts.correlationDistanceMinDistAbsolute = 1
IntegralCurveAtts.correlationDistanceMinDistBBox = 0.005
IntegralCurveAtts.correlationDistanceMinDistType = IntegralCurveAtts.FractionOfBBox  # Absolute, FractionOfBBox
IntegralCurveAtts.selection = ""

# Test coloring options

IntegralCurveAtts.dataValue = IntegralCurveAtts.Speed
SetOperatorOptions(IntegralCurveAtts)
DrawPlots()
Test( "ic_streamlines_01" )

IntegralCurveAtts.dataValue = IntegralCurveAtts.Vorticity
SetOperatorOptions(IntegralCurveAtts)
Test( "ic_streamlines_02" )

IntegralCurveAtts.dataValue = IntegralCurveAtts.ArcLength
SetOperatorOptions(IntegralCurveAtts)
Test( "ic_streamlines_03" )

IntegralCurveAtts.dataValue = IntegralCurveAtts.TimeAbsolute
SetOperatorOptions(IntegralCurveAtts)
Test( "ic_streamlines_04" )

IntegralCurveAtts.dataValue = IntegralCurveAtts.SeedPointID
SetOperatorOptions(IntegralCurveAtts)
Test( "ic_streamlines_05" )

IntegralCurveAtts.dataValue = IntegralCurveAtts.Variable 
IntegralCurveAtts.dataVariable = "hardyglobal"
SetOperatorOptions(IntegralCurveAtts)
Test( "ic_streamlines_06" )

IntegralCurveAtts.dataValue = IntegralCurveAtts.Solid
SetOperatorOptions(IntegralCurveAtts)
Test( "ic_streamlines_07" )

# test termination modes (termination by time implied in previous tests)

IntegralCurveAtts.dataValue = IntegralCurveAtts.ArcLength

IntegralCurveAtts.terminateByTime = 0
IntegralCurveAtts.terminateByDistance = 1
IntegralCurveAtts.termDistance = 10
SetOperatorOptions(IntegralCurveAtts)
Test( "ic_streamlines_08" )

IntegralCurveAtts.termDistance = 5
SetOperatorOptions(IntegralCurveAtts)
Test( "ic_streamlines_09" )

IntegralCurveAtts.terminateByDistance = 0
IntegralCurveAtts.maxSteps = 500
SetOperatorOptions(IntegralCurveAtts)
Test( "ic_streamlines_10" )

IntegralCurveAtts.maxSteps = 250
SetOperatorOptions(IntegralCurveAtts)
Test( "ic_streamlines_11" )

Exit()
