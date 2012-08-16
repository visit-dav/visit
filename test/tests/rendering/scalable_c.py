# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  scalable_c.py
#
#  Tests:      Runs the scalable.py test but with compression turned on 
#              scalable.py tests for existence of 'useCompression' variable
#              and turns on compression if it exists. We expect identical
#              output from the tests but because this py file has a different
#              name, it will generate them a subdir by the name 'scalable_c'
#              and look for baselines likewise. So, in baseline dir,
#              the scalable_c dir is a symlink to scalable.
#
#  Programmer: Mark C. Miller
#  Date:       19Aug08 
#
# ----------------------------------------------------------------------------

useCompression = 1
Source(tests_path("rendering","scalable.py"))
