# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  restorewithsources.py
#
#  Notes:      This test case tests restoring sessions with different sources.
#
#  Programmer: Brad Whitlock
#  Date:       Wed Mar 28 10:36:04 PDT 2007
#
#  Modifications:
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
# ----------------------------------------------------------------------------

TurnOnAllAnnotations()

RestoreSession("tests/session/restorewithsources.session", 0)
Test("restorewithsources00")

# Now, try restoring the session with a different data file. Note that since
# there's just one source, we pass a string instead of a tuple of strings.
RestoreSessionWithDifferentSources(tests_path("session/restorewithsources.session"),
                                   0,
                                   silo_data_path("rect2d.silo") )
Test("restorewithsources01")

# Make sure that the plots are deleted
DeleteAllPlots()

Exit()
