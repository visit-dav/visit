import os

data_path = os.environ["VISIT_TEST_DATA"]
variable = os.environ["VISIT_TEST_VARIABLE"]
output_dir = os.environ["VISIT_RENDER_DIR"]

if OpenDatabase(data_path) != 1:
    raise RuntimeError("Could not open %s" % data_path)
if AddPlot("Pseudocolor", variable) != 1:
    raise RuntimeError("Could not add the Pseudocolor plot")
if DrawPlots() != 1:
    raise RuntimeError("Could not draw the test plot")

attributes = SaveWindowAttributes()
attributes.outputToCurrentDirectory = 0
attributes.outputDirectory = output_dir
attributes.fileName = "visit_gui_smoke"
attributes.family = 0
attributes.format = attributes.PNG
attributes.width = 640
attributes.height = 480
SetSaveWindowAttributes(attributes)

if SaveWindow() == "":
    raise RuntimeError("SaveWindow did not return an output path")

print("VISIT_GUI_SMOKE_OK")
DeleteAllPlots()
CloseDatabase(data_path)
