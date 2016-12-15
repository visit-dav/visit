# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  LCS.py
#
#  Tests:      operator   - LCS
#
#  Programmer: Allen Sanderson
#  Date:       August 25, 2015
# ----------------------------------------------------------------------------

visit.Source(TestEnv.params["tests_dir"][:-5] + "tests/operators/lcs_operator.py")

operator="LimirCycle"

databases=["ftle_double_gyre_2_domains"]

LCS_operator( operator, databases )

Exit()
  
# LCS->IC single   domain - native mesh      - serial   - okay
# LCS->IC single   domain - rectilinear grid - serial   - okay
# LCS->IC multiple domain - native mesh      - serial   - okay
# LCS->IC multiple domain - rectilinear grid - serial   - failed zero velocity
  
# LCS->IC single   domain - native mesh      - parallel - okay
# LCS->IC single   domain - rectilinear grid - parallel - okay
# LCS->IC multiple domain - native mesh      - parallel - okay
# LCS->IC multiple domain - rectilinear grid - parallel - failed in avtPICSFilter::InitializeLocators (fixed but failed like serial)
