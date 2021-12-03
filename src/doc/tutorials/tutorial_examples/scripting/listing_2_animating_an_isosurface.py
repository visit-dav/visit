###########################################
# file: VisIt Scripting Tutorial Listing 2
###########################################
#
# 1) Open VisIt
# 2) Open 'example.silo'
# 3) Paste and execute via the Commands window, or the CLI.
#

DeleteAllPlots()
AddPlot("Pseudocolor", "temp")
iso_atts = IsosurfaceAttributes()
iso_atts.contourMethod = iso_atts.Value
iso_atts.variable = "temp"
AddOperator("Isosurface")
DrawPlots()
for i in range(30):
   iso_atts.contourValue = (2 + 0.1*i)
   SetOperatorOptions(iso_atts)
   # For moviemaking, you'll need to save off the image
   # SaveWindow()
