# ----------------------------------------------------------------------------
#  CLASSES: nightly
#  LIMIT: 2000
#  Test Case:  mfem_lor1.py 
#
#  Tests:      mfem files 
#
#  Notes: this calls the functions in 'mfemHelpers', which was copied from
#         the original 'mfem.py'. It only calls the lor_scalar and lor_mesh tests.
#
#  Programmer: Kathleen Biagas 
#  Date:       Fri Oct 30, 2026
#
#  Modifications:
#
# ----------------------------------------------------------------------------

Source(tests_path("databases", "mfemHelpers"))

def main():
    test_lor_mesh_controls()
    test_lor_scalar_field_controls()

main()
Exit()
