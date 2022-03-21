#!/usr/bin/env python3

# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.


# https://martinheinz.dev/blog/57

import os, sys, tarfile

#
# Output a usage message with optional error messag
#
def usage(errmsg=""):
    print(\
"""
%s
Usage: Basic tar-compatible archiver for VisIt test data archives
    using Python3 tarfile & lzma modules with (xz) compression.

    Create: visit_pytar c|C <batch>_test_data.tar.xz
    List:   visit_pytar t   <batch>_test_data.tar.xz
    Expand: visit_pytar x|X <batch>_test_data.tar.xz

    Where <batch> is an arbitrary string naming a batch of related
    data files. Usually, it is used to indicate the underlying data
    format all the files use.

    For Create, (c) option, a matching directory name of the form
    <batch>_test_data must already exist. Lowercase c Create fails
    if <batc>_test_data.tar.xz already exists. Uppercase C ignores
    this check.

    For Expand, Lowercase x fails if <batch>_test_data already
    exists. Uppercase X ignores this check.

    This tooling is provided only as a convenience. More advanced
    archiving and compression operations are likely possible using
    your platform's native tooling. For example, on many systems
    the command...

    tar cvf - my_test_data | xz -9e -T0 - > my_test_data.tar.xz

    will use tar and xv commands in a pipe with maximal compression
    and as many threads as actual hardware cores.
"""%("***Error***: %s\n"%errmsg if errmsg else "")
)
    sys.exit(1)

if len(sys.argv) == 1:
    usage()
elif sys.version_info.major != 3:
    usage('Python version 3 is required')

elif 'c' in sys.argv[1].lower(): # Create
    if not sys.argv[2].endswith('_test_data.tar.xz'):
        usage('"%s" must be of the form <dbformat>_test_data.tar.xz'%sys.argv[2])
    if 'c' in sys.argv[1] and os.path.exists(sys.argv[2]):
        usage('%s must not already exist'%sys.argv[2])
    if not os.path.isdir(sys.argv[2][:-7]): 
        usage('"%s" must be a pre-existing directory'%sys.argv[2][:-7])
    tf = tarfile.open(sys.argv[2], 'w:xz', preset=9)
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
