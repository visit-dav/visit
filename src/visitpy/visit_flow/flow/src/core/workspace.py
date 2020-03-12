# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: workspace.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 10/14/2010
 description:
    Workspace data flow abstractions.

"""

import sys
import hashlib
import imp
import traceback

from .registry      import *
from .filter_graph  import *
from .state_control import *

from ..parser      import *

from . import log

# logging helper for workspace
def info(msg):
    log.info(msg,"workspace")

def define_module(module_name,module_script,parent_dict=None):
    if module_name in sys.modules:
        module = sys.modules[module_name]
    else:
        module = imp.new_module(module_name)
        sys.modules[module_name] = module
        module.__file__ = "<%s>" % module_name
        module.__loader = None
    exec(module_script,module.__dict__,module.__dict__)
    if not parent_dict is None:
        parent_dict[module_name] = __input__(module_name)
    return module


class Context(object):
    """
    Base class for contexts.
    """
    context_type   = "Context"
    default_params = {}
    def __init__(self, workspace, name, params = None, parent = None):
        self.__workspace = workspace
        self.name     = name
        self.params   = PropertyTree(init=self.default_params)
        self.parent   = parent
        if not params is None:
            self.params.update(PropertyTree(init=params))
    def add_context(self,context_type,context_name, params = None):
        """
        Adds a context to the workspace.

        This instance is the parent of the new context.
        """
        return self.__workspace.add_context(context_type,context_name,params,parent=self)
    def add_filter(self,filter_type,filter_name=None,params=None):
        """
        Adds a node to the workspace.

        This instance is used as the context of the new node.
        """
        return self.__workspace.add_filter(filter_type,filter_name,params,self)
    def has_filter(self,filter_name):
        return self.__workspace.has_filter(filter_name)
    def filter_names(self,filter_name):
        return self.__workspace.filter_names()
    def add_registry_source(self,
                            entry_key,
                            obj,
                            uref=-1,
                            filter_type=None,
                            filter_name=None,
                            context=None):
        """
        Adds a data object to the registry and creates a source node for this
        data object.

        This instance is used as the context of the new node.
        """
        return self.__workspace.add_registry_source(entry_key,obj,uref,
                                                    filter_type,filter_name,
                                                    self)
    def connect(self,src_name,des_port):
        """
        Connects filter nodes in the workspace.

        Convenience method for interacting with contexts.
        """
        return self.__workspace.connect(src_name,des_port,self)
    def remove_filter(self,fitler_name):
        """
        Removes a filter node from the workspace.

        Convenience method for interacting with contexts.
        """
        return self.__workspace.remove_filter(fitler_name)
    def registry_add(self,key,obj,uref=-1):
        """
        Adds an entry to the workspace registry.

        Convenience method for interacting with contexts.
        """
        return self.__workspace.registry.add_entry(key,obj,uref)
    def registry_fetch(self,key):
        """
        Fetches an entry from the workspace registry.

        Registry reference count is not changed.
        """
        return self.__workspace.registry.fetch_entry(key,direct_fetch=True)
    def registry_keys(self):
        """
        Returns a list of keys of the active entires in the workspace's
        registry.
        """
        return self.__workspace.registry_keys()
    def parent_context(self,context_type=None,context_name=None):
        """
        Fetches a parent context with a given name or type.

        ctx.parent_context(context_name="root")
        ctx.parent_context(context_type="<default_context>")
        """
        if self.parent is None or (context_type is None and context_name is None):
            return self.parent
        elif not context_name is None:
            if self.parent.name == context_name:
                return self.parent
            else:
                return self.parent.parent_context(context_name=context_name)
        elif not context_type is None:
            if self.parent.context_type == context_type:
                return self.parent
            else:
                return self.parent.parent_context(context_type=context_type)
        return None
    def parameters(self):
        return self.params
    @classmethod
    def default_parameters(cls):
        if isinstance(cls.default_params,PropertyTree):
            return cls.default_params.properties()
        else:
            return dict(cls.default_params)
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
        return "%s:[%s]" % (self.name, self.context_type)


class Workspace(object):
    """
    Main data flow container abstraction.


    """
    def __init__(self):
        self.graph    = FilterGraph()
        self.registry = Registry()
        self.context_types = {}
        self.contexts      = {}
        self.contexts["<default_context>"] = Context(self,"<default_context>")
        self.register_filter(RegistrySource)
    def register_filter_module(self,filter_module):
        """
        Registers a set of filters and contexts exposed in a filter module.

        Registers Filter subclasses in a list named `fitlers'.
        Registers Context subclasses in a list named `context'.
        """
        mdir = dir(filter_module)
        if "filters" in mdir:
             for f in filter_module.filters:
                self.register_filter(f)
        if "contexts" in mdir:
            for ctx in filter_module.contexts:
                self.register_context(ctx)
    def register_filters(self,filters):
        """
        Helper
        """
        if "filters" in dir(filters):
            self.register_filter_module(filters)
        else:
            for f in filters:
                self.register_filter(f)
    def register_context(self,context):
        """
        Registers a new Context subclass for use.
        """
        self.context_types[context.context_type] = context
    def register_filter(self,filter_class):
        """
        Registers a new Filter subclass for use.
        """
        self.graph.register_filter(filter_class)
    def add_context(self,context_type,context_name,parent=None):
        """
        Adds a context to the workspace.
        """
        if context_type in list(self.context_types.keys()):
            ccls = self.context_types[context_type]
            res = ccls(self,context_name,parent)
            self.contexts[context_name] = res
            return res
        else:
            raise UnregisteredContextError(context_type)
    def add_filter(self,filter_type,name=None,params=None,context=None):
        """
        Adds a filter node instance to the workspace.
        """
        if context is None:
            context = self.get_context("<default_context>")
        return self.graph.add_node(filter_type,name,params,context)
    def add_registry_source(self,
                            entry_key,
                            obj,uref=-1,
                            filter_type=None,
                            filter_name=None,
                            context=None):
        """
        Adds a data object to the registry and creates a source node for this
        data object.
        """
        self.registry_add(entry_key,obj,uref)
        if filter_type is None:
            filter_type = "<registry_source>"
        if filter_name is None:
            filter_name = entry_key
        self.add_filter(filter_type,filter_name,context=context)
    def connect(self,src_name,des_port,context=None):
        """
        Connects filter nodes in the workspace.
        """
        # check for a reg source
        reg_src = src_name.startswith(":")
        if reg_src and not self.has_filter(src_name):
            # assume data is in the registry & auto add a reg source.
            info("Adding automatic registry source = %s" % src_name)
            self.add_filter("<registry_source>",src_name,context=context)
        if isinstance(des_port,str):
            des,port = des_port.split(":")
        else: # tuple or list
            des,port = des_port
        self.graph.connect(src_name,des,port)
    def remove_filter(self,filter_name):
        """
        Removes the filter node with the given name from the workspace.
        """
        return self.graph.filter_name(filter_name)
    def has_filter(self,filter_name):
        """
        Returns True if a filter node with the given name exists in
        the workspace.
        """
        return self.graph.has_node(filter_name)
    def filter_names(self):
        """
        Returns the names of the active filter nodes in the workspace.
        """
        return list(self.graph.nodes.keys())
    def has_context(self,context_name):
        """
        Returns True if a context with the given name exists in
        the workspace.
        """
        return context_name in list(self.contexts.keys())
    def get_context(self,context_name):
        """
        Returns the names of the active filter nodes in the workspace.
        """
        if context_name in list(self.contexts.keys()):
            return self.contexts[context_name]
        return None
    def get_filter(self,filter_name):
        """
        Returns the a filter node with the given name exists in the workspace.
        """
        return self.graph.get_node(filter_name)
    def registry_add(self,entry_key,obj,uref=-1):
        """
        Adds an entry to the workspace's registry.
        """
        return self.registry.add_entry(entry_key,obj,uref)
    def registry_fetch(self,entry_key):
        """
        Fetches an entry from the workspace's registry.
        """
        return self.registry.fetch_entry(entry_key)
    def registry_clear(self):
        """
        Clears all entries from the workspace's registry.
        """
        return self.registry.clear()
    def registry_keys(self):
        """
        Returns a list of keys of the active entires in the workspace's
        registry.
        """
        return list(self.registry.keys())
    def execution_plan(self):
        """
        Generates a workspace execution plan.
        """
        return ExecutionPlan(self.graph)
    def execute(self,states=None):
        """
        Executes a flow workspace for a given set of states.

        TODO:MORE INFO
        """
        if states is None:
            states = StateVector(0,[0])
        plan = self.execution_plan()
        if isinstance(states,StateVector):
            return self.__execute_single(plan,states)
        elif isinstance(states,list) or isinstance(states,StateVectorGenerator):
            for svec in states:
                self.__execute_single(plan,svec)
    def __execute_single(self,plan,svec):
        """
        Helper used to execute a flow workspace for a single state vector.
        """
        info("Execute single: StateVector = %s" % str(svec))
        rval = None
        tidx = 0
        for t in plan.traversals:
            info("Execute Traversal %d" % tidx)
            for v in t:
                # get filter node & # of refs
                node_name, uref = v
                node = self.graph.nodes[node_name]
                try:
                    # get inputs from registry
                    inputs = {}
                    msg  = "Execute: %s" % node_name
                    for port_name in node.input_ports:
                        src_name = self.graph.edges_in[node_name][port_name]
                        entry_key = str(svec) + ":" + src_name
                        msg += " (%s:%s)" % (port_name,entry_key)
                        data = self.registry.fetch_entry(entry_key)
                        inputs[port_name]  = data
                    node.set_inputs(inputs)
                    node.set_state_vector(svec)
                    info(msg)
                    res = node.execute()
                except Exception as e:
                    msg  = "Execute Error: %s" % node_name
                    exc_type, exc_value, exc_traceback = sys.exc_info()
                    emsg = traceback.format_exception(exc_type,
                                                      exc_value,
                                                      exc_traceback)
                    emsg = "".join(emsg)
                    info(msg)
                    info("\n<Traceback>\n" + emsg)
                    print(msg)
                    print("\n<Traceback>\n" + emsg)
                    raise e
                # if output exists, place in registry
                if not res is None:
                    entry_key = str(svec) + ":" + node.name
                    self.registry.add_entry(entry_key,res,uref)
                    rval= res
                tidx += 1
        return res
    def setup_expression_network(self,txt,ctx=None):
        """
        Uses the expression parser to setup the workspace from a user
        expression.
        """
        if ctx is None:
            ctx = self.get_context("<default_context>")
        Generator.parse_network(txt,ctx)
    @classmethod
    def load_workspace_script(cls,src=None,file=None):
        """
        Helper used to load a workspace from a python script.

        (Legacy Path)
        """
        if src is None and not filename is None:
            info("Loading workingspace from: %s" % os.path.abspath(file))
            src = open(file).read()
        module_name = hashlib.md5(src).hexdigest()
        res = define_module(module_name,src)
        # setup the workspace
        w = res.setup_workspace()
        return w;
    def to_dict(self):
        res = {"context_types":{},
               "contexts":{}}
        for k,v in list(self.context_types.items()):
            res["context_types"][k] = {"default_params":dict(v.default_parameters())}
        for k,v in list(self.contexts.items()):
            ctx = {"type":v.context_type,
                   "params": v.parameters().properties(),
                   "parent": None}
            if not v.parent is None:
                ctx["parent"] = v.parent.name
            res["contexts"][k] = ctx
        graph_res = self.graph.to_dict()
        res.update(graph_res)
        return res
    def load_dict(self,wdict):
        # for now assume the filters and contexts are installed
        # just create and hook up the filters
        for node_name, node in list(wdict["nodes"].items()):
            params = None
            ctx    = None
            if "params" in node:
                params = node["params"]
            if "context" in node:
                ctx = self.get_context(node["context"])
            self.add_filter(node["type"],node_name,params,ctx)
        for edge in wdict["connections"]:
            self.connect(edge["from"],[edge["to"],edge["port"]])


class ExecutionPlan(object):
    """
    Workspace execution plan.

    Provides info about graph traversals that is used to execute
    a data flow network.
    """
    def __init__(self,g):
        self.traversals = []
        self.untouched  = []
        # find src & sink nodes
        snks = []
        srcs = []
        for node in list(g.nodes.values()):
            if not node.output_port or len(g.edges_out[node.name]) == 0:
                snks.append(node.name)
            if node.output_port and not node.name in list(g.edges_in.keys()):
                srcs.append(node.name)
        tags = {}
        for name in list(g.nodes.keys()):
            tags[name] = 0
        # execute bf traversals from each snk
        for snk_name in snks:
            trav = []
            self.__visit(g,snk_name,tags,trav)
            if len(trav) > 0:
                self.traversals.append(trav)
            self.untouched = []
            for name, tag in list(tags.items()):
                if tag == 0:
                    self.untouched.append(name)
    def __visit(self,g,node_name,tag,trav):
        """
        Traversal visitor for graph topo-sort.
        """
        if tag[node_name] != 0 : return
        uref = 1
        tag[node_name] = 1
        node = g.nodes[node_name]
        if node.output_port:
            uref = max(1,len(g.edges_out[node_name]))
        if node.number_of_input_ports() > 0:
            for src_name in list(g.edges_in[node_name].values()):
                    if not src_name is None:
                        self.__visit(g,src_name,tag,trav)
                    else: # dangle?
                        uref = 0
        if uref > 0:
            trav.append((node_name, uref))
    def __str__(self):
        """
        String pretty print.
        """
        ntrav = 0
        res = "Execution Plan:\n# of Traversals = %d\n"  % len(self.traversals)
        for trav in self.traversals:
            res += "\n Traversal %d:\n" % ntrav
            for node_name,uref in trav:
                res += "  %s (%d)\n"  %(node_name,uref)
            res += "\n"
            ntrav +=1
        nut = 0
        res += "# of Untouched Filter Nodes = %d\n" % len(self.untouched)
        if  len(self.untouched) > 0:
            res += " Untouched Filter Nodes:\n"
        for node_name in self.untouched:
                res += "  %s\n"  %(node_name)
        return res

