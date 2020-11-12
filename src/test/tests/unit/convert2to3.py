# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  convert2to3.py
#
#  Tests:      Tests methods that help with converting python 2 style scripts
#              to python 3. 
#
#  Cyrus Harrison, Mon Jul 20 11:32:42 PDT 2020
# ----------------------------------------------------------------------------



test_script = "print 'Old Style Print!'\n"

TestText("simple_2to3_input",test_script)
TestText("simple_2to3_result",ConvertPy2to3(test_script))

Exit()