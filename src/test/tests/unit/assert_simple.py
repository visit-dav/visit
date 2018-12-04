# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  assert_simple.py
#
#  Tests:      VisIt Test Suite Assertions
#
#  Defect ID:  none
#
#  Programmer: Cyrus Harrison
#  Date:       Fri Nov 22 14:46:01 PST 2013
#
# ----------------------------------------------------------------------------

AssertTrue("assert_simple_001",True)
AssertFalse("assert_simple_002",False)
AssertEqual("assert_simple_003",100,100)

AssertGT("assert_simple_005",200,100)
AssertGTE("assert_simple_006",200,100)
AssertGTE("assert_simple_007",200,200)

AssertLT("assert_simple_008",100,200)
AssertLTE("assert_simple_009",100,200)
AssertLTE("assert_simple_010",200,200)

Exit()
