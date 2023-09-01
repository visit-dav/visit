# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  extrudeStacked.py
#
#  Tests:      operator   - Extrude Stacked
#
#  Programmer: Allen Sanderson
#  Date:       30 August 2023
#
# ----------------------------------------------------------------------------
RequiredDatabasePlugin("ExtrudeStacked_Tester")

# For testing using the CLI

#def data_path( db_name ):
#  db="/Users/allen/Projects/VisIt/git/visit/data/%s" %(db_name)
#  return db

#def Test(case_name):
#   swatts = SaveWindowAttributes()
#   swatts.family = 0
#   swatts.fileName = "/Users/allen/Projects/VisIt/git/visit/data/%s" %(case_name)
#   SetSaveWindowAttributes(swatts)
#   SaveWindow()
#   return

#def TestSection(tmpstr):
#   return

#def Exit():
#   return

# Set the local view
def SetLocalView():
  ResetView()
  #
  View3DAtts = View3DAttributes()
  View3DAtts.viewNormal = (0, -1, 0)
  View3DAtts.focus = (5, 0.5, 13)
  View3DAtts.viewUp = (0, 0, 1)
  View3DAtts.viewAngle = 30
  View3DAtts.parallelScale = 13.9374
  View3DAtts.nearPlane = -27.8747
  View3DAtts.farPlane = 27.8747
  View3DAtts.imagePan = (0, 0)
  View3DAtts.imageZoom = 1
  View3DAtts.perspective = 0
  View3DAtts.eyeAngle = 2
  View3DAtts.centerOfRotationSet = 0
  View3DAtts.centerOfRotation = (5, 0.5, 13)
  View3DAtts.axis3DScaleFlag = 0
  View3DAtts.axis3DScales = (1, 1, 1)
  View3DAtts.shear = (0, 0, 1)
  View3DAtts.windowValid = 1
  SetView3D(View3DAtts)

# Open the database here and add a plot.
db=data_path("extrude_test_data/extrude_structured.vtk")
OpenDatabase(db)

AddPlot("Pseudocolor", "V1", 1, 0)
AddOperator("ExtrudeStacked", 0)
SetActivePlots(0)
DrawPlots()

PseudocolorAtts = PseudocolorAttributes()
PseudocolorAtts.renderSurfaces = 1
PseudocolorAtts.renderWireframe = 1
SetPlotOptions(PseudocolorAtts)

SetLocalView()

# Loop through all of the databases
databases=["lines", "polygons", "rectilinear", "structured" ]

