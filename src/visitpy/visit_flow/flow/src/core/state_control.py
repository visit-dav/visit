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
            for k,v in dims.items():
                self.add_dimension(k,v)
    def __len__(self):
        """
        Returns the number of dimensions in the state space.
        """
        return len(self.dims.keys())
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
        return name in self.dims.keys()
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
        return self.dims.values()

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
            return [ self[i] for i in xrange(*idx.indices(self.length))]
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

