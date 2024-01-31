# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  amr_stitch.py
#
#  Programmer: Gunther H. Weber
#  Date:       April 29, 2014
#
#  Modifications:
#
#      Alister Maguire, Wed Jan  6 08:31:05 PST 2021
#      Added the -enable-ghosts-for-t-intersections flag, as it
#      is now disabled by default.
#
# ----------------------------------------------------------------------------

# NOTE: the comment below tells visit to use the
# 'enable-ghosts-for-t-intersections' flag when running this test.
# This flag must be enabled for this test to succeed.

#VARGS: "-enable-ghosts-for-t-intersections"

# Turn off all annotation
a = AnnotationAttributes()
TurnOffAllAnnotations(a)

# Files used for tests
filename_template = "AMRStitchCell_test_data/AMRStitchCellTest.%d.no_ghost.phi.%dd.hdf5"
test_case_filenames = [ data_path(filename_template % ( case, dim )) for dim in range(2,4) for case in range(1,5) ]

# Setup plots and run first test
OpenDatabase(test_case_filenames[0])
AddPlot("Contour", "phi")
AddOperator("AMRStitchCell")
DrawPlots()
Test("amr_stitch_01")

# Perform remaining tests
for test_no, ds in enumerate(test_case_filenames[1:], 2):
    ReplaceDatabase(ds)
    Test("amr_stitch_%02d" % test_no)

# Clean-up
DeleteAllPlots()
for ds in test_case_filenames:
    CloseDatabase(ds)

Exit()
