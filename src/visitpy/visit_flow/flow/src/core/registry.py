# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: registry.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 10/14/2010
 description:
    Provides a reference counting object registry.

"""

from .filter_graph import Filter
from .errors import *
from . import log

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
        if key in list(self.entries.keys()):
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
        res = list(self.entries.keys())
        res.sort()
        return res
    def __str__(self):
        """
        String pretty print.
        """
        ekeys = list(self.entries.keys())
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
