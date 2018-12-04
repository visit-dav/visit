# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  ic_termination.py
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

IntegralCurveAtts = IntegralCurveAttributes()
IntegralCurveAtts.sourceType = IntegralCurveAtts.SpecifiedPlane  # SpecifiedPoint, SpecifiedPointList, SpecifiedLine, SpecifiedCircle, SpecifiedPlane, SpecifiedSphere, SpecifiedBox
IntegralCurveAtts.terminateByTime = 1
IntegralCurveAtts.termTime = 200
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

# test defaults
SetOperatorOptions(IntegralCurveAtts)
DrawPlots()
Test( "ic_termination_01" )

IntegralCurveAtts.termTime = 50
SetOperatorOptions(IntegralCurveAtts)
Test( "ic_termination_02" )

IntegralCurveAtts.terminateByDistance = 1
IntegralCurveAtts.termDistance = 4
SetOperatorOptions(IntegralCurveAtts)
Test( "ic_termination_03" )

IntegralCurveAtts.terminateByDistance = 1
IntegralCurveAtts.termDistance = 12
SetOperatorOptions(IntegralCurveAtts)
Test( "ic_termination_04" )

IntegralCurveAtts.terminateByTime = 0
SetOperatorOptions(IntegralCurveAtts)
Test( "ic_termination_05" )

IntegralCurveAtts.maxSteps = 40
SetOperatorOptions(IntegralCurveAtts)
Test( "ic_termination_06" )

IntegralCurveAtts.termDistance = 25
SetOperatorOptions(IntegralCurveAtts)
Test( "ic_termination_07" )

IntegralCurveAtts.maxSteps = 10000
SetOperatorOptions(IntegralCurveAtts)
Test( "ic_termination_08" )

Exit()
