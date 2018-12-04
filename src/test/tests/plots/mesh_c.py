# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  mesh_c.py
#
#  Tests:      Runs the mesh.py test but with compression turned on.
#              mesh.py tests for existence of 'useCompression' variable
#              and turns on compression if it exists. We expect identical
#              output from the tests but because this py file has a different
#              name, it will generate them a subdir by the name 'mesh_c'
#              and look for baselines likewise. So, in baseline dir,
#              the mesh_c dir is a symlink to scalable.
#
#  Programmer: Mark C. Miller
#  Date:       19Aug08
#
# ----------------------------------------------------------------------------

useCompression = 1
Source(tests_path("plots","mesh.py"))
