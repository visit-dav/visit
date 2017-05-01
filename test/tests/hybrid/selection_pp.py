# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  selection_pp.py
#
#  Tests:      filter        - named selection
#              operator      - persistent particles
#              plot          - pseudocolor
#
#  Defect ID:  -
#
#  Programmer: Allen Sanderson
#  Date:       January 27, 2016
#
# ----------------------------------------------------------------------------


db = data_path("cq_pp_test_data/cq_pp_test_data.h5part")

#db = "/Projects/VisIt/trunk/data/cq_pp_test_data/cq_pp_test_data.h5part"

OpenDatabase(db, 0, "H5Part")

AddPlot("Pseudocolor", "id", 1, 0)
pc = PseudocolorAttributes()
pc.pointSizePixels =  10
SetPlotOptions(pc)
DrawPlots()

View3DAtts = View3DAttributes()
View3DAtts.viewNormal = (0.286696, -0.8594, 0.423364)
View3DAtts.focus = (4.5, 4.6, 2.6)
View3DAtts.viewUp = (0.0488582, 0.454456, 0.889428)
View3DAtts.viewAngle = 30
View3DAtts.parallelScale = 7.06527
View3DAtts.nearPlane = -14.1305
View3DAtts.farPlane = 14.1305
View3DAtts.imagePan = (0, 0)
View3DAtts.imageZoom = 1
View3DAtts.perspective = 0
View3DAtts.eyeAngle = 2
View3DAtts.centerOfRotationSet = 0
View3DAtts.centerOfRotation = (4.5, 4.6, 2.6)
View3DAtts.axis3DScaleFlag = 0
View3DAtts.axis3DScales = (1, 1, 1)
View3DAtts.shear = (0, 0, 1)
View3DAtts.windowValid = 1
SetView3D(View3DAtts)

SetActivePlots(0)
selName = "selection1"
s = SelectionProperties()
s.variables = ("wt")

s.name = selName
s.source = db
s.host = "localhost"
s.selectionType = s.CumulativeQuerySelection
s.variableMins = (0.75)
s.variableMaxs = (1.0)
s.histogramType = s.HistogramVariable
s.histogramVariable = s.variables[0]
s.histogramNumBins = 10
s.histogramStartBin = 5
s.histogramEndBin = 9
CreateNamedSelection(selName, s)

ApplyNamedSelection(selName)
DrawPlots()
Test("selections_cq_pp_01")


s.histogramType = s.HistogramID
s.histogramNumBins = 10
s.histogramStartBin = 0
s.histogramEndBin = 4
UpdateNamedSelection(selName, s)
DrawPlots()
Test("selections_cq_pp_02")


s.histogramType = s.HistogramMatches
s.histogramNumBins = 3
s.histogramStartBin = 2
s.histogramEndBin = 2
UpdateNamedSelection(selName, s)
DrawPlots()
Test("selections_cq_pp_03")


s.histogramType = s.HistogramTime
s.histogramNumBins = 10
s.histogramStartBin = 5
s.histogramEndBin = 7
UpdateNamedSelection(selName, s)
DrawPlots()
Test("selections_cq_pp_04")


AddOperator("PersistentParticles", 0)
PersistentParticlesAtts = PersistentParticlesAttributes()
PersistentParticlesAtts.startIndex = 0
PersistentParticlesAtts.stopIndex = 0
PersistentParticlesAtts.stride = 1
PersistentParticlesAtts.startPathType = PersistentParticlesAtts.Absolute  # Absolute, Relative
PersistentParticlesAtts.stopPathType = PersistentParticlesAtts.Relative  # Absolute, Relative
PersistentParticlesAtts.traceVariableX = "default"
PersistentParticlesAtts.traceVariableY = "default"
PersistentParticlesAtts.traceVariableZ = "default"
PersistentParticlesAtts.connectParticles = 0
PersistentParticlesAtts.showPoints = 0
PersistentParticlesAtts.indexVariable = "default"
SetOperatorOptions(PersistentParticlesAtts, 0)
DrawPlots()
Test("selections_cq_pp_05")

SetTimeSliderState(9)
DrawPlots()
Test("selections_cq_pp_06")


PersistentParticlesAtts = PersistentParticlesAttributes()
PersistentParticlesAtts.connectParticles = 1
PersistentParticlesAtts.showPoints = 1
PersistentParticlesAtts.indexVariable = "default"
SetOperatorOptions(PersistentParticlesAtts, 0)
DrawPlots()
Test("selections_cq_pp_07")


PersistentParticlesAtts = PersistentParticlesAttributes()
PersistentParticlesAtts.connectParticles = 1
PersistentParticlesAtts.showPoints = 1
PersistentParticlesAtts.indexVariable = "id"
SetOperatorOptions(PersistentParticlesAtts, 0)
DrawPlots()
Test("selections_cq_pp_08")


ApplyNamedSelection("")
DrawPlots()
Test("selections_cq_pp_09")


Exit()
