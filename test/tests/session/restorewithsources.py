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
# ----------------------------------------------------------------------------

RestoreSession("tests/session/restorewithsources.session", 0)
Test("restorewithsources00")

# Now, try restoring the session with a different data file. Note that since
# there's just one source, we pass a string instead of a tuple of strings.
RestoreSessionWithDifferentSources("tests/session/restorewithsources.session", 0, "../data/rect2d.silo")
Test("restorewithsources01")

# Make sure that the plots are deleted
DeleteAllPlots()

Exit()
