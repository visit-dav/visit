# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  sl_termination.py
#
#  Defect ID:  -
#
#  Programmer: Hank Childs
#  Date:       October 9, 2010
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
StreamlineAtts.terminateByTime = 1
StreamlineAtts.termTime = 200
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
StreamlineAtts.maxProcessCount = 10
StreamlineAtts.maxDomainCacheSize = 3
StreamlineAtts.workGroupSize = 32
StreamlineAtts.coloringMethod = StreamlineAtts.ColorByTime

# test defaults
SetPlotOptions(StreamlineAtts)
DrawPlots()
Test( "sl_termination_01" )

StreamlineAtts.termTime = 50
SetPlotOptions(StreamlineAtts)
Test( "sl_termination_02" )

StreamlineAtts.terminateByDistance = 1
StreamlineAtts.termDistance = 4
SetPlotOptions(StreamlineAtts)
Test( "sl_termination_03" )

StreamlineAtts.terminateByDistance = 1
StreamlineAtts.termDistance = 12
SetPlotOptions(StreamlineAtts)
Test( "sl_termination_04" )

StreamlineAtts.terminateByTime = 0
SetPlotOptions(StreamlineAtts)
Test( "sl_termination_05" )

StreamlineAtts.maxSteps = 40
SetPlotOptions(StreamlineAtts)
Test( "sl_termination_06" )

StreamlineAtts.termDistance = 25
SetPlotOptions(StreamlineAtts)
Test( "sl_termination_07" )

StreamlineAtts.maxSteps = 10000
SetPlotOptions(StreamlineAtts)
Test( "sl_termination_08" )

Exit()
