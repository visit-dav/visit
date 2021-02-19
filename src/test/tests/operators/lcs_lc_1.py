# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  lcs_lc_1.py
#
#  Tests:      operator   - LCS
#
#  Programmer: Allen Sanderson
#  Date:       August 25, 2015
# ----------------------------------------------------------------------------

Source(tests_path("operators", "lcs_operator_main"))

LCS_operator("LimitCycle", "ftle_double_gyre_1_domain")

Exit()
