# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

import os, sys, tarfile

#
# Output a usage message with optional error messag
#
def usage(errmsg=""):
    print(\
"""
%s
Usage: Basic archiver with xz compression using Python 3 tarfile
    module for VisIt test data archives:

    Create: visit_pytar c <batch>_test_data.tar.xz
    List:   visit_pytar t <batch>_test_data.tar.xz
    Expand: visit_pytar x|X <batch>_test_data.tar.xz (X=expand into)

    Where <batch> is an arbitrary string naming a batch of related
    data files. For Create, (c) option, a matching directory name
    of the form <batch>_test_data must already exist. Expand with
    lowercase x fails if <batch>_test_data already exists. Use X
    to override.

    This tooling is provided only as a convenience.
    
    More advanced archiving and compression operations are likely
    possible using your platform's native tooling.
"""%("***Error***: %s\n"%errmsg if errmsg else "")
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
    if not os.path.isdir(sys.argv[2][:-7]): 
        usage('"%s" must be a pre-existing directory'%sys.argv[2][:-7])
    tf = tarfile.open(sys.argv[2], 'w:xz')
    tf.add(sys.argv[2][:-7])
    tf.close()
elif 't' in sys.argv[1]: # List
    if not os.path.exists(sys.argv[2]):
        usage('%s does not exist'%sys.argv[2])
    tf = tarfile.open(sys.argv[2], 'r')
    tf.list()
    tf.close()
elif 'x' in sys.argv[1].lower(): # Expand
    if not os.path.exists(sys.argv[2]):
        usage('%s does not exist'%sys.argv[2])
    if 'x' in sys.argv[1] and os.path.isdir(sys.argv[2][:-7]):
        usage('%s already exists. Use uppercase X to expand into'%sys.argv[2][:-7])
    tf = tarfile.open(sys.argv[2], 'r')
    tf.extractall()
    tf.close()
else:
    usage('"%s" uncrecognized option'%sys.argv[1])
sys.exit(0)
