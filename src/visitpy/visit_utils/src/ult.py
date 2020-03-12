# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
file: ult.py
author: Cyrus Harrison (cyrush@llnl.gov)
created: 10/28/08

Utils for processing ultra curves.

"""
#*****************************************************************************
# modifications:
#  Cyrus Harrison, Tue Dec  9 08:08:42 PST 2008
#  Added Merger & Sample classes, refactored Curve class.
#  Added comments.
# 
#  Cyrus Harrison, Mon Jan 12 16:11:56 PST 2009
#  Changed Merger to use a simpler chain linking strategy.
# 
#  Cyrus Harrison, Fri Apr  3 14:58:03 PDT 2009
#  Added:
#   extents() method to the Curve class
#   clone(), dist(), and lerp() to the Sample class.
#  Changed: Reader and Writer to use class methods for load & save.
#  Changed: Merger to use class method for execute.
# 
#  Cyrus Harrison, Fri Jul 24 09:14:10 PDT 2009
#  Added  __getitem__ method to the Curve object to allow easy
#  index based access and slicing of samples.
# 
#  Cyrus Harrison, Tue Oct 12 15:40:21 PDT 2010
#  Added get_value and find_index methods to aid qplot curve renderer.
# 
#  Cyrus Harrison, Fri Oct 15 09:46:39 PDT 2010
#  Better parsing.
# 
#  Cyrus Harrison, Wed Oct 27 16:49:26 PDT 2010
#  Added interp, which lerps between samples.
# 
#  Cyrus Harrison, Mon Dec 13 10:37:28 PST 2010
#  Added support to convert curve into a numpy array.
# 
#  Cyrus Harrison, Wed Mar  7 13:21:23 PST 2012
#  Change numpy import style.
#
#  Cyrus Harrison, Thu Apr 17 16:04:18 PDT 2014
#  Add numpy guards and helper that plots all the curves in a database.
#
#*****************************************************************************

import sys
import math

using_numpy = False

try:
    import visit
except:
    pass

try:
    import numpy as npy
    using_numpy = True
except:
    pass

from .common import VisItException, require_visit

@require_visit
def plot(dbfile=None):
    if not dbfile is None:
        visit.OpenDatabase(dbfile)
    else:
        wi = visit.GetWindowInformation()
        dbfile = wi.activeSource
    md = visit.GetMetaData(dbfile)
    ncrvs = md.GetNumCurves()
    cnames = [md.GetCurves(i).name for i in range(ncrvs)]
    cnames.sort()
    for cname in cnames:
        visit.AddPlot("Curve",cname)
    visit.DrawPlots()

class Sample:
    """
    Holds a single sample.
    """
    def __init__(self,x,y):
        """
        Sample Constructor.
        """
        self.x = x
        self.y = y
    def clone(self):
        """
        Creates a copy of this sample.
        """
        return Sample(self.x,self.y)

    def dist(self,samp):
        """
        Calcs the L2-distance between two samples.
        """
        return math.sqrt((self.x-samp.x)*(self.x-samp.x) +
                         (self.y-samp.y)*(self.y-samp.y))
    def __lt__(self,b):
        """
        Less than operator.
        """
        return self.x < b.x
    def __gt__(self,b):
        """
        Greater than operator.
        """
        return self.x > b.x
    def __eq__(self,b):
        """
        Equal to operator.
        """
        return self.x == b.x and self.y == b.y
    def __hash__(self):
        return self.__str__().__hash__()
    def __str__(self):
        """
        Pretty print.
        """
        return "%s %s" % (str(self.x),str(self.y))
    def lerp(self,s,xval):
        """
        Lerp between this and another sample, given an x value.
        """
        if self.x == xval:
            return self.clone()
        elif s.x == xval:
            return sb.clone()
        else:
            x0 = self.x
            x1 = s.x
            y0 = self.y
            y1 = s.y
            return Sample(xval,y0 + (xval - x0) * (y1-y0)/(x1-x0))

class Curve(object):
    """
    Represents a curve - a collection of samples.
    """
    def __init__(self,name = "",samples = None):
        """
        Curve Constructor.
        """
        self.name    = name
        if samples is None:
            self.samples = []
        else:
            # Check if samples is an ndarray, if so
            # convert to a list of Samples.
            if using_numpy and isinstance(samples,npy.ndarray):
                lshape = len(samples.shape)
                if  lshape == 1:
                    r = samples.shape
                    samples = [ Sample(i,samples[i]) for i in range(r)]
                elif lshape  == 2:
                    r,c = samples.shape
                    samples = [ Sample(samples[i,0],samples[i,1]) for i in range(r)]
                else:
                    # error
                    msg = "Cannot convert ndarry w/ shape %s to Curve " % str(lshape)
                    raise VisItException(msg)
            self.samples = samples
        self.__xmap = None
    def first(self):
        """
        Returns the first sample.
        """
        return self.samples[0]
    def last(self):
        """
        Returns the last sample.
        """
        return self.samples[-1]
    def interp(self,xval):
        idx = 0
        nsamps = len(self)
        while idx < nsamps and xval > self.samples[idx].x:
            idx+=1
        if idx >= nsamps:
            return self.samples[-1].y
        elif self.samples[idx].x == xval or idx == 0:
            return self.samples[idx].y
        s0 = self.samples[idx-1]
        s1 = self.samples[idx]
        x0,y0 = s0.x,s0.y
        x1,y1 = s1.x,s1.y
        return (y1-y0)*(xval-x0)/(x1-x0)+y0
    def get(self,x=None,y=None):
        """
        Returns value given x or y coord.
        Note: Assumes curve is a function - x is unique but y does not
         have to be. So for a given y, this function can return multiple x
         values.
        """
        if not x is None:
            # lookup by x
            return self.__get_y(x)
        if not y is None:
            # lookup by y
            return self.__get_x(y)
        raise Exception
    def extents(self):
        x_min = min([s.x for s in self.samples])
        y_min = min([s.y for s in self.samples])
        x_max = max([s.x for s in self.samples])
        y_max = max([s.y for s in self.samples])
        return [x_min,x_max,y_min,y_max]
    def add_sample(self,x,y):
        # this invalidates the xmap
        self.__xmap = None
        return self.samples.append(Sample(x,y))
    def get_sample(self,index):
        if index <0 or index >= len(self.samples):
            return None
        return self.samples[index]
    def find_index(self,xval):
        idx = 0
        nsamps = len(self.samples)
        while idx < nsamps and xval > self.samples[idx].x:
            idx+=1
        if idx < nsamps and idx >0:
            return idx-1
        return None
    def ndarray(self):
        #assumes successful numpy import ...
        if using_numpy:
            return npy.array([[s.x,s.y] for s in self.samples])
        return None
    def __get_y(self,x):
        """
        Helper to get a y value given x.
        """
        if self.__xmap is None:
            self.__xmap = {}
            for i in range(len(self.samples)):
                self.__xmap[self.samples[i].x] = i
        return self.samples[self.__xmap[x]].y
    def __get_x(self,y):
        """
        Helper to get x values given a y.
        """
        res = []
        for s in self.samples:
            if s.y == y:
                res.append(s)
        return s
    def values(self):
        """
        Generator used to iterate over samples.
        """
        for s in self.samples:
            yield [s.x,s.y]
        return
    def __getitem__(self,idx):
        """
        Enables nice python syntatic sugar for slicing & accessing samples.
        """
        if isinstance(idx,slice):
            return [ self[i] for i in range(*idx.indices(len(self)))]
        if not isinstance(idx,int):
            raise TypeError
        if idx < 0 or idx >= len(self):
            raise IndexError
        s = self.samples[idx]
        return [s.x,s.y]
    def reverse_samples(self):
        """
        Reverses the order of the samples.
        """
        res_order = []
        nsamples = len(self.samples)
        i = nsamples -1
        while i >= 0:
            res_order.append(self.samples[i])
            i=i-1
        self.samples = res_order
    def __len__(self):
        """
        Returns the number of samples.
        """
        return len(self.samples)
    def __str__(self):
        """
        Pretty print.
        """
        res = "# %s\n" % self.name
        for s in self.samples:
            res += "%s\n" % str(s)
        return res + "\n"
    @classmethod
    def save(cls,fname,data,echo=False,append=False):
        """
        Saves a curve (or curves) to an ultra file.
        """
        if echo:
            print("[Creating: %s ]" % fname)
        ocmd = "w"
        if append:
            ocmd += "a"
        f = open(fname,ocmd)
        if isinstance(data,list):
            cid = 0
            for c in data:
                cls.__save_curve(f,c,"curve_%d" % cid)
                cid +=1
        elif isinstance(data,dict):
            for k, c in list(data.items()):
                cls.__save_curve(f,c,k)
        else:
            cls.__save_curve(f,data)
        f.close()
    @classmethod
    def __save_curve(cls,fobj,data, cname = ""):
        """
        Helper to save a single curve.
        """
        if isinstance(data,Curve):
            if data.name != "":
                cname = data.name
        if cname == "":
            cname = "curve"
        fobj.write("# %s\n"  % cname)
        if isinstance(data,Curve):
            for v in list(data.values()):
                fobj.write("%s %s\n" % (str(v[0]),str(v[1])))
        elif using_numpy and isinstance(data,npy.ndarray):
            for i in range(data.shape[0]):
                fobj.write("%s %s\n" % (str(data[i,0]),str(data[i,1])))
        else: # error unknown data type
            msg = "Cannot save curve w/ data object of type: %s" % repr(type(data))
            raise VisItException(msg)
    @classmethod
    def load(cls,fname):
        """
        Reads a curve or set of curves from an ultra file.
        """
        curves = []
        f = open(fname)
        curr = None
        lines = [l.strip() for l in f.readlines() if l.strip() != ""]
        nlines = len(lines)
        f.close()
        for i in range(nlines):
            l = lines[i]
            if l[0] == "#" and (i == nlines-1 or lines[i+1][0] != "#"):
                if curr is not None:
                    if curr.first() > curr.last():
                        curr.reverse_samples()
                    curves.append(curr)
                curr = Curve()
                curr.name = l[1:]
            elif l[0] != "#" and l.count("end") == 0:
                tok = [float(t) for t in l.split()]
                curr.samples.append(Sample(tok[0],tok[1]))
        if curr is not None:
            if curr.first() > curr.last():
                curr.reverse_samples()
            curves.append(curr)
        return curves 




class Merger(object):
    """
    Helper that merges multiple curve segments that share end points.
    """
    @classmethod
    def execute(self,cname,curves):
        """
        Merges the samples from a set of curves into a new curve.
        Treats each curve as a segment and sews things together at first() 
        & last() samples.
        """
        res = Curve(cname)
        # find sensible ordering starting with the curve that contains 
        # the first end point.
        pts = {}
        for c in curves:
            if not c.first() in list(pts.keys()):
                pts[c.first()] =  [0,c]
            if not c.last() in list(pts.keys()):
                pts[c.last()] =  [0,c]
            pts[c.first()][0] +=1
            pts[c.last()][0]  +=1
        curr = None
        for k,v in list(pts.items()):
            if v[0] == 1 and v[1].first() == k:
                curr = v[1]
                break
        if curr is None:
            raise VisItException("Could not find start point w/ edge degree=1!")
        res_order = [curr]
        rcurves  = []
        for c in curves:
            if c != curr:
                rcurves.append(c)
        # build chain from remaining curves
        while len(rcurves) > 0:
            found = False
            for c in rcurves:
                if curr.last() == c.last() or curr.first() == c.first():
                    c.reverse_samples()
                if c.first() == curr.last():
                    res_order.append(c)
                    rcurves.remove(c)
                    curr = c
                    found = True
                    break
            if not found:
                raise VisItException("Could not find curve join point!")
        for r in res_order:
            for s in r.samples:
                if len(res.samples) == 0 or not res.samples[-1] == s:
                    res.samples.append(Sample(s.x,s.y))
        return res