TestSection("Extrude stacked function")
for i in range(len(databases)):
  db=data_path("extrude_test_data/extrude_%s.vtk") %(databases[i])
  tmpstr="Testing database = %s.vtk" %(databases[i])
  TestSection(tmpstr)
  OpenDatabase(db)
  #
  # Replace the database from before with this one as a new plot can not
  # be opened within the loop when using runtest. This issue is a bug
  ReplaceDatabase(db)
  #
  ExtrudeStackedAtts = ExtrudeStackedAttributes()
  ExtrudeStackedAtts.axis = (0, 0, 1)
  ExtrudeStackedAtts.byVariable = 0
  ExtrudeStackedAtts.scalarVariableNames = ()
  ExtrudeStackedAtts.visualVariableNames = ()
  ExtrudeStackedAtts.extentMinima = ()
  ExtrudeStackedAtts.extentMaxima = ()
  ExtrudeStackedAtts.variableDisplay = ExtrudeStackedAtts.Index  # Index, Value
  ExtrudeStackedAtts.length = 1
  ExtrudeStackedAtts.steps = 1
  ExtrudeStackedAtts.preserveOriginalCellNumbers = 1
  SetOperatorOptions(ExtrudeStackedAtts, 0, 0)
  #
  tmpstr="extrude_%s_fixed" %(databases[i])
  Test(tmpstr)
  #
  ExtrudeStackedAtts = ExtrudeStackedAttributes()
  ExtrudeStackedAtts.axis = (0, 0, 1)
  ExtrudeStackedAtts.byVariable = 1
  ExtrudeStackedAtts.scalarVariableNames = ()
  ExtrudeStackedAtts.visualVariableNames = ()
  ExtrudeStackedAtts.extentMinima = ()
  ExtrudeStackedAtts.extentMaxima = ()
  ExtrudeStackedAtts.variableDisplay = ExtrudeStackedAtts.Index  # Index, Value
  ExtrudeStackedAtts.length = 1
  ExtrudeStackedAtts.steps = 1
  ExtrudeStackedAtts.preserveOriginalCellNumbers = 1
  SetOperatorOptions(ExtrudeStackedAtts, 0, 0)
  #
  tmpstr="extrude_%s_v0_value" %(databases[i])
  Test(tmpstr)
  #
  ExtrudeStackedAtts = ExtrudeStackedAttributes()
  ExtrudeStackedAtts.axis = (0, 0, 1)
  ExtrudeStackedAtts.byVariable = 1
  ExtrudeStackedAtts.scalarVariableNames = ("V1")
  ExtrudeStackedAtts.visualVariableNames = ("V1")
  ExtrudeStackedAtts.extentMinima = (-1e+37)
  ExtrudeStackedAtts.extentMaxima = (1e+37)
  ExtrudeStackedAtts.variableDisplay = ExtrudeStackedAtts.Value  # Index, Value
  ExtrudeStackedAtts.length = 1
  ExtrudeStackedAtts.steps = 1
  ExtrudeStackedAtts.preserveOriginalCellNumbers = 1
  SetOperatorOptions(ExtrudeStackedAtts, 0, 0)
  #
  tmpstr="extrude_%s_v1_value" %(databases[i])
  Test(tmpstr)
  #
  ExtrudeStackedAtts = ExtrudeStackedAttributes()
  ExtrudeStackedAtts.axis = (0, 0, 1)
  ExtrudeStackedAtts.byVariable = 1
  ExtrudeStackedAtts.scalarVariableNames = ("V1", "V2")
  ExtrudeStackedAtts.visualVariableNames = ("V1", "V2")
  ExtrudeStackedAtts.extentMinima = (-1e+37, -1e+37)
  ExtrudeStackedAtts.extentMaxima = (1e+37, 1e+37)
  ExtrudeStackedAtts.variableDisplay = ExtrudeStackedAtts.Value  # Index, Value
  ExtrudeStackedAtts.length = 1
  ExtrudeStackedAtts.steps = 1
  ExtrudeStackedAtts.preserveOriginalCellNumbers = 1
  SetOperatorOptions(ExtrudeStackedAtts, 0, 0)
  #
  tmpstr="extrude_%s_v2_value" %(databases[i])
  Test(tmpstr)
  #
  ExtrudeStackedAtts = ExtrudeStackedAttributes()
  ExtrudeStackedAtts.axis = (0, 0, 1)
  ExtrudeStackedAtts.byVariable = 1
  ExtrudeStackedAtts.scalarVariableNames = ("V1", "V2")
  ExtrudeStackedAtts.visualVariableNames = ("V1", "V2")
  ExtrudeStackedAtts.extentMinima = (-1e+37, -1e+37)
  ExtrudeStackedAtts.extentMaxima = (1e+37, 1e+37)
  ExtrudeStackedAtts.variableDisplay = ExtrudeStackedAtts.Index  # Index, Value
  ExtrudeStackedAtts.length = 1
  ExtrudeStackedAtts.steps = 1
  ExtrudeStackedAtts.preserveOriginalCellNumbers = 1
  SetOperatorOptions(ExtrudeStackedAtts, 0, 0)
  #
  tmpstr="extrude_%s_v2_index" %(databases[i])
  Test(tmpstr)
  #
  ExtrudeStackedAtts = ExtrudeStackedAttributes()
  ExtrudeStackedAtts.axis = (0, 0, 1)
  ExtrudeStackedAtts.byVariable = 1
  ExtrudeStackedAtts.scalarVariableNames = ("V1", "V2", "V3")
  ExtrudeStackedAtts.visualVariableNames = ("V1", "V2", "V3")
  ExtrudeStackedAtts.extentMinima = (-1e+37, -1e+37, -1e+37)
  ExtrudeStackedAtts.extentMaxima = (1e+37, 1e+37, 1e+37)
  ExtrudeStackedAtts.variableDisplay = ExtrudeStackedAtts.Value  # Index, Value
  ExtrudeStackedAtts.length = 1
  ExtrudeStackedAtts.steps = 1
  ExtrudeStackedAtts.preserveOriginalCellNumbers = 1
  SetOperatorOptions(ExtrudeStackedAtts, 0, 0)
  #
  tmpstr="extrude_%s_v3_value" %(databases[i])
  Test(tmpstr)
  #
  ExtrudeStackedAtts = ExtrudeStackedAttributes()
  ExtrudeStackedAtts.axis = (0, 0, 1)
  ExtrudeStackedAtts.byVariable = 1
  ExtrudeStackedAtts.scalarVariableNames = ("V1", "V2", "V3")
  ExtrudeStackedAtts.visualVariableNames = ("V1", "V2", "V3")
  ExtrudeStackedAtts.extentMinima = (-1e+37, -1e+37, -1e+37)
  ExtrudeStackedAtts.extentMaxima = (1e+37, 1e+37, 1e+37)
  ExtrudeStackedAtts.variableDisplay = ExtrudeStackedAtts.Index  # Index, Value
  ExtrudeStackedAtts.length = 1
  ExtrudeStackedAtts.steps = 1
  ExtrudeStackedAtts.preserveOriginalCellNumbers = 1
  SetOperatorOptions(ExtrudeStackedAtts, 0, 0)
  #
  tmpstr="extrude_%s_v3_index" %(databases[i])
  Test(tmpstr)
  #
  ExtrudeStackedAtts = ExtrudeStackedAttributes()
  ExtrudeStackedAtts.axis = (0, 0, 1)
  ExtrudeStackedAtts.byVariable = 1
  ExtrudeStackedAtts.scalarVariableNames = ("V1", "V2", "V3", "V4")
  ExtrudeStackedAtts.visualVariableNames = ("V1", "V2", "V3", "V4")
  ExtrudeStackedAtts.extentMinima = (-1e+37, -1e+37, -1e+37, -1e+37)
  ExtrudeStackedAtts.extentMaxima = (1e+37, 1e+37, 1e+37, 1e+37)
  ExtrudeStackedAtts.variableDisplay = ExtrudeStackedAtts.Value  # Index, Value
  ExtrudeStackedAtts.length = 1
  ExtrudeStackedAtts.steps = 1
  ExtrudeStackedAtts.preserveOriginalCellNumbers = 1
  SetOperatorOptions(ExtrudeStackedAtts, 0, 0)
  #
  tmpstr="extrude_%s_v4_value" %(databases[i])
  Test(tmpstr)
  #
  ExtrudeStackedAtts = ExtrudeStackedAttributes()
  ExtrudeStackedAtts.axis = (0, 0, 1)
  ExtrudeStackedAtts.byVariable = 1
  ExtrudeStackedAtts.scalarVariableNames = ("V1", "V2", "V3", "V4")
  ExtrudeStackedAtts.visualVariableNames = ("V1", "V2", "V3", "V4")
  ExtrudeStackedAtts.extentMinima = (-1e+37, -1e+37, -1e+37, -1e+37)
  ExtrudeStackedAtts.extentMaxima = (1e+37, 1e+37, 1e+37, 1e+37)
  ExtrudeStackedAtts.variableDisplay = ExtrudeStackedAtts.Index  # Index, Value
  ExtrudeStackedAtts.length = 1
  ExtrudeStackedAtts.steps = 1
  ExtrudeStackedAtts.preserveOriginalCellNumbers = 1
  SetOperatorOptions(ExtrudeStackedAtts, 0, 0)
  #
  tmpstr="extrude_%s_v4_index" %(databases[i])
  Test(tmpstr)

Exit()
