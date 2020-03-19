# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: property_tree.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 4/15/2010
 description:
    (almost too) flexible property tree class.

"""


class PropertyTree(object):
    def __init__(self,ptype ="tree",init = None):
        self._type = ptype
        self._locked = False
        if self._type == "tree":
            self._value = {}
            if not init is None:
                self.update(init)
        else:
            self._value = init
    def update(self,pval):
        if isinstance(pval,dict):
            for path,value in  list(pval.items()):
                self.add_property(path,value)
        else:
            for path,value in list(pval.properties().items()):
                self.add_property(path,value)
    def clear(self):
        self._locked = False
        if self._type == "tree":
            self._value = {}
        else:
            self._value = None
    def properties(self):
        res = {}
        if self._type == "tree":
            keys = list(self._value.keys())
            for k in keys:
                curr = self._value[k]
                if curr._type=="tree":
                    for p,v in list(curr.properties().items()):
                        res[k + "/" + p] = v
                else:
                    res[k] = curr._value
        return res
    def children(self):
        res = {}
        if self._type == "tree":
            keys = list(self._value.keys())
            keys.sort()
            for k in keys:
                curr = self._value[k]
                res[k] = curr
        return res
    def add_property(self,path,value=None):
        idx = path.find("/")
        if idx > 0:
            lpath = path[:idx]
            rpath = path[idx+1:]
            if not lpath in list(self._value.keys()):
                tree = PropertyTree()
                self._value[lpath] = tree
            else:
                tree = self._value[lpath]
            tree.add_property(rpath,value)
        else:
            if value is None:
                self._value[path] = PropertyTree()
            else:
                self._value[path] = PropertyTree("node",value)
    def has_property(self,path):
        node = self.fetch_property(path)
        return not node is None
    def remove_property(self,path):
        # find the proper node in the tree and remove it
        idx = path.find("/")
        if idx > 0:
            lpath = path[:idx]
            rpath = path[idx+1:]
            tree = self._value[lpath]
            tree.remove_property(rpath)
        elif path in list(self._value.keys()):
            del self._value[path]
    def lock(self):
        self._locked = True
        if self._type == "tree":
            for v in list(self._value.values()):
                v.lock()
    def unlock(self):
        self._locked = False
        if self._type == "tree":
            for v in list(self._value.values()):
                v.unlock()
    def __getitem__(self,path):
        node = self.fetch_property(path)
        if node is None:
            if self._locked:
                raise AttributeError(path)
            self.add_property(path)
        node = self.fetch_property(path)
        if node._type == "tree":
            return node
        else:
            return node._value
    def __setitem__(self,path,obj):
        node = self.fetch_property(path)
        if node is None:
            if self._locked:
                raise AttributeError(path)
            self.add_property(path,obj)
        else:
            node._type  = "node"
            node._value = obj
    def fetch_property(self,path):
        idx = path.find("/")
        if idx > 0:
            lpath = path[:idx]
            if lpath in list(self._value.keys()):
                rpath = path[idx+1:]
                tree = self._value[lpath]
                return tree.fetch_property(rpath)
            return None
        elif path in list(self._value.keys()):
            return self._value[path]
        else:
            return None
    def __str__(self):
        return self.__gen_string("")
    def __gen_string(self,path):
        res = ""
        if self._type == "tree":
            for k in list(self._value.keys()):
               npath = path + k + "/"
               res +=  self._value[k].__gen_string(npath)
        else:
            res = path + "%s:%s\n" % (self._type,str(self._value))
        return res
    def __getattr__(self, name):
        if name.startswith("__") and name.endswith("__"):
            raise AttributeError(name)
        try:
            return self.__getitem__(name)
        except KeyError:
                raise AttributeError(name)
    def __setattr__(self, name,obj):
        if name == "_value" or name == "_type" or name == "_locked":
            self.__dict__[name] = obj
        else:
            self.__setitem__(name, obj)



