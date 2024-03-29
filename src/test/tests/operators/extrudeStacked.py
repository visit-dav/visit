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
#  Modifications:
#    Kathleen Biagas, Wed Sep 20, 2023
#    Changed SetLocalView to retrieve the view via GetView3D() and only
#    change the viewNormal and viewUp, so that test images are larger.
#
# ----------------------------------------------------------------------------

RequiredDatabasePlugin("VTK")

# For local testing using the CLI

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
  View3DAtts = GetView3D()
  View3DAtts.viewNormal = (0, -1, 0)
  View3DAtts.viewUp = (0, 0, 1)
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
  # Replace the database from before with the one just openned.
  ReplaceDatabase(db)
  #
  ExtrudeStackedAtts = ExtrudeStackedAttributes()
  ExtrudeStackedAtts.axis = (0, 0, 1)
  ExtrudeStackedAtts.byVariable = 1
  ExtrudeStackedAtts.scalarVariableNames = ("default")
  ExtrudeStackedAtts.visualVariableNames = ("default")
  ExtrudeStackedAtts.extentMinima = (-1e+37)
  ExtrudeStackedAtts.extentMaxima = (1e+37)
  ExtrudeStackedAtts.extentScale = (1)
  ExtrudeStackedAtts.variableDisplay = ExtrudeStackedAtts.OriginalData
  ExtrudeStackedAtts.length = 1
  ExtrudeStackedAtts.steps = 1
  ExtrudeStackedAtts.preserveOriginalCellNumbers = 1
  SetOperatorOptions(ExtrudeStackedAtts, 0, 0)
  #
  tmpstr="extrude_%s_v0_node_height" %(databases[i])
  Test(tmpstr)
  #
  ExtrudeStackedAtts = ExtrudeStackedAttributes()
  ExtrudeStackedAtts.axis = (0, 0, 1)
  ExtrudeStackedAtts.byVariable = 1
  ExtrudeStackedAtts.scalarVariableNames = ("V1")
  ExtrudeStackedAtts.visualVariableNames = ("V1")
  ExtrudeStackedAtts.extentMinima = (-1e+37)
  ExtrudeStackedAtts.extentMaxima = (1e+37)
  ExtrudeStackedAtts.extentScale = (1)
  ExtrudeStackedAtts.variableDisplay = ExtrudeStackedAtts.NodeHeight
  ExtrudeStackedAtts.length = 1
  ExtrudeStackedAtts.steps = 1
  ExtrudeStackedAtts.preserveOriginalCellNumbers = 1
  SetOperatorOptions(ExtrudeStackedAtts, 0, 0)
  #
  tmpstr="extrude_%s_v1_node_height" %(databases[i])
  Test(tmpstr)
  #
  ExtrudeStackedAtts = ExtrudeStackedAttributes()
  ExtrudeStackedAtts.axis = (0, 0, 1)
  ExtrudeStackedAtts.byVariable = 1
  ExtrudeStackedAtts.scalarVariableNames = ("V1")
  ExtrudeStackedAtts.visualVariableNames = ("V1")
  ExtrudeStackedAtts.extentMinima = (-1e+37)
  ExtrudeStackedAtts.extentMaxima = (1e+37)
  ExtrudeStackedAtts.extentScale = (1)
  ExtrudeStackedAtts.variableDisplay = ExtrudeStackedAtts.CellHeight
  ExtrudeStackedAtts.length = 1
  ExtrudeStackedAtts.steps = 1
  ExtrudeStackedAtts.preserveOriginalCellNumbers = 1
  SetOperatorOptions(ExtrudeStackedAtts, 0, 0)
  #
  tmpstr="extrude_%s_v1_cell_height" %(databases[i])
  Test(tmpstr)
  #
  ExtrudeStackedAtts = ExtrudeStackedAttributes()
  ExtrudeStackedAtts.axis = (0, 0, 1)
  ExtrudeStackedAtts.byVariable = 1
  ExtrudeStackedAtts.scalarVariableNames = ("V1", "V2")
  ExtrudeStackedAtts.visualVariableNames = ("V1", "V2")
  ExtrudeStackedAtts.extentMinima = (-1e+37, -1e+37)
  ExtrudeStackedAtts.extentMaxima = (1e+37, 1e+37)
  ExtrudeStackedAtts.extentScale = (1, 1)
  ExtrudeStackedAtts.variableDisplay = ExtrudeStackedAtts.NodeHeight
  ExtrudeStackedAtts.length = 1
  ExtrudeStackedAtts.steps = 1
  ExtrudeStackedAtts.preserveOriginalCellNumbers = 1
  SetOperatorOptions(ExtrudeStackedAtts, 0, 0)
  #
  tmpstr="extrude_%s_v2_node_height" %(databases[i])
  Test(tmpstr)
  #
  ExtrudeStackedAtts = ExtrudeStackedAttributes()
  ExtrudeStackedAtts.axis = (0, 0, 1)
  ExtrudeStackedAtts.byVariable = 1
  ExtrudeStackedAtts.scalarVariableNames = ("V1", "V2")
  ExtrudeStackedAtts.visualVariableNames = ("V1", "V2")
  ExtrudeStackedAtts.extentMinima = (-1e+37, -1e+37)
  ExtrudeStackedAtts.extentMaxima = (1e+37, 1e+37)
  ExtrudeStackedAtts.extentScale = (1, 1)
  ExtrudeStackedAtts.variableDisplay = ExtrudeStackedAtts.CellHeight
  ExtrudeStackedAtts.length = 1
  ExtrudeStackedAtts.steps = 1
  ExtrudeStackedAtts.preserveOriginalCellNumbers = 1
  SetOperatorOptions(ExtrudeStackedAtts, 0, 0)
  #
  tmpstr="extrude_%s_v2_cell_height" %(databases[i])
  Test(tmpstr)
  #
  ExtrudeStackedAtts = ExtrudeStackedAttributes()
  ExtrudeStackedAtts.axis = (0, 0, 1)
  ExtrudeStackedAtts.byVariable = 1
  ExtrudeStackedAtts.scalarVariableNames = ("V1", "V2")
  ExtrudeStackedAtts.visualVariableNames = ("V1", "V2")
  ExtrudeStackedAtts.extentMinima = (-1e+37, -1e+37)
  ExtrudeStackedAtts.extentMaxima = (1e+37, 1e+37)
  ExtrudeStackedAtts.extentScale = (1, 1)
  ExtrudeStackedAtts.variableDisplay = ExtrudeStackedAtts.VariableIndex
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
  ExtrudeStackedAtts.extentScale = (1, 1, 1)
  ExtrudeStackedAtts.variableDisplay = ExtrudeStackedAtts.NodeHeight
  ExtrudeStackedAtts.length = 1
  ExtrudeStackedAtts.steps = 1
  ExtrudeStackedAtts.preserveOriginalCellNumbers = 1
  SetOperatorOptions(ExtrudeStackedAtts, 0, 0)
  #
  tmpstr="extrude_%s_v3_node_height" %(databases[i])
  Test(tmpstr)
  #
  ExtrudeStackedAtts = ExtrudeStackedAttributes()
  ExtrudeStackedAtts.axis = (0, 0, 1)
  ExtrudeStackedAtts.byVariable = 1
  ExtrudeStackedAtts.scalarVariableNames = ("V1", "V2", "V3")
  ExtrudeStackedAtts.visualVariableNames = ("V1", "V2", "V3")
  ExtrudeStackedAtts.extentMinima = (-1e+37, -1e+37, -1e+37)
  ExtrudeStackedAtts.extentMaxima = (1e+37, 1e+37, 1e+37)
  ExtrudeStackedAtts.extentScale = (1, 1, 1)
  ExtrudeStackedAtts.variableDisplay = ExtrudeStackedAtts.CellHeight
  ExtrudeStackedAtts.length = 1
  ExtrudeStackedAtts.steps = 1
  ExtrudeStackedAtts.preserveOriginalCellNumbers = 1
  SetOperatorOptions(ExtrudeStackedAtts, 0, 0)
  #
  tmpstr="extrude_%s_v3_cell_height" %(databases[i])
  Test(tmpstr)
  #
  ExtrudeStackedAtts = ExtrudeStackedAttributes()
  ExtrudeStackedAtts.axis = (0, 0, 1)
  ExtrudeStackedAtts.byVariable = 1
  ExtrudeStackedAtts.scalarVariableNames = ("V1", "V2", "V3")
  ExtrudeStackedAtts.visualVariableNames = ("V1", "V2", "V3")
  ExtrudeStackedAtts.extentMinima = (-1e+37, -1e+37, -1e+37)
  ExtrudeStackedAtts.extentMaxima = (1e+37, 1e+37, 1e+37)
  ExtrudeStackedAtts.extentScale = (1, 1, 1)
  ExtrudeStackedAtts.variableDisplay = ExtrudeStackedAtts.VariableIndex
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
  ExtrudeStackedAtts.extentScale = (1, 1, 1, 1)
  ExtrudeStackedAtts.variableDisplay = ExtrudeStackedAtts.NodeHeight
  ExtrudeStackedAtts.length = 1
  ExtrudeStackedAtts.steps = 1
  ExtrudeStackedAtts.preserveOriginalCellNumbers = 1
  SetOperatorOptions(ExtrudeStackedAtts, 0, 0)
  #
  tmpstr="extrude_%s_v4_node_height" %(databases[i])
  Test(tmpstr)
  #
  ExtrudeStackedAtts = ExtrudeStackedAttributes()
  ExtrudeStackedAtts.axis = (0, 0, 1)
  ExtrudeStackedAtts.byVariable = 1
  ExtrudeStackedAtts.scalarVariableNames = ("V1", "V2", "V3", "V4")
  ExtrudeStackedAtts.visualVariableNames = ("V1", "V2", "V3", "V4")
  ExtrudeStackedAtts.extentMinima = (-1e+37, -1e+37, -1e+37, -1e+37)
  ExtrudeStackedAtts.extentMaxima = (1e+37, 1e+37, 1e+37, 1e+37)
  ExtrudeStackedAtts.extentScale = (1, 1, 1, 1)
  ExtrudeStackedAtts.variableDisplay = ExtrudeStackedAtts.CellHeight
  ExtrudeStackedAtts.length = 1
  ExtrudeStackedAtts.steps = 1
  ExtrudeStackedAtts.preserveOriginalCellNumbers = 1
  SetOperatorOptions(ExtrudeStackedAtts, 0, 0)
  #
  tmpstr="extrude_%s_v4_cell_height" %(databases[i])
  Test(tmpstr)
  #
  ExtrudeStackedAtts = ExtrudeStackedAttributes()
  ExtrudeStackedAtts.axis = (0, 0, 1)
  ExtrudeStackedAtts.byVariable = 1
  ExtrudeStackedAtts.scalarVariableNames = ("V1", "V2", "V3", "V4")
  ExtrudeStackedAtts.visualVariableNames = ("V1", "V2", "V3", "V4")
  ExtrudeStackedAtts.extentMinima = (-1e+37, -1e+37, -1e+37, -1e+37)
  ExtrudeStackedAtts.extentMaxima = (1e+37, 1e+37, 1e+37, 1e+37)
  ExtrudeStackedAtts.extentScale = (1, 1, 1, 1)
  ExtrudeStackedAtts.variableDisplay = ExtrudeStackedAtts.VariableIndex
  ExtrudeStackedAtts.length = 1
  ExtrudeStackedAtts.steps = 1
  ExtrudeStackedAtts.preserveOriginalCellNumbers = 1
  SetOperatorOptions(ExtrudeStackedAtts, 0, 0)
  #
  tmpstr="extrude_%s_v4_index" %(databases[i])
  Test(tmpstr)

Exit()
