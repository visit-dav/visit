# ----------------------------------------------------------------------------
#  Test script that creates a plot
#
#  Modifications:
#
# ----------------------------------------------------------------------------

OpenDatabase("localhost:/usr/gapps/visit/data/globe.silo")
AddPlot("Pseudocolor", "u")
DrawPlots()
