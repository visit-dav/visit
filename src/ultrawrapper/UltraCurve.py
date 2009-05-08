import os
from pyparsing import alphas
# ----------------------------------------------------------------------------
# Class: UltraCurveItem
# ----------------------------------------------------------------------------
class UltraCurveItem(object):
    """ An ultra curve info object.

        Consists of Var, extents and filename"
    """

    def __init__(self, v, e = (0,0,0,0), f = None):
        self.var = v
        self.extents = e
        self.fileName = f


    def __str__(self):
        if self.fileName is not None:
            return self.var.ljust(25) + "  % .2e  % .2e  % .2e  % .2e  " % self.extents + self.fileName
        else: 
            return self.var.ljust(25) + "  % .2e  % .2e  % .2e  % .2e" % self.extents

    def __repr__(self):
        s = "var = \"%s\"" % self.var
        s += "\nextents = (%.2e, %.2e, %.2e, %.2e)" % (self.extents[0], \
             self.extents[1], self.extents[2], self.extents[3])
        s += "\nfileName = \"%s\"" % self.fileName
        return s

    def remove(self):
        pass

    def getPath(self):
        if self.fileName is not None:
            return os.path.split(self.fileName)[0]
        else:
            return None

    def getDB(self):
        if self.fileName is not None:
            return os.path.split(self.fileName)[1]
        else:
            return None
            
# ----------------------------------------------------------------------------
# Class: MenuCurveItem
# ----------------------------------------------------------------------------
class MenuCurveItem(UltraCurveItem):
    """ Subclass of UltraCurveItem, representing a curve to select from a menu

        Adds an integer Identifier.
    """
    def __init__(self, v, e = (0,0,0,0), f = None):
        UltraCurveItem.__init__(self, v,e,f)
        self.id = 0 

    def __str__(self):
        return  repr(self.id).rjust(4) + " " + UltraCurveItem.__str__(self)

    def __repr__(self):
        s = "id = %d" % self.id
        s += "\n%s" % UltraCurveItem.__repr__(self)
        return s

    def __cmp__(self, other):
        if self.id < other.id:
            return -1
        if self.id > other.id:
            return 1
        return 0 

# ----------------------------------------------------------------------------
# Class: PlotCurveItem
# ----------------------------------------------------------------------------
class PlotCurveItem(UltraCurveItem):
    """ Subclass of UltraCurveItem, representing a plotted curve.

        Adds a character Identifier, a plotId, a modifiedFlag, and a list of
        expressions defined on the curve
    """
    def __init__(self, v, e = (0,0,0,0), f = None):
        UltraCurveItem.__init__(self, v,e,f)
        self.id = 'z'
        self.plotId = -1
        self.modified = False
        self.expressions = []

        
    def __str__(self):
        mod = ""
        if self.modified:
           mod = "*"
        return mod.rjust(2) + " " + self.id + " " + UltraCurveItem.__str__(self)
        
    def __repr__(self):
        s = "id = %c" % self.id
        s += "\n%s" % UltraCurveItem.__repr__(self)
        s += "\nmodified = %s" % self.modified
        s += "\nplotId = %s" % self.plotId
        s += "\nexpressions = %s" % self.expressions
        return s

    def __cmp__(self, other):
        if self.id < other.id:
            return -1
        if self.id > other.id:
            return 1
        return 0 

    def copy(self):
        p = PlotCurveItem(self.var, self.extents, self.fileName)
       
        if self.plotId is not None:
            p.plotId = self.plotId
        if self.modified is not None:
            p.modified = self.modified
        if self.expressions is not None:
            p.expressions = self.expressions[:]
        return p

# ----------------------------------------------------------------------------
# Class: CurveList
#
#   Modifications:
#
#    Kathleen Bonnell, Fri May  8 15:13:18 PDT 2009
#    Raise an exception when lookup-fails for get_item.  Add try-except blocks
#    around use of get_item in slice.
#
# ----------------------------------------------------------------------------
class CurveList(object):
    """ A list containing UltraCurveItem's
    """
    labels = list(alphas[26:])
    labels += list(alphas[0:26])
    labels.reverse()

    count = 1 
    data_id = 1

    def __init__(self):
        self.curves = []
        self.__class__.count = 1
        self.__class__.data_id = 1

    def add(self, c):
        if type(c) is MenuCurveItem:
            c.id = self.__class__.count
            self.__class__.count += 1
        elif type(c) is PlotCurveItem:
            if len(self.__class__.labels) == 0:
                # need to raise an exception here!
                print "Ran out of curve labels!"
            else:
                c.id = self.__class__.labels.pop()
        self.curves.append(c)

    def getCurve(self, i):
        if i in range(len(self.curves)):
            return self.curves[i]

    def getCurveWithId(self, _id):
        if (self.curves != []):
            for curve in self.curves:
                if curve.id == _id:
                    return curve 

    def remove(self, c):
        if type(c) is PlotCurveItem:
            self.__class__.labels.append(c.id)
            self.__class__.labels.sort(reverse=True)
            self.curves.remove(c)
        elif type(c) is MenuCurveItem:
            self.curves.remove(c)
        elif type(c) == int:
            self.curves.remove(self.curves[c])

    def removeAll(self):
        for i in range(len(self.curves)-1, -1, -1):
            c = self.curves[i]
            if type(c) is PlotCurveItem:
                self.__class__.labels.append(c.id)
                self.__class__.labels.sort(reverse=True)
                self.curves.remove(c)
            elif type(c) is MenuCurveItem:
                self.curves.remove(c)
            elif type(c) == int:
                self.curves.remove(self.curves[c])
            
    def __getitem__(self, index):
        if (self.curves != []):
            for curve in self.curves:
                if curve.id == index:
                    return curve 
        # Lookup failed, Raise an exception
        raise LookupError

    def __str__(self):
        if (self.curves) != []:
            sc = sorted(self.curves)
            s = ""
            for i in sc:
                s +=  "%s\n" %(i.__str__())
            return s
        else:
            return "[]"

    def __repr__(self):
        if (self.curves) != []:
            sc = sorted(self.curves)
            s = "[\n"
            for i in sc:
                s +=  "%s,\n" %(i.__str__())
            s += "]"
            return s
        else:
            return "[]"


    def slice(self, t):
        rv = []
        if len(self.curves) > 0:
            for i in range(len(t)):
                s = t[i].partition(":")
                if type(self.curves[0]) is MenuCurveItem:
                    if s[1] == '':
                        try:
                            rv.append(self.__getitem__(int(s[0])))
                        except LookupError:
                            pass
                    else:
                        for j in range(int(s[0]), int(s[2])+1):
                            try:
                                rv.append(self.__getitem__(j))
                            except LookupError:
                                pass
                else:
                    if s[1] == '':
                        try:
                            rv.append(self.__getitem__(s[0]))
                        except LookupError:
                            pass
                    else:
                        for j in range(ord(s[0]), ord(s[2])+1):
                            try:
                                rv.append(self.__getitem__(chr(j)))
                            except LookupError:
                                pass 
        return rv
           
 
