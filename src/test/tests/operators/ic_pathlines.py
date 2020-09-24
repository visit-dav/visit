# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  ic_pathlines.py
#
#  Tests:      operator      - integralcurve
#
#  Defect ID:  -
#
#  Programmer: Allen Sanderson
#  Date:       March 10, 2016
#
#  Modifications:
#    Kathleen Biagas, Thu Sep 17 11:35:47 PDT 2020
#    Turn off IC warnings. Set larger linewidth so for better images.
# ----------------------------------------------------------------------------
RequiredDatabasePlugin("PICS_Tester")

OpenDatabase(data_path("pics_test_data/pathline_test.pics"))
AddPlot("Pseudocolor", "operators/IntegralCurve/velocity", 1, 0)
pcAtts = PseudocolorAttributes();
pcAtts.lineWidth = 3
SetPlotOptions(pcAtts)

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

IntegralCurveAtts = IntegralCurveAttributes()
IntegralCurveAtts.sourceType = IntegralCurveAtts.SpecifiedPoint
IntegralCurveAtts.pointSource = (0.5, 0.1, 0.1)
IntegralCurveAtts.lineStart = (0, 0, 0)
IntegralCurveAtts.lineEnd = (1, 0, 0)
IntegralCurveAtts.planeOrigin = (0, 0, 0)
IntegralCurveAtts.planeNormal = (0, 0, 1)
IntegralCurveAtts.planeUpAxis = (0, 1, 0)
IntegralCurveAtts.radius = 1
IntegralCurveAtts.sphereOrigin = (0, 0, 0)
IntegralCurveAtts.boxExtents = (0, 1, 0, 1, 0, 1)
IntegralCurveAtts.useWholeBox = 1
IntegralCurveAtts.pointList = (0, 0, 0, 1, 0, 0, 0, 1, 0)
IntegralCurveAtts.fieldData = ()
IntegralCurveAtts.sampleDensity0 = 2
IntegralCurveAtts.sampleDensity1 = 2
IntegralCurveAtts.sampleDensity2 = 2
IntegralCurveAtts.dataValue = IntegralCurveAtts.TimeAbsolute
IntegralCurveAtts.dataVariable = ""
IntegralCurveAtts.integrationDirection = IntegralCurveAtts.Forward
IntegralCurveAtts.maxSteps = 1000
IntegralCurveAtts.terminateByDistance = 0
IntegralCurveAtts.termDistance = 10
IntegralCurveAtts.terminateByTime = 0
IntegralCurveAtts.termTime = 10
IntegralCurveAtts.maxStepLength = 0.01
IntegralCurveAtts.limitMaximumTimestep = 0
IntegralCurveAtts.maxTimeStep = 0.1
IntegralCurveAtts.relTol = 0.0001
IntegralCurveAtts.absTolSizeType = IntegralCurveAtts.FractionOfBBox
IntegralCurveAtts.absTolAbsolute = 1e-06
IntegralCurveAtts.absTolBBox = 1e-06
IntegralCurveAtts.fieldType = IntegralCurveAtts.Default
IntegralCurveAtts.fieldConstant = 1
IntegralCurveAtts.velocitySource = (0, 0, 0)
IntegralCurveAtts.integrationType = IntegralCurveAtts.Euler
IntegralCurveAtts.parallelizationAlgorithmType = IntegralCurveAtts.VisItSelects
IntegralCurveAtts.maxProcessCount = 10
IntegralCurveAtts.maxDomainCacheSize = 3
IntegralCurveAtts.workGroupSize = 32
IntegralCurveAtts.pathlines = 1
IntegralCurveAtts.pathlinesOverrideStartingTimeFlag = 0
IntegralCurveAtts.pathlinesOverrideStartingTime = 0
IntegralCurveAtts.pathlinesPeriod = 0
IntegralCurveAtts.pathlinesCMFE = IntegralCurveAtts.CONN_CMFE
IntegralCurveAtts.displayGeometry = IntegralCurveAtts.Lines
IntegralCurveAtts.cropBeginFlag = 0
IntegralCurveAtts.cropBegin = 0
IntegralCurveAtts.cropEndFlag = 0
IntegralCurveAtts.cropEnd = 0
IntegralCurveAtts.cropValue = IntegralCurveAtts.Time
IntegralCurveAtts.sampleDistance0 = 10
IntegralCurveAtts.sampleDistance1 = 10
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
IntegralCurveAtts.correlationDistanceMinDistType = IntegralCurveAtts.FractionOfBBox
IntegralCurveAtts.selection = ""
IntegralCurveAtts.issueAdvectionWarnings = 0
IntegralCurveAtts.issueBoundaryWarnings = 0
IntegralCurveAtts.issueTerminationWarnings = 0
IntegralCurveAtts.issueStepsizeWarnings = 0
IntegralCurveAtts.issueStiffnessWarnings = 0
IntegralCurveAtts.issueCriticalPointsWarnings = 0

SetOperatorOptions(IntegralCurveAtts, 0)
DrawPlots()
Test("ic_pathlines_01")

IntegralCurveAtts.terminateByTime = 1
IntegralCurveAtts.termTime = 3
SetOperatorOptions(IntegralCurveAtts)
Test("ic_pathlines_02")

TimeSliderSetState(6)
e = GetLastError()
TestText("ic_pathlines_03", e)

IntegralCurveAtts.integrationDirection = IntegralCurveAtts.Backward
SetOperatorOptions(IntegralCurveAtts)
DrawPlots()
Test("ic_pathlines_04")

IntegralCurveAtts.terminateByTime = 0
SetOperatorOptions(IntegralCurveAtts)
Test("ic_pathlines_05")

IntegralCurveAtts.pathlinesOverrideStartingTimeFlag = 1
IntegralCurveAtts.pathlinesOverrideStartingTime = 3.5
SetOperatorOptions(IntegralCurveAtts)
Test("ic_pathlines_06")

TimeSliderSetState(0)
IntegralCurveAtts.pathlinesCMFE = IntegralCurveAtts.POS_CMFE
SetOperatorOptions(IntegralCurveAtts)
Test("ic_pathlines_07")

IntegralCurveAtts.pathlinesCMFE = IntegralCurveAtts.POS_CMFE
SetOperatorOptions(IntegralCurveAtts)
Test("ic_pathlines_08")

IntegralCurveAtts.pathlinesOverrideStartingTimeFlag = 0
SetOperatorOptions(IntegralCurveAtts)
e = GetLastError()
TestText("ic_pathlines_09", e)

Exit()
