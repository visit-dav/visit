# ----------------------------------------------------------------------------
#  CLASSES: nightly
#  LIMIT: 2000
#  Test Case:  mfem.py 
#
#  Tests:      mfem files 
#
#  Notes: this calls the functions in 'mfemHelpers', which was copied from
#         the original 'mfem.py'. It only calls the non-lor tests.
#
#  Programmer: Kathleen Biagas 
#  Date:       Fri Oct 30, 2026
#
#  Modifications:
#
# ----------------------------------------------------------------------------

Source(tests_path("databases", "mfemHelpers"))

def main():
    test_input_mesh_files()
    test_mfem_expressions()
    test_input_mesh_boundary_topology()
    test_direct_mesh_open()
    test_quadrature_functions()

main()
Exit()
