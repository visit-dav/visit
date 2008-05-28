# ---------------------------------------------------------------------------- 
#  CLASSES: nightly
#
#  Test Case:  pickcleanzonesonly.py
#  Tests:      queries     - Pick on mixed zones of clean zones only
#
#  Defect ID:  VisIt00003894
#
#  Programmer: Jeremy Meredith
#  Date:       October 30, 2003
#
#  Modifications:
#    Kathleen Bonnell, Thu Apr 29 07:40:58 PDT 2004 
#    Slice defaults atts have changed, update accordingly. 
#
# ----------------------------------------------------------------------------

# Make a sliced filled boundary plot with clean zones only enabled
TurnOnAllAnnotations()
OpenDatabase("../data/ucd3d.silo")
AddPlot("FilledBoundary","mat1")
f=FilledBoundaryAttributes()
f.cleanZonesOnly=1
SetPlotOptions(f)
AddOperator("Slice")
s=SliceAttributes()
s.axisType=s.YAxis
s.originIntercept = .01
s.project2d=1
SetOperatorOptions(s)
DrawPlots()

# Pick on one of the mixed zones
Pick((-3.5,10.1))
text = GetPickOutput()
TestText("PickCleanZonesOnly1", text)

# Change the slice position and do it again
s.axisType=s.ZAxis
s.originIntercept=3.
SetOperatorOptions(s)
Pick((-1,3.2))
text = GetPickOutput()
TestText("PickCleanZonesOnly2", text)

# Remove the slice and do it again
RemoveAllOperators()
Pick((3,2,19))
text = GetPickOutput()
TestText("PickCleanZonesOnly3", text)

# All done!
Exit()
