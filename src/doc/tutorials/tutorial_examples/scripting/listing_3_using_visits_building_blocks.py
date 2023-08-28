###########################################
# file: VisIt Scripting Tutorial Listing 3
###########################################
#
# 1) Open VisIt
# 2) Open 'example.silo'
# 3) Paste and execute via the Commands window, or the CLI.
#

# Clear any previous plots
DeleteAllPlots()
# Create a plot of the scalar field 'temp'
AddPlot("Pseudocolor","temp")
# Slice the volume to show only three
# external faces.
AddOperator("ThreeSlice")
tatts = ThreeSliceAttributes()
tatts.x = -10
tatts.y = -10
tatts.z = -10
SetOperatorOptions(tatts)
DrawPlots()
# Find the maximum value of the field 'temp'
Query("Max")
val = GetQueryOutputValue()
print("Max value of 'temp' = ", val)

# Create a streamline plot that follows
# the gradient of 'temp'
DefineVectorExpression("g","gradient(temp)")
AddPlot("Pseudocolor", "operators/IntegralCurve/g")
iatts = IntegralCurveAttributes()
iatts.sourceType = iatts.SpecifiedBox
iatts.sampleDensity0 = 7
iatts.sampleDensity1 = 7
iatts.sampleDensity2 = 7
iatts.dataValue = iatts.SeedPointID
iatts.integrationType = iatts.DormandPrince
iatts.issueStiffnessWarnings = 0
iatts.issueCriticalPointsWarnings = 0
SetOperatorOptions(iatts)

# set style of streamlines
patts = PseudocolorAttributes()
patts.lineType = patts.Tube 
patts.tailStyle = patts.Spheres
patts.headStyle = patts.Cones
patts.endPointRadiusBBox = 0.01
SetPlotOptions(patts)

DrawPlots()