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

from . import cmd
from . import file_ops
# requires install of imagemagick command line tools.
from . import imagick
from . import script_pipeline

# check for import error only ...
try:
    # requires numpy
    from . import npy_ops
except ImportError as e:
    pass

# check for import error only ...
try:
    # these require pyopencl
    from . import pyocl_env
    from . import pyocl_ops
    from . import pyocl_batch
    from . import pyocl_compile
except ImportError as e:
    pass

from .management import *