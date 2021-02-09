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



test_script_src = "print 'Old Style Print!'\n"

TestText("simple_2to3_input",test_script_src)
TestText("simple_2to3_result",ConvertPy2to3(test_script_src))


#
# Test on-the-fly logic for visit.Source
#

test_script_file = "tgen_test_py2_script.py";
open(test_script_file,"w").write(test_script_src)

curr_val = visit_utils.builtin.GetAutoPy2to3()

visit_utils.SetAutoPy2to3(True)
visit.Source(test_script_file)
visit_utils.SetAutoPy2to3(curr_val)

Exit()
