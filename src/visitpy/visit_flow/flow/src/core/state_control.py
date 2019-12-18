# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: state_control.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 10/14/2010
 description:
    State space abstractions.

"""

import copy

class StateVector(object):
    """
    Holds an instance of a state vector.

    State vectors represent an index into to a state space.
    """
    def __init__(self,idx,vec):
        self.idx = idx
        self.vec = copy.deepcopy(vec)
    def __str__(self):
        return "[%d]:%s" % (self.idx, str(self.vec))
    def index(self):
        return self.idx

class Dimension(object):
    """
    Represents a state space dimension with a given name and extent.
    """
    def __init__(self,name,extent):
        self.name = name
        self.extent = extent

class StateSpace(object):
    """
    Used to define a state space.
    """
    def __init__(self,dims = None):
        self.dims = {}
        if not dims is None:
            for k,v in list(dims.items()):
                self.add_dimension(k,v)
    def __len__(self):
        """
        Returns the number of dimensions in the state space.
        """
        return len(list(self.dims.keys()))
    def add_dimension(self,name,extent):
        """
        Adds a dimension with the given name an extent.
        """
        self.dims[name] = Dimension(name,extent)
    def has_dimension(self,name):
        """
        Returns if the state space contains a dimension with
        the given name.
        """
        return name in list(self.dims.keys())
    def remove_dimension(self,name):
        """
        Removes a dimension with the given name if it exists.
        """
        if self.has_dimension(name):
            del self.dims[name]
    def dimensions(self):
        """
        Returns a list with the active dimensions.
        """
        return list(self.dims.values())

class StateVectorGenerator(object):
    """
    Generates an ordering of state vectors that sweep a given state space.

    Current modes: full_walk, padded_index
    """
    def __init__(self,space,mode = "full_walk"):
        self.space   = space
        self.mode    = mode
        self.length  = 0
        self.ndims   = len(self.space)
        self.names   = [ d.name for d in self.space.dimensions()]
        self.extents = [ d.extent for d in self.space.dimensions()]
        if self.mode == "full_walk":
            self.__init_full_walk()
        elif self.mode == "padded_index":
            self.__init_padded_index()
    def __len__(self):
        """
        Returns the length of the state vector index space provided
        by the generator.
        """
        return self.length
    def __getitem__(self,idx):
        """
        Generates a state vector for the [] operator.
        """
        if isinstance(idx,slice):
            return [ self[i] for i in range(*idx.indices(self.length))]
        if not isinstance(idx,int):
            raise TypeError
        if idx < 0 or idx >= self.length:
            raise IndexError
        if self.mode == "full_walk":
            res = self.__get_full_walk(idx)
        elif self.mode == "padded_index":
            res = self.__get_padded_index(idx)
        return res
    def __init_full_walk(self):
        self.length = 1
        for e in self.extents:
            self.length = self.length  * e
    def __init_padded_index(self):
        self.length = max(self.extents)
    def __get_padded_index(self,idx):
        idxs = []
        for i in range(self.ndims):
            if idx >= self.extents[i]:
                idxs.append(self.extents[i]-1)
            else:
                idxs.append(idx)
        return StateVector(idx,idxs)
    def __get_full_walk(self,idx):
        # Assume *left* is lsw,
        # Note: an orderd dict would make life easier
        idxs = [0]*self.ndims
        slvl = self.ndims - 1
        dv = 1
        for v in self.extents[:slvl]:
            dv *=v
        cidx = idx
        for lvl in range(slvl,-1,-1):
            idxs[lvl] = (cidx / dv) % self.extents[lvl]
            cidx = cidx % dv
            if lvl > 0:
                dv = dv / self.extents[lvl-1]
        return StateVector(idx,idxs)

