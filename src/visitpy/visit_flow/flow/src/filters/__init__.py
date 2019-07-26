# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: __init__.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 10/14/2010
 description:
    Init for 'flow.filters'.

"""

import cmd
import file_ops
# requires install of imagemagick command line tools.
import imagick
import script_pipeline

# check for import error only ...
try:
    # requires numpy
    import npy_ops
except ImportError as e:
    pass

# check for import error only ...
try:
    # these require pyopencl
    import pyocl_env
    import pyocl_ops
    import pyocl_batch
    import pyocl_compile
except ImportError as e:
    pass

from management import *