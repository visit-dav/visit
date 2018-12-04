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
#    Brad Whitlock, Thu Jul 22 11:03:42 PDT 2010
#    Force some save window settings so testing routines don't mess up in
#    the scalable,parallel,icet mode. VisIt itself, outside of testing doesn't
#    have problems as far as I can tell so we're working around "issues" with
#    our testing routines.
#    
# ----------------------------------------------------------------------------

# Get the save window atts and force a few settings.
sa = GetSaveWindowAttributes()
sa.screenCapture = 0
sa.width,sa.height = 300,300

TurnOnAllAnnotations()
RestoreSessionWithDifferentSources(tests_path("session","legendproperties.session"), 0,
                                   silo_data_path("bigsil.silo"))


Test("legendproperties00", altSWA=sa)

# Now that we've restored the session, see if we can get the legend objects
# and change them.
subsetLegend = GetAnnotationObject(GetPlotList().GetPlots(0).plotName)
pcLegend = GetAnnotationObject(GetPlotList().GetPlots(1).plotName)

subsetLegend.drawBoundingBox = 0
pcLegend.drawBoundingBox = 0
Test("legendproperties01", altSWA=sa)

# Make sure that the plots are deleted
DeleteAllPlots()

Exit()
