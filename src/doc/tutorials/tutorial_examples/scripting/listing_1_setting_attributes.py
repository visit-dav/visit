###########################################
# file: VisIt Scripting Tutorial Listing 1
###########################################
#
# 1) Open VisIt
# 2) Open 'example.silo'
# 3) Paste and execute via the Commands window, or the CLI.
#

DeleteAllPlots()
AddPlot("Pseudocolor", "temp")
DrawPlots()
p = PseudocolorAttributes()
p.minFlag = 1
p.maxFlag = 1
p.min = 3.5
p.max = 7.5
SetPlotOptions(p)
