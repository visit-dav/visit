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
def data_path( db_name ):
  db="/Projects/VisIt/trunk/build/data/%s" %(db_name)
  return db

def Test(case_name):
   swatts = SaveWindowAttributes()
   swatts.family = 0
   swatts.fileName = "/Projects/tmp/fb/ser/%s" %(case_name)
   SetSaveWindowAttributes(swatts)
   SaveWindow()
   return

def TestSection(tmpstr):
   return

def Exit():
   return   


databases=["sorted", "unsorted"]



for i in range(len(databases)):
  DeleteAllPlots()
  TestSection(databases[i])
  db=data_path("cq_pp_test_data/%s.h5part") %(databases[i])

  OpenDatabase(db, 0, "H5Part")

  AddPlot("Pseudocolor", "id", 1, 0)
  DrawPlots()

  ViewCurveAtts = ViewCurveAttributes()
  ViewCurveAtts.domainCoords = (0, 1)
  ViewCurveAtts.rangeCoords = (0, 1)
  ViewCurveAtts.viewportCoords = (0.2, 0.95, 0.15, 0.95)
  ViewCurveAtts.domainScale = ViewCurveAtts.LINEAR  # LINEAR, LOG
  ViewCurveAtts.rangeScale = ViewCurveAtts.LINEAR  # LINEAR, LOG
  SetViewCurve(ViewCurveAtts)
  View2DAtts = View2DAttributes()
  View2DAtts.windowCoords = (0, 1, 0, 1)
  View2DAtts.viewportCoords = (0.2, 0.95, 0.15, 0.95)
  View2DAtts.fullFrameActivationMode = View2DAtts.Auto  # On, Off, Auto
  View2DAtts.fullFrameAutoThreshold = 100
  View2DAtts.xScale = View2DAtts.LINEAR  # LINEAR, LOG
  View2DAtts.yScale = View2DAtts.LINEAR  # LINEAR, LOG
  View2DAtts.windowValid = 0
  SetView2D(View2DAtts)
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
  ViewAxisArrayAtts = ViewAxisArrayAttributes()
  ViewAxisArrayAtts.domainCoords = (0, 1)
  ViewAxisArrayAtts.rangeCoords = (0, 1)
  ViewAxisArrayAtts.viewportCoords = (0.15, 0.9, 0.1, 0.85)
  SetViewAxisArray(ViewAxisArrayAtts)
    
  SetActivePlots(0)

  selName = databases[i]
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
  tmpstr="selections_cq_pp_%s_01" %(databases[i])
  Test(tmpstr)

  
  s.histogramType = s.HistogramID
  s.histogramNumBins = 10
  s.histogramStartBin = 0
  s.histogramEndBin = 4
  UpdateNamedSelection(selName, s)
  DrawPlots()
  tmpstr="selections_cq_pp_%s_02" %(databases[i])
  Test(tmpstr)

  
  s.histogramType = s.HistogramMatches
  s.histogramNumBins = 3
  s.histogramStartBin = 2
  s.histogramEndBin = 2
  UpdateNamedSelection(selName, s)
  DrawPlots()
  tmpstr="selections_cq_pp_%s_03" %(databases[i])
  Test(tmpstr)
  
  
  s.histogramType = s.HistogramTime
  s.histogramNumBins = 10
  s.histogramStartBin = 5
  s.histogramEndBin = 7
  UpdateNamedSelection(selName, s)
  DrawPlots()
  tmpstr="selections_cq_pp_%s_04" %(databases[i])
  Test(tmpstr)
  
  
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
  tmpstr="selections_cq_pp_%s_05" %(databases[i])
  Test(tmpstr)

  
  SetTimeSliderState(9)
  DrawPlots()
  tmpstr="selections_cq_pp_%s_06" %(databases[i])
  Test(tmpstr)
  
  
  PersistentParticlesAtts = PersistentParticlesAttributes()
  PersistentParticlesAtts.connectParticles = 1
  PersistentParticlesAtts.showPoints = 1
  PersistentParticlesAtts.indexVariable = "default"
  SetOperatorOptions(PersistentParticlesAtts, 0)
  DrawPlots()
  tmpstr="selections_cq_pp_%s_07" %(databases[i])
  Test(tmpstr)
  
  
  PersistentParticlesAtts = PersistentParticlesAttributes()
  PersistentParticlesAtts.connectParticles = 1
  PersistentParticlesAtts.showPoints = 1
  PersistentParticlesAtts.indexVariable = "id"
  SetOperatorOptions(PersistentParticlesAtts, 0)
  DrawPlots()
  tmpstr="selections_cq_pp_%s_08" %(databases[i])
  Test(tmpstr)
  
  
  ApplyNamedSelection("")
  DrawPlots()
  tmpstr="selections_cq_pp_%s_09" %(databases[i])
  Test(tmpstr)
  
  
Exit()

selName = databases[i]
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
