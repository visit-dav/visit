# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: filter_graph.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 10/14/2010
 description:
    Core filter graph data structures.

"""

from queue import Queue
from .property_tree import PropertyTree
from .errors import *

class Filter(object):
    """
    Base class for filters.
    """
    def __init__(self, name, context):
        if not self.__validate():
            raise InvalidFilterDefinitionError(self)
        self.name     = name
        self.params   = PropertyTree(init=self.default_params)
        self.context  = context
    def input(self,id):
        """
        Fetches an input obj.

        Usage in subclass:
          Fetch by index:
            self.input(0) # first input
          Fetch by name:
            self.input("named_port") # input from port "named_port"
        """
        if isinstance(id,int):
            id = self.input_ports[id]
        return self.__inputs[id]
    def execute(self):
        """
        Main filter entry point.
        """
        return None
    def set_state_vector(self,svec):
        """
        Sets the filter node's state vector.

        (Used by workspace runtime.)
        """
        self.state_vector = svec
    def set_params(self,params):
        self.params.update(params)
    def set_inputs(self,inputs):
        """
        Sets the filter node's inputs.

        (Used by workspace runtime.)
        """
        self.__inputs = inputs
    def set_context(self,context):
        """
        Sets the filter node's context.

        (Used by workspace runtime.)
        """
        self.context = context
    def number_of_input_ports(self):
        """
        Returns the number of input ports.
        """
        return len(self.input_ports)
    def parameters(self):
        return self.params.properties()
    def __getitem__(self,path):
        """
        Fetches an entry from the params PropertyTree.
        """
        return self.params[path]
    def __setitem__(self,path,obj):
        """
        Sets an entry in the params PropertyTree.
        """
        self.params[path] = obj
    def __str__(self):
        """
        String pretty print.
        """
        res  = "%s:[%s]" % (self.name, self.filter_type)
        res += "(%s)"  % str(self.context)
        return res
    @classmethod
    def default_parameters(cls):
        if isinstance(cls.default_params,PropertyTree):
            return cls.default_params.properties()
        else:
            return dict(cls.default_params)
    @classmethod
    def info(cls):
        """
        Returns class level pretty print string.
        """
        vals = cls.__check_fields()
        res = "Filter Name     = "
        if vals[0]:
            res += cls.filter_type
        else:
            res += "{error: missing 'filter_type'}"
        res += "\nInput Port(s)   = "
        if vals[1]:
            res +=  str(cls.input_ports)
        else:
            res += "{error: missing 'input_ports'}"
        res += "\nDefault Params: "
        if vals[2]:
            res += str(cls.default_params)
        else:
            res += "{error: missing 'default_params'}"
        res += "\nOutput Port:    "
        if vals[3]:
            res += str(cls.output_port)
        else:
            res += "{error: missing 'output_port'}"
        return res + "\n"
    @classmethod
    def __validate(cls):
        """
        Helper that checks if the class members required by the Filter
        interface are defined.
        """
        vals = cls.__check_fields()
        if False in vals:
            return False
        return True
    @classmethod
    def __check_fields(cls):
        """
        Returns a list of boolean values that represent which of the class
        members required by the Filter interface are defined.
        """
        cdir = dir(cls)
        return [val in cdir for val in ["filter_type",
                                        "input_ports",
                                        "default_params",
                                        "output_port"]]


class FilterGraph(object):
    def __init__(self):
        self.filters    = {}
        self.nodes      = {}
        self.edges_out  = {}
        self.edges_in   = {}
        self.node_count = -1
    def register_filter(self,filter_class):
        """
        Registers a filter class.
        """
        if not issubclass(filter_class,Filter):
            raise InvalidFilterDefinitionError(filter_class)
        self.filters[filter_class.filter_type] = filter_class
    def connect(self,src_name,des_name,port_name):
        """
        Connects two nodes in the filter graph.
        """
        src_node = self.nodes[src_name]
        des_node = self.nodes[des_name]
        if isinstance(port_name,int):
            port_name = des_node.input_ports[port_name]
        if not port_name in des_node.input_ports:
            raise InvalidInputPortError(src_name,des_name,port_name)
        if not src_node.output_port:
            raise InvalidOutputPortError(src_name,des_name,port_name)
        self.edges_in[des_name][port_name] =  src_name
        self.edges_out[src_name].append(des_name)
    def has_node(self,node_name):
        """
        Checks if a node with the given name exists in the filter graph.
        """
        return node_name in list(self.nodes.keys())
    def add_node(self,filter_type,node_name=None,node_params=None,node_context=None):
        """
        Adds a new filter nodeinstance of the given type to the filter graph.
        """
        if not filter_type in list(self.filters.keys()):
            raise UnregisteredFilterError(filter_type)
        if node_name is None or self.has_node(node_name):
            node_name = self.__next_node_name(filter_type)
            while self.has_node(node_name):
                node_name  = self.__next_node_name(filter_type)
        fcls = self.filters[filter_type]
        node = fcls(node_name,node_context)
        if not node_params is None:
            node.set_params(node_params)
        if not node_context is None:
            node.set_context(node_context)
        # add node to the appropriate edge maps
        if node.number_of_input_ports() > 0:
            self.edges_in[node.name] = {}
            for port in node.input_ports:
                self.edges_in[node.name][port] = None
        if node.output_port:
            self.edges_out[node.name] = []
        self.nodes[node.name] = node
        return node
    def remove_node(self,inst_name):
        """
        Removes a node from the filter graph.
        """
        # make sure we have a node with the given name
        if not self.has_node(inst_name):
            raise UnknownFilterNodeError(inst_name)
        # get the node
        node = self.nodes[inst_name]
        # clean up any edges
        if node.number_of_input_ports() > 0:
            del self.edges_in[inst_name]
        if node.output_port:
            del self.edges_out[inst_name]
        del self.nodes[inst_name]
    def to_dict(self):
        res = {"filter_types":{},
               "nodes":{},
               "connections":[]}
        for k,v in list(self.filters.items()):
            res["filter_types"][k] = {"input_ports":    v.input_ports,
                                      "default_params": v.default_params,
                                      "output_port":    v.output_port}
        for nname, node in list(self.nodes.items()):
            nres = {"type":    node.filter_type,
                    "params":  node.params.properties(),
                    "context": node.context.name}
            res["nodes"][nname] = nres
        for des_name, ein_map in list(self.edges_in.items()):
            for port_name, src_name in list(ein_map.items()):
                if not src_name is None:
                    conn = {"from":src_name,"to":des_name,"port":port_name}
                    res["connections"].append(conn)
        return res
    def save_dot(self,fname):
        r   = "digraph G {\n"
        for nname, node in list(self.nodes.items()):
            for e in self.edges_out[nname]:
                if nname[0] == ":" :
                    nname = "VAR_FETCH_"  + nname[1:]
                r += "%s -> %s\n" %(nname,e)
        r += "}\n"
        open(fname,"w").write(r)
        return r
    def get_node(self,inst_name):
        """
        Returns the node with the given name, if it exists in the filter graph.
        """
        if inst_name in list(self.nodes.keys()):
            return self.nodes[inst_name]
        return None
    def __str__(self):
        """
        String pretty print.
        """
        res = "Registered Filter Types:\n"
        for f in list(self.filters.values()):
            res += f.info()  + "\n"
        res += "Active Filter Nodes:\n"
        for n in list(self.nodes.values()):
            res += str(n) + "\n"
        return res
    def __next_node_name(self,filter_type=""):
        """
        Helper used to generates unique node names.
        """
        self.node_count += 1
        res = "node"
        if filter_type != "":
            res += "_%s" % filter_type
        res += "_%04d" % self.node_count
        return res





