# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  wavecontour.py
#
#  Tests:      mesh      - 3D curvilinear, single domain
#              plots     - Contour, Mesh
#              operators - Reflect
#
#  Notes:      This test case uses a session file to ensure that the Contour
#              plot can be restored from a session file and have its correct
#              colors and number of contour levels. This test case tests
#              for a bug reported in VisIt00003883.
#
#  Programmer: Brad Whitlock
#  Date:       Wed Oct 22 15:14:20 PST 2003
#
#  Modifications:
#
# ----------------------------------------------------------------------------

TurnOnAllAnnotations()
RestoreSessionWithDifferentSources(tests_path("session","wavecontour.session"), 0,
                                   silo_data_path("wave.visit"))
# tslider state is lost w/ restore with sources
TimeSliderSetState(28)
Test("wavecontour00")
Exit()
