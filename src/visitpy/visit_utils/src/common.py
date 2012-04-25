#*****************************************************************************
#
# Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
# Produced at the Lawrence Livermore National Laboratory
# LLNL-CODE-442911
# All rights reserved.
#
# This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
# full copyright notice is contained in the file COPYRIGHT located at the root
# of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
#
# Redistribution  and  use  in  source  and  binary  forms,  with  or  without
# modification, are permitted provided that the following conditions are met:
#
#  - Redistributions of  source code must  retain the above  copyright notice,
#    this list of conditions and the disclaimer below.
#  - Redistributions in binary form must reproduce the above copyright notice,
#    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
#    documentation and/or other materials provided with the distribution.
#  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
#    be used to endorse or promote products derived from this software without
#    specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
# ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
# LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
# DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
# SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
# CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
# LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
# OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
# DAMAGE.
#*****************************************************************************
"""
 file: common.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 4/9/2010
 description:
    Common utilities and base setup for the visit_utils module.

"""

import os
import sys
import subprocess
import socket
import re
import imp
import datetime

from property_tree import PropertyTree

try:
    import visit
    __visit_imported = True
except:
    __visit_imported = False


#
# Some functions wrap calls to the visit module, however not all require
# it to exist.  Use @require_visit decorator to provide a friendly
# exception if the visit module needed by not avalaible.
#

def require_visit(fn):
    """ Decorator for functions that require the visit module. """
    def run_fn(*args,**kwargs):
        # note: this check happens in the 'common' module but that should
        # be sufficent - if we can't load it here we wont be able to in other
        # namespaces.
        if not __visit_imported:
            raise VisItException("Could not import visit module")
        return fn(*args,**kwargs)
    return run_fn

class VisItException(Exception):
    """ Generic exception class for VisIt errors. """
    def __init__(self,msg):
        self.msg = msg
    def __str__(self):
        return "<VisIt Error> %s" % self.msg

#
# Misc Helpers
#

def timestamp(t=None,sep="_"):
    """ Creates a timestamp that can easily be included in a filename. """
    if t is None:
        t = datetime.datetime.now()
    sargs = (t.year,t.month,t.day,t.hour,t.minute,t.second)
    sbase = "".join(["%04d",sep,"%02d",sep,"%02d",sep,"%02d",sep,"%02d",sep,"%02d"])
    return  sbase % sargs

def uname():
    """ Return username from the env. """
    return os.environ["USER"]

def hostname(full=True):
    """ Returns the base hostname of the current machine. """
    host_name = socket.gethostname()
    if full:
        return host_name
    else:
        return re.compile("[a-zA-z]*").match(host_name).group()

def lsearch(l,pattern):
    """ Search for items in list l that have substring match to pattern. """
    rvals = []
    for v in l:
        if not v.find(pattern) == -1:
            rvals.append(v)
    return rvals

def sexe(cmd,ret_output=False,echo = False):
    """ Helper for executing shell commands. """
    if echo:
        print "[exe: %s]" % cmd
    if ret_output:
        p = subprocess.Popen(cmd,
                             shell=True,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.STDOUT)
        res =p.communicate()[0]
        return p.returncode,res
    else:
        return subprocess.call(cmd,shell=True)

def define_module(module_name,module_script,parent_dict=None):
    """ Dynamically defines a python module. """
    if module_name in sys.modules:
        module = sys.modules[module_name]
    else:
        # create a new module
        module = imp.new_module(module_name)
        sys.modules[module_name] = module
        module.__file__ = "<%s>" % module_name
        module.__loader = None
    exec(module_script,module.__dict__,module.__dict__)
    if not parent_dict is None:
        parent_dict[module_name] = __import__(module_name)
    return module

def load_params(params_file,module_name="params"):
    """ Helper to load a python file used to provide input parameters. """
    pscript = "from visit_utils import *\n" + open(params_file).read()
    params = define_module(module_name,pscript)
    # auto setup 'root' PropertyTree
    if not 'root' in dir(params):
        root = PropertyTree()
        for val in dir(params):
            if val.count("__") != 2:
                root[val] = params.__dict__[val]
        params.__dict__['root'] = root
    return params


