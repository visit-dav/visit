# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

import os, sys, tarfile

def usage(msg=""):
    print(\
"""
%s
Simple xz archiving tool using Python 3 tarfile module:

    Create: visit_pytar c <archive> <input dir>
    List:   visit_pytar t <archive>
    Expand: visit_pytar x <archive>

    <archive> is a file name of the form <dbformat>_test_data.tar.xz
    <input dir> is a directory name of the form <dbformat>_test_data

    This tooling is provided only as a convenience.
    
    More advanced archiving and compression operations are likely
    possible using your platform's native tooling.
"""%("***%s***\n"%msg if msg else "")
)
    sys.exit(1)

if len(sys.argv) == 1:
    usage()
elif sys.version_info.major != 3:
    usage('Python version 3 is required')
elif 'c' in sys.argv[1]: # Create
    if not sys.argv[2].endswith('_test_data.tar.xz'):
        usage('"%s" must be of the form <dbformat>_test_data.tar.xz'%sys.argv[2])
    if os.path.exists(sys.argv[2]):
        usage('%s must not already exist'%sys.argv[2])
    if not os.path.isdir(sys.argv[3]): 
        usage('"%s" must be a directory'%sys.argv[3])
    if not sys.argv[3].endswith('_test_data'):
        usage('"%s" must be of the form <dbformat>_test_data'%sys.argv[3])
    tf = tarfile.open(sys.argv[2], 'w:xz')
    tf.add(sys.argv[3])
elif 't' in sys.argv[1]: # List
    tf = tarfile.open(sys.argv[2], 'r')
    tf.list()
elif 'x' in sys.argv[1]: # Expand
    tf = tarfile.open(sys.argv[2], 'r')
    tf.extractall()
tf.close()
sys.exit(0)
