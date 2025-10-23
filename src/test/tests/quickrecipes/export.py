# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Cyrus Harrison, Tue Sep 16 11:19:26 PDT 2025
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


OpenDatabase(silo_data_path("rect3d.silo"))
export_to_vtk()
export_to_silo()
export_to_blueprint()
export_to_csv()

Exit()
