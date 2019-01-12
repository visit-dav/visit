# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  sessionview.py
#
#  Tests:      mesh      - 3D unstructured, 3D rectilinear, single domain
#              plots     - Pseudocolor, Mesh
#              operators - Reflect
#
#  Notes:      This test case makes sure that we can switch back and forth
#              between session files that use expressions.
#
#  Programmer: Brad Whitlock
#  Date:       Wed Dec 31 15:21:23 PST 2003
#
#  Modifications:
#
# ----------------------------------------------------------------------------

TurnOnAllAnnotations()

# Open the first session file
RestoreSessionWithDifferentSources(tests_path("session","sessionview1.session"), 0,
                                   silo_data_path("globe.silo"))

Test("sessionview00")

# Open the second session file
RestoreSession("tests/session/sessionview2.session", 0)
RestoreSessionWithDifferentSources(tests_path("session","sessionview2.session"), 0,
                                   data_path("ANALYZE_test_data",
                                             "s01_anatomy_stripped.hdr"))


Test("sessionview01")

# Open the first session file again. This used to crash VisIt
RestoreSessionWithDifferentSources(tests_path("session","sessionview1.session"), 0,
                                   silo_data_path("globe.silo"))

Test("sessionview02")

# Open the second session file again.
RestoreSessionWithDifferentSources(tests_path("session","sessionview2.session"), 0,
                                   data_path("ANALYZE_test_data",
                                             "s01_anatomy_stripped.hdr"))
Test("sessionview03")

# Open the first session file
RestoreSessionWithDifferentSources(tests_path("session","sessionview1.session"), 0,
                                   silo_data_path("globe.silo"))
Test("sessionview04")

Exit()
