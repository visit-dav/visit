# ----------------------------------------------------------------------------
#  CLASSES: nightly
#  LIMIT: 2000
#  Test Case:  mfem_lor2.py 
#
#  Tests:      mfem files 
#
#  Notes: this calls the functions in 'mfemHelpers', which was copied from
#         the original 'mfem.py'. It only calls the lor_vector tests.
#
#  Programmer: Kathleen Biagas 
#  Date:       Fri Oct 30, 2026
#
#  Modifications:
#
# ----------------------------------------------------------------------------

Source(tests_path("databases", "mfemHelpers"))


def main():
    test_lor_vector_field_controls_close_up()
    test_lor_vector_field_controls()
    test_lor_vector_magnitude()

main()
Exit()
