###########################################
# file: VisIt Scripting Tutorial Listing 7
###########################################
#
# 1) Open VisIt
# 2) Open 'example.silo'
# 3) Paste and execute via the Commands window, or the CLI.
#

def TakeMassPerSlice():
   DeleteAllPlots()
   AddPlot("Pseudocolor", "chromeVf")
   AddOperator("Slice")
   DrawPlots()
   f = open("mass_per_slice.ultra", "w")
   f.write("# mass_per_slice\n")
   for i in range(50):
     intercept = -10 + 20*(i/49.)
     s = SliceAttributes()
     s.axisType = s.XAxis
     s.originType = s.Intercept
     s.originIntercept = intercept
     SetOperatorOptions(s)
     Query("Weighted Variable Sum")
     t2 = GetQueryOutputValue()
     str = "%25.15e %25.15e\n" %(intercept, t2)
     f.write(str)
   f.close()
 
TakeMassPerSlice()
DeleteAllPlots()
OpenDatabase("mass_per_slice.ultra")
AddPlot("Curve", "mass_per_slice")
DrawPlots()
