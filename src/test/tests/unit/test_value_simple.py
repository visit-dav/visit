# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  test_value_simple.py
#
#  Tests:      VisIt Test Suite Test Value tests
#
#  Defect ID:  none
#
#  Programmer: Mark C. Miller, Sun Jan 10 10:24:59 PST 2021
#
# ----------------------------------------------------------------------------

# Basic cases
TestSection("Basic")
TestValueEQ("test_value_basic_001",100,100)
TestValueNE("test_value_basic_002",200,100)

TestValueLT("test_value_basic_003",100,200)
TestValueLE("test_value_basic_004",100,200)
TestValueLE("test_value_basic_005",200,200)

TestValueGT("test_value_basic_006",200,100)
TestValueGE("test_value_basic_007",200,100)
TestValueGE("test_value_basic_008",200,200)

TestValueEQ("test_value_basic_009",100.0,100)

# Rounding cases
TestSection("Precision")
TestValueEQ("test_value_prec_001",200.0001,200,3)
TestValueEQ("test_value_prec_002",200.0000000001,200,9)
TestValueNE("test_value_prec_004",200.0001,200,9)

# IN operator
TestSection("In Operator")
TestValueIN("test_value_in_001",(100,200,300),200)
TestValueIN("test_value_in_001",[(0,0), (1,0), (1,-1), (0,1)],(1,-1))

# Custom equivalence operator for TestValueIN
def EqMod5(a,b):
    return a%5 == b%5

# This tests that '35' is IN '(17,18,19,20,21)' by the EqMod5 operator
# which says two things are equal if their mod 5 values are equal. So,
# '35' is IN the list because 20%5==35%5 (0)
TestValueIN("test_value_in_002",(17,18,19,20,21),35,2,EqMod5)

# General Python objects wind up being tested as strings
TestSection("Objects")
TestValueEQ("test_value_object_001",[1,'2',3],"[1, '2', 3]")
TestValueLT("test_value_object_002","apple","carrot")
TestValueLT("test_value_object_003",(1,1,0.00000471),(1,1,0.00000501))

Exit()
