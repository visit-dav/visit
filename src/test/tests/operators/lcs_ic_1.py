# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  lcs_ic_1.py
#
#  Tests:      operator   - LCS, IntegralCurve
#
#  Programmer: Allen Sanderson
#  Date:       August 25, 2015
# ----------------------------------------------------------------------------

Source(tests_path("operators", "lcs_operator_main"))

LCS_operator("IntegralCurve", "ftle_double_gyre_1_domain")

Exit()
