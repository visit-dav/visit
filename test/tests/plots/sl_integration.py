# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  sl_integration.py
#
#  Defect ID:  -
#
#  Programmer: Hank Childs
#  Date:       October 2, 2010
#
# ----------------------------------------------------------------------------

OpenDatabase(silo_data_path("noise.silo"))


AddPlot("Streamline", "grad", 1, 0)

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

StreamlineAtts = StreamlineAttributes()
StreamlineAtts.sourceType = StreamlineAtts.SpecifiedPlane  # SpecifiedPoint, SpecifiedPointList, SpecifiedLine, SpecifiedCircle, SpecifiedPlane, SpecifiedSphere, SpecifiedBox
StreamlineAtts.limitMaximumTimestep = 1
StreamlineAtts.maxTimeStep = 0.02
StreamlineAtts.terminateByTime = 1
StreamlineAtts.termTime = 200
StreamlineAtts.maxSteps = 100000
StreamlineAtts.planeOrigin = (0.5, 1, 0.5)
StreamlineAtts.planeNormal = (0, 1, 0)
StreamlineAtts.planeUpAxis = (1, 0, 0)
StreamlineAtts.sampleDensity0 = 5
StreamlineAtts.sampleDensity1 = 5
StreamlineAtts.sampleDistance0 = 18
StreamlineAtts.sampleDistance1 = 18
StreamlineAtts.displayMethod = StreamlineAtts.Tubes  # Lines, Tubes, Ribbons
StreamlineAtts.integrationDirection = StreamlineAtts.Both  # Forward, Backward, Both
StreamlineAtts.relTol = 1e-06
StreamlineAtts.absTolSizeType = StreamlineAtts.Absolute
StreamlineAtts.absTolAbsolute = 1e-07
StreamlineAtts.integrationType = StreamlineAtts.DormandPrince  # DormandPrince, AdamsBashforth, M3DC1Integrator
StreamlineAtts.maxStreamlineProcessCount = 10
StreamlineAtts.maxDomainCacheSize = 3
StreamlineAtts.workGroupSize = 32
StreamlineAtts.coloringMethod = StreamlineAtts.ColorByTime

# test defaults
SetPlotOptions(StreamlineAtts)
DrawPlots()
Test( "sl_integration_01" )

StreamlineAtts.maxTimeStep = 0.05
SetPlotOptions(StreamlineAtts)
Test( "sl_integration_02" )

StreamlineAtts.limitMaximumTimestep = 0
SetPlotOptions(StreamlineAtts)
Test( "sl_integration_03" )

StreamlineAtts.absTolAbsolute = 1e-03
SetPlotOptions(StreamlineAtts)
Test( "sl_integration_04" )

StreamlineAtts.absTolSizeType = StreamlineAtts.FractionOfBBox
SetPlotOptions(StreamlineAtts)
Test( "sl_integration_05" )

StreamlineAtts.absTolBBox = 1e-09
SetPlotOptions(StreamlineAtts)
Test( "sl_integration_06" )

StreamlineAtts.absTolBBox = 1e-04
SetPlotOptions(StreamlineAtts)
Test( "sl_integration_07" )

StreamlineAtts.integrationType = StreamlineAtts.AdamsBashforth
SetPlotOptions(StreamlineAtts)
Test( "sl_integration_08" )

StreamlineAtts.maxStepLength = 0.2
SetPlotOptions(StreamlineAtts)
Test( "sl_integration_09" )

StreamlineAtts.maxStepLength = 1.0
SetPlotOptions(StreamlineAtts)
Test( "sl_integration_10" )

StreamlineAtts.maxStepLength = 0.01
SetPlotOptions(StreamlineAtts)
Test( "sl_integration_11" )

Exit()
