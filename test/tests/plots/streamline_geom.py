# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  streamline_geom.py
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
StreamlineAtts.maxProcessCount = 10
StreamlineAtts.maxDomainCacheSize = 3
StreamlineAtts.workGroupSize = 32
StreamlineAtts.coloringMethod = StreamlineAtts.ColorByTime

# test defaults
SetPlotOptions(StreamlineAtts)
DrawPlots()
Test( "streamline_geom_01" )

StreamlineAtts.seedRadiusBBox = 0.03
SetPlotOptions(StreamlineAtts)
Test( "streamline_geom_02" )

StreamlineAtts.seedRadiusAbsolute = 0.6
StreamlineAtts.seedRadiusSizeType = StreamlineAtts.Absolute
SetPlotOptions(StreamlineAtts)
Test( "streamline_geom_03" )

StreamlineAtts.displayMethod = StreamlineAtts.Tubes
SetPlotOptions(StreamlineAtts)
Test( "streamline_geom_04" )

StreamlineAtts.tubeRadiusBBox = 0.01
SetPlotOptions(StreamlineAtts)
Test( "streamline_geom_05" )

StreamlineAtts.tubeRadiusAbsolute = 0.2
StreamlineAtts.tubeSizeType = StreamlineAtts.Absolute
SetPlotOptions(StreamlineAtts)
Test( "streamline_geom_06" )

StreamlineAtts.displayMethod = StreamlineAtts.Ribbons
SetPlotOptions(StreamlineAtts)
Test( "streamline_geom_07" )

StreamlineAtts.ribbonWidthBBox = 0.02
SetPlotOptions(StreamlineAtts)
Test( "streamline_geom_08" )

StreamlineAtts.ribbonWidthAbsolute = 0.4
StreamlineAtts.ribbonWidthSizeType = StreamlineAtts.Absolute
SetPlotOptions(StreamlineAtts)
Test( "streamline_geom_09" )

StreamlineAtts.displayMethod = StreamlineAtts.Lines
StreamlineAtts.showHeads = 1
SetPlotOptions(StreamlineAtts)
Test( "streamline_geom_10" )

StreamlineAtts.headRadiusBBox = 0.04
SetPlotOptions(StreamlineAtts)
Test( "streamline_geom_11" )

StreamlineAtts.headRadiusAbsolute = 0.8
StreamlineAtts.headRadiusSizeType = StreamlineAtts.Absolute
SetPlotOptions(StreamlineAtts)
Test( "streamline_geom_12" )

StreamlineAtts.headDisplayType = StreamlineAtts.Cone
SetPlotOptions(StreamlineAtts)
Test( "streamline_geom_13" )

StreamlineAtts.headHeightRatio = 3
SetPlotOptions(StreamlineAtts)
Test( "streamline_geom_14" )

#
# Test crop w/ head (cone) location
#
DeleteAllPlots()
AddPlot("Streamline", "grad", 1, 0)

satts =  StreamlineAttributes()
satts.displayEndFlag  = 1
satts.showHeads       = 1
satts.headDisplayType = satts.Cone
satts.displayEnd      = 2
SetPlotOptions(satts)
DrawPlots()
Test( "streamline_geom_crop_head_1")

satts.displayEnd      = 5
SetPlotOptions(satts)
DrawPlots()
Test( "streamline_geom_crop_head_2")

satts.displayEnd      = 8
SetPlotOptions(satts)
DrawPlots()
Test( "streamline_geom_crop_head_3")

Exit()
