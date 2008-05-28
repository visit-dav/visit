# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  legendproperties.py
#
#  Notes:      This test case sets legend properties and makes sure that they
#              can be accessed via uses a session file to create a plot that uses
#              a user defined color table. This makes sure that user defined
#              color tables can be used from session files. VisIt00004000.
#
#  Programmer: Brad Whitlock
#  Date:       Mon Mar 26 10:48:25 PDT 2007
#
#  Modifications:
#
# ----------------------------------------------------------------------------

TurnOnAllAnnotations()
RestoreSession("tests/session/legendproperties.session", 0)
Test("legendproperties00")

# Now that we've restored the session, see if we can get the legend objects
# and change them.
subsetLegend = GetAnnotationObject(GetPlotList().GetPlots(0).plotName)
pcLegend = GetAnnotationObject(GetPlotList().GetPlots(1).plotName)

subsetLegend.drawBoundingBox = 0
pcLegend.drawBoundingBox = 0
Test("legendproperties01")

# Make sure that the plots are deleted
DeleteAllPlots()

Exit()
