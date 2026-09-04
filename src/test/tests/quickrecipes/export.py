# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Cyrus Harrison, Tue Sep 16 11:19:26 PDT 2025
# 
#  Modifications:
#    Justin Privitera, Fri Aug 14 15:52:42 PDT 2026
#    Added OBJ export quickrecipe.
# ----------------------------------------------------------------------------

from os.path import join as pjoin


def clean_data(path):
  print(path)
  if os.path.isfile(path):
    os.remove(path)
  elif os.path.isfile(path):
    # Note: assumes passed dir is already empty
    os.rmdir(path)

def export_to_vtk():
  clean_data("my_vtk_export.vtk")
  # exportToVTK {
  # Create a Pseudocolor plot and export multiple fields
  AddPlot("Pseudocolor", "d")
  # optional: setup operators to modify data before export
  DrawPlots()
  eatts = ExportDBAttributes()
  eatts.filename = "my_vtk_export"
  eatts.db_type = "VTK"
  eatts.variables = ("d","p")
  ExportDatabase(eatts)
  # exportToVTK }
  TestValueEQ("qr_export_to_vtk",os.path.isfile("my_vtk_export.vtk"),True)
  
def export_to_silo():
  clean_data("my_silo_export.silo")
  # exportToSilo {
  # Create a Pseudocolor plot and export multiple fields
  AddPlot("Pseudocolor", "d")
  # optional: setup operators to modify data before export
  DrawPlots()
  eatts = ExportDBAttributes()
  eatts.filename = "my_silo_export"
  eatts.db_type = "Silo"
  eatts.variables = ("d","p")
  ExportDatabase(eatts)
  # exportToSilo }
  TestValueEQ("qr_export_to_silo",os.path.isfile("my_silo_export.silo"),True)
  DeleteAllPlots()

def export_to_blueprint():
  clean_data("my_blueprint_export.cycle_000048.root")
  # exportToBlueprint {
  # Create a Pseudocolor plot and export multiple fields
  AddPlot("Pseudocolor", "d")
  # optional: setup operators to modify data before export
  DrawPlots()
  eatts = ExportDBAttributes()
  eatts.filename = "my_blueprint_export"
  eatts.db_type = "Blueprint"
  eatts.variables = ("d","p")
  ExportDatabase(eatts)
  # exportToBlueprint }
  TestValueEQ("qr_export_to_blueprint",os.path.isfile("my_blueprint_export.cycle_000048.root"),True)
  DeleteAllPlots()

def export_to_csv():
  clean_data(pjoin("my_csv_export.csv","element_data.csv"))
  clean_data(pjoin("my_csv_export.csv","vertex_data.csv"))
  # clean up dir last 
  clean_data("my_csv_export.csv")
  # exportToBlueprintCSV {
  # Create a Pseudocolor plot and export multiple fields
  AddPlot("Pseudocolor", "d")
  # optional: setup operators to modify data before export
  DrawPlots()
  eatts = ExportDBAttributes()
  eatts.filename = "my_csv_export"
  eatts.db_type = "Blueprint"
  opts = GetExportOptions("Blueprint")
  eatts.variables = ("d","p")
  opts["Operation"] = "Flatten_CSV"
  ExportDatabase(eatts,opts)
  # exportToBlueprintCSV }
  TestValueEQ("qr_export_to_csv",os.path.isdir("my_csv_export.csv"),True)
  TestValueEQ("qr_export_to_csv_vertex",os.path.isfile(pjoin("my_csv_export.csv","element_data.csv")),True)
  TestValueEQ("qr_export_to_csv_cell",os.path.isfile(pjoin("my_csv_export.csv","vertex_data.csv")),True)
  DeleteAllPlots()

def export_to_obj():
  clean_data("my_obj_export.obj")
  clean_data("my_obj_export.mtl")
  clean_data("my_obj_export.png")
  # exportToOBJ {
  # Create a Pseudocolor plot and export a single field
  AddPlot("Pseudocolor", "d")
  # optional: setup operators to modify data before export
  DrawPlots()
  eatts = ExportDBAttributes()
  eatts.db_type = "WavefrontOBJ"
  eatts.variables = ("d")
  eatts.filename = "my_obj_export"

  # fetch Pseudocolor plot attributes to hand to OBJ export options
  # (assumes your current active plot is a pseudocolor plot)
  pc_atts = GetPlotOptions()

  opts = GetExportOptions("WavefrontOBJ")
  opts["Output colors"]              = 1 # turn colors on (1) or off (0)
  opts["Color table"]                = pc_atts.colorTableName # choose a color table by name
  opts["Number of colors"]           = 256 # choose a color table resolution
  opts["Invert color table"]         = pc_atts.invertColorTable
  opts["Use minimum"]                = pc_atts.minFlag
  opts["Minimum"]                    = pc_atts.min
  opts["Use color for values < min"] = pc_atts.useBelowMinColor
  opts["Color for values < min"]     = pc_atts.belowMinColor
  opts["Use maximum"]                = pc_atts.maxFlag
  opts["Maximum"]                    = pc_atts.max
  opts["Use color for values > max"] = pc_atts.useAboveMaxColor
  opts["Color for values > max"]     = pc_atts.aboveMaxColor

  ExportDatabase(eatts, opts)
  # exportToOBJ }
  TestValueEQ("qr_export_to_obj",os.path.isfile("my_obj_export.obj"),True)
  TestValueEQ("qr_export_to_obj_mtl",os.path.isfile("my_obj_export.mtl"),True)
  TestValueEQ("qr_export_to_obj_png",os.path.isfile("my_obj_export.png"),True)
  DeleteAllPlots()


OpenDatabase(silo_data_path("rect3d.silo"))
export_to_vtk()
export_to_silo()
export_to_blueprint()
export_to_csv()
export_to_obj()

Exit()
