# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: common.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 4/9/2010
 description:
    Common utilities.

"""

import sys
import subprocess
from . import log


def sexe(cmd,ret_output=False,echo = False):
    """
    Helper for executing shell commands.
    """
    if echo:
        log.info("[exe: %s]" % cmd)
    if ret_output:
        p = subprocess.Popen(cmd,
                             shell=True,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.STDOUT)
        res =p.communicate()[0]
        return p.returncode,res
    else:
        return subprocess.call(cmd,shell=True)

class WallTimer(object):
    def __init__(self,tag):
        self.tag = tag
    def start(self):
        from time import time
        self.__start = time()
    def stop(self):
        from time import time
        self.__end   = time()
    def get_elapsed(self):
        return self.__end - self.__start
    def __str__(self):
        return "%s %s (s)" % (self.tag,repr(self.get_elapsed()))

