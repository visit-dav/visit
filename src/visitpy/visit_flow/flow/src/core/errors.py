# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: errors.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 3/24/2012
 description:
    Exception infrastructure for flow.
"""

from . import log

# base error class
class BaseError(Exception):
    """
    Base error class for exceptions in the 'flow' module.
    """
    categories = []
    def __init__(self,msg):
        self.msg = msg
    def log(self):
        """
        Logs exception to the root and selected component logs.
        """
        log.error(self.msg,components = self.categories)
    def __str__(self):
        """
        String pretty print.
        """
        return "<%s>" % self.msg

# filter_graph exceptions
class InvalidPortError(BaseError):
    """
    Base class exception used to represent errors related to Filter Graph
    port connections.
    """
    categories = ["filter_graph"]
    def __init__(self,src_name,des_name,port_name):
        self.src  = src_name
        self.port = port_name
        self.des  = des_name
        msg = "Cannot connect %s to %s:%s" % (self.src,self.des,self.port)
        super(InvalidPortError,self).__init__(msg)
        self.log()

class InvalidInputPortError(InvalidPortError):
    """
    Exception thrown by a FilterGraph to identify a connection to an invalid
    input port.
    """
    def __init__(self,src_name,des_name,port_name):
        super(InvalidInputPortError,self).__init__(src_name,des_name,port_name)
        self.msg += " (invalid input port name: '%s')" % self.port
        self.log()

class InvalidOutputPortError(InvalidPortError):
    """
    Exception thrown by a FilterGraph to identify connection from a nonexistent
    output port.
    """
    def __init__(self,src_name,des_name,port_name):
        super(InvalidOutputPortError,self).__init__(src_name,des_name,port_name)
        self.msg += " (%s does not have an output port)" % self.src
        self.log()

class UnknownFilterNodeError(BaseError):
    """
    Exception thrown by a FilterGraph to identify an attempt to access a
    nonexistent filter node.
    """
    categories = ["filter_graph"]
    def __init__(self,filter_name):
        self.filter_name = filter_name
        msg = "Unknown filter node: '%s'" % self.filter_name
        super(UnknownFilterNodeError,self).__init__(msg)
        self.log()


class UnregisteredFilterError(BaseError):
    """
    Exception thrown by a FilterGraph to identify an attempt to create a filter
    node with an unregistered type.
    """
    categories = ["filter_graph"]
    def __init__(self,filter_type):
        self.filter_type = filter_type
        msg = "Unknown filter type: '%s'" % self.filter_type
        super(UnregisteredFilterError,self).__init__(msg)
        self.log()


class InvalidFilterDefinitionError(BaseError):
    """
    Exception thrown by a Filter subclass instance when the class does not
    implement the class members required by the Filter interface.
    """
    categories = ["filter_graph"]
    def __init__(self,obj):
        self.obj  = obj
        msg = "<InvalidFilterDefinitionError>\n%s"
        if issubclass(self.obj,Filter):
            msg += str(self.node.info())
        else:
            msg += " (Does not inherit from 'Filter')"
        super(InvalidFilterDefinitionError,self).__init__(msg)
        self.log()

# registry exceptions
class InvalidRegistryKeyError(BaseError):
    """
    Exception thrown by a Registry to identify an attempt to fetch a
    nonexistent entry key.
    """
    categories = ["registry"]
    def __init__(self,key):
        self.key = key
        msg = "Invalid registry key: '%s'" % self.key
        super(InvalidRegistryKeyError,self).__init__(msg)
        self.log()

# workspace exceptions
class UnregisteredContextError(BaseError):
    """
    Exception thrown by a Workspace to identify an attempt to create a context
    with an unregistered type.
    """
    categories = ["workspace"]
    def __init__(self,context_type):
        self.context_type = context_type
        msg = "Unknown context type: '%s'" % self.context_type
        super(UnregisteredContextError,self).__init__(msg)
        self.log()
