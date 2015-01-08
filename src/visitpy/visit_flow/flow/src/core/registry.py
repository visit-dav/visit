#*****************************************************************************
#
# Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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
 file: registry.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 10/14/2010
 description:
    Provides a reference counting object registry.

"""

from filter_graph import Filter
from errors import *
import log

# logging helper for registry
def info(msg):
    log.info(msg,"registry")

class RegistryEntry(object):
    """
    Holds a registry entry's key, object and reference count.
    """
    def __init__(self,key,obj,uref):
        self.key     = key
        self.obj     = obj
        self.uref    = uref

class Registry(object):
    """
    Object Registry.

    Holds named reference counted objects.
    """
    def __init__(self):
        self.entries = {}
    def add_entry(self,key,obj,uref=-1):
        """
        Adds an entry to the Registry.

        Reference counting is disabled if uref=-1 .
        """
        info("Registry add_entry:    %s uref=%d" % (key,uref))
        self.entries[key] = RegistryEntry(key,obj,uref)
    def fetch_entry(self,key,direct_fetch = False):
        """
        Fetches the registry entry with the given key.

        direct_fetch=True allows access to an entry without modifying its
        reference count.

        Raises `InvalidRegistryKeyError' if the given key does not
        map to an entry.
        """
        info("Registry fetch_entry:  %s" % key)
        # get obj from dict using key.
        # dec uref & if it equals zero remove it from the dict.
        # note: negative uref will live forever.
        if key in self.entries.keys():
            res = self.entries[key]
            if res.uref > 0 and not direct_fetch:
                res.uref +=-1
                if res.uref == 0:
                    info("Registry remove_entry: %s uref=0" % key)
                    try:
                        res.obj.release()
                    except:
                        pass
                    del self.entries[key]
            return res.obj
        raise InvalidRegistryKeyError(key)
    def clear(self):
        """
        Removes all registry entires.
        """
        self.entries = {}
    def keys(self):
        """
        Returns a list of keys for all active entires.
        """
        res = self.entries.keys()
        res.sort()
        return res
    def __str__(self):
        """
        String pretty print.
        """
        ekeys = self.entries.keys()
        ekeys.sort()
        res = "Registry Entries:\n"
        for ek in ekeys:
            re = self.entries[ek]
            res += " %s\n" % str(re)
        return res

class RegistrySource(Filter):
    """
    Special filter that provides convenient access to registry entires
    from the enclosing workspace.
    """
    filter_type    = "<registry_source>"
    input_ports    = []
    default_params = {}
    output_port    = True
    def execute(self):
        # the instance name determines the reg entry_key
        key = self.name
        return self.context.registry_fetch(key)
