# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  ic_integration.py
#
#  Tests:      operator      - integralcurve
#
#  Defect ID:  -
#
#  Programmer: Allen Sanderson
#  Date:       March 10, 2016
#
#  Modifications:
#    Kathleen Biagas, Tue Sep 29 11:01:37 PDT 2020
#    Turn off IC warnings.
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

IntegralCurveAtts = IntegralCurveAttributes()
IntegralCurveAtts.sourceType = IntegralCurveAtts.SpecifiedPlane  # SpecifiedPoint, SpecifiedPointList, SpecifiedLine, SpecifiedCircle, SpecifiedPlane, SpecifiedSphere, SpecifiedBox
IntegralCurveAtts.limitMaximumTimestep = 1
IntegralCurveAtts.maxTimeStep = 0.02
IntegralCurveAtts.terminateByTime = 1
IntegralCurveAtts.termTime = 200
IntegralCurveAtts.maxSteps = 100000
IntegralCurveAtts.planeOrigin = (0.5, 1, 0.5)
IntegralCurveAtts.planeNormal = (0, 1, 0)
IntegralCurveAtts.planeUpAxis = (1, 0, 0)
IntegralCurveAtts.sampleDensity0 = 5
IntegralCurveAtts.sampleDensity1 = 5
IntegralCurveAtts.sampleDistance0 = 18
IntegralCurveAtts.sampleDistance1 = 18
IntegralCurveAtts.integrationDirection = IntegralCurveAtts.Both  # Forward, Backward, Both
IntegralCurveAtts.relTol = 1e-06
IntegralCurveAtts.absTolSizeType = IntegralCurveAtts.Absolute
IntegralCurveAtts.absTolAbsolute = 1e-07
IntegralCurveAtts.integrationType = IntegralCurveAtts.DormandPrince  # DormandPrince, AdamsBashforth, M3DC1Integrator
IntegralCurveAtts.maxProcessCount = 10
IntegralCurveAtts.maxDomainCacheSize = 3
IntegralCurveAtts.workGroupSize = 32
IntegralCurveAtts.dataValue = IntegralCurveAtts.TimeAbsolute
# turn off warnings
IntegralCurveAtts.issueAdvectionWarnings = 0
IntegralCurveAtts.issueBoundaryWarnings = 0
IntegralCurveAtts.issueTerminationWarnings = 0
IntegralCurveAtts.issueStepsizeWarnings = 0
IntegralCurveAtts.issueStiffnessWarnings = 0
IntegralCurveAtts.issueCriticalPointsWarnings = 0


# test defaults
SetOperatorOptions(IntegralCurveAtts)
DrawPlots()
Test( "ic_integration_01" )

IntegralCurveAtts.maxTimeStep = 0.05
SetOperatorOptions(IntegralCurveAtts)
Test( "ic_integration_02" )

IntegralCurveAtts.limitMaximumTimestep = 0
SetOperatorOptions(IntegralCurveAtts)
Test( "ic_integration_03" )

IntegralCurveAtts.absTolAbsolute = 1e-03
SetOperatorOptions(IntegralCurveAtts)
Test( "ic_integration_04" )

IntegralCurveAtts.absTolSizeType = IntegralCurveAtts.FractionOfBBox
SetOperatorOptions(IntegralCurveAtts)
Test( "ic_integration_05" )

IntegralCurveAtts.absTolBBox = 1e-09
SetOperatorOptions(IntegralCurveAtts)
Test( "ic_integration_06" )

IntegralCurveAtts.absTolBBox = 1e-04
SetOperatorOptions(IntegralCurveAtts)
Test( "ic_integration_07" )

IntegralCurveAtts.integrationType = IntegralCurveAtts.AdamsBashforth
SetOperatorOptions(IntegralCurveAtts)
Test( "ic_integration_08" )

IntegralCurveAtts.maxStepLength = 0.2
SetOperatorOptions(IntegralCurveAtts)
Test( "ic_integration_09" )

IntegralCurveAtts.maxStepLength = 1.0
SetOperatorOptions(IntegralCurveAtts)
Test( "ic_integration_10" )

IntegralCurveAtts.maxStepLength = 0.01
SetOperatorOptions(IntegralCurveAtts)
Test( "ic_integration_11" )

Exit()
