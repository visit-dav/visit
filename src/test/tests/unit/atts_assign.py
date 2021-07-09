# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  atts_assign.py
#
#  Tests: Behavior of assignment for attribute objects. Ensures good cases
#  succeed and bad cases fail with specific python exceptions. Tests variety
#  of types present in members of VisIt attribute objects. Tests both
#  assignment usage (e.g. atts.memberName=...) and setter function usage
#  (e.g. atts.SetMemberName(...))
#
#  Mark C. Miller, Tue Jun  8 15:51:59 PDT 2021
#
# ----------------------------------------------------------------------------
import copy, io, sys

# Some useful global variables
X = [2,4,6]

def TestAssignmentToTuple():
    TestSection('Assignment to tuple member (of CylinderAttributes())')

    ca = CylinderAttributes()
    
    # Non-existent member name 'point'
    try:
        ca.point = 1,2,3
        TestFOA('ca.point=1,2,3', LINE())
    except NameError:
        TestPOA('ca.point=1,2,3')
        pass
    except:
        TestFOA('ca.point=1,2,3', LINE())
        pass

    # Non-existent member name 'point'
    try:
        ca.SetPoint(1,2,3)
        TestFOA('ca.SetPoint(1,2,3)', LINE())
    except ValueError:
        TestPOA('ca.SetPoint(1,2,3)')
        pass
    except:
        TestFOA('ca.SetPoint(1,2,3)', LINE())
        pass
    
    # CSV too short
    try:
        ca.point1 = 1,2
        TestFOA('ca.point1=1,2', LINE())
    except TypeError:
        TestPOA('ca.point1=1,2')
        pass
    except:
        TestFOA('ca.point1=1,2', LINE())
        pass
    
    # CSV too long
    try:
        ca.point1 = 1,2,3,4
        TestFOA('ca.point1=1,2,3,4', LINE())
    except TypeError:
        TestPOA('ca.point1=1,2,3,4')
        pass
    except:
        TestFOA('ca.point1=1,2,3,4', LINE())
        pass

    # The above cases can't be put in a loop. Put remaining cases in a loop
    fails = [(1,2), (1,2,3,4), '123', (1,1+2j,3), (1,X,3), (1,'b',3), (1,None,3)]
    for i in range(len(fails)):
        try:
            ca.point1 = fails[i]
            TestFOA('ca.point1=%s'%repr(fails[i]), '%s:%s'%(LINE(),i))
        except TypeError:
            TestPOA('ca.point1=%s'%repr(fails[i]))
            pass
        except:
            TestFOA('ca.point1=%s'%repr(fails[i]), '%s:%s'%(LINE(),i))
            pass

    for i in range(len(fails)):
        try:
            ca.SetPoint1(fails[i])
            TestFOA('ca.SetPoint1(%s)'%repr(fails[i]), '%s:%s'%(LINE(),i))
        except TypeError:
            TestPOA('ca.SetPoint1(%s)'%repr(fails[i]))
            pass
        except:
            TestFOA('ca.SetPoint1(%s)'%repr(fails[i]), '%s:%s'%(LINE(),i))
            pass

    try:
        ca.point1 = 1,2,3
        TestPOA('ca.point1=1,2,3')
    except:
        TestFOA('ca.point1=1,2,3', LINE())
        pass

    works = [(1,2,3), (1.1,2.2,3.3), tuple(X)]
    for i in range(len(works)):
        try:
            ca.point1 = works[i]
            TestPOA('ca.point1=%s'%repr(works[i]))
        except:
            TestFOA('ca.point1=%s'%repr(works[i]), '%s:%s'%(LINE(),i))
            pass

    works = [(1,2,3), (1.1,2.2,3.3), tuple(X)]
    for i in range(len(works)):
        try:
            ca.SetPoint1(works[i])
            TestPOA('ca.SetPoint1(%s)'%repr(works[i]))
        except:
            TestFOA('ca.SetPoint1(%s)'%repr(works[i]), '%s:%s'%(LINE(),i))
            pass

def TestAssignmentToBool():
    TestSection('Assignment to bool member (of CylinderAttributes())')

    ca = CylinderAttributes()

    try:
        ca.inverse = 1,2
        TestFOA('ca.inverse=1,2', LINE())
    except TypeError:
        TestPOA('ca.inverse=1,2')
        pass
    except:
        TestFOA('ca.inverse=1,2', LINE())
        pass

    fails = ['123', 1+2j, X, None]
    for i in range(len(fails)):
        try:
            ca.inverse = fails[i]
            TestFOA('ca.inverse=%s'%repr(fails[i]), '%s:%s'%(LINE(),i))
        except TypeError:
            TestPOA('ca.inverse=%s'%repr(fails[i]))
            pass
        except:
            TestFOA('ca.inverse=%s'%repr(fails[i]), '%s:%s'%(LINE(),i))
            pass

    for i in range(len(fails)):
        try:
            ca.SetInverse(fails[i])
            TestFOA('ca.SetInverse(%s)'%repr(fails[i]), '%s:%s'%(LINE(),i))
        except TypeError:
            TestPOA('ca.SetInverse(%s)'%repr(fails[i]))
            pass
        except:
            TestFOA('ca.SetInverse(%s)'%repr(fails[i]), '%s:%s'%(LINE(),i))
            pass

    works = [0, 1, 5, True, False]
    for i in range(len(works)):
        try:
            ca.inverse = works[i]
            TestPOA('ca.inverse=%s'%repr(works[i]))
        except:
            TestFOA('ca.inverse=%s'%repr(works[i]), LINE()) 

    for i in range(len(works)):
        try:
            ca.SetInverse(works[i])
            TestPOA('ca.SetInverse(%s)'%repr(works[i]))
        except:
            TestFOA('ca.SetInverse(%s)'%repr(works[i]), LINE()) 

def TestAssignmentToInt():
    TestSection('Assignment to int member (of VolumeAttributes())')

    va = VolumeAttributes()

    try:
        va.samplesPerRay = 1,2
        TestFOA('va.samplesPerRay=1,2', LINE())
    except TypeError:
        TestPOA('va.samplesPerRay=1,2')
        pass
    except:
        TestFOA('va.samplesPerRay=1,2', LINE())
        pass

    fails = ['123', 1+2j, None, X, 123123123123123123123123123123]
    for i in range(len(fails)):
        try:
            va.samplesPerRay = fails[i]
            TestFOA('va.samplesPerRay=%s'%repr(fails[i]), LINE())
        except TypeError:
            TestPOA('va.samplesPerRay=%s'%repr(fails[i]))
            pass
        except:
            TestFOA('va.samplesPerRay=%s'%repr(fails[i]), LINE())
            pass

    for i in range(len(fails)):
        try:
            va.SetSamplesPerRay(fails[i])
            TestFOA('va.SetSamplesPerRay(%s)'%repr(fails[i]), LINE())
        except TypeError:
            TestPOA('va.SetSamplesPerRay(%s)'%repr(fails[i]))
            pass
        except:
            TestFOA('va.SetSamplesPerRay(%s)'%repr(fails[i]), LINE())
            pass

    works = [0, 1, -1, 5, True, False]
    for i in range(len(works)):
        try:
            va.samplesPerRay = works[i]
            TestPOA('va.samplesPerRay=%s'%repr(works[i]))
        except:
            TestFOA('va.samplesPerRay=%s'%repr(works[i]), LINE()) 

    for i in range(len(works)):
        try:
            va.SetSamplesPerRay(works[i])
            TestPOA('va.SetSamplesPerRay(%s)'%repr(works[i]))
        except:
            TestFOA('va.SetSamplesPerRay(%s)'%repr(works[i]), LINE()) 

def TestAssignmentToFloat():
    TestSection('Assignment to float member (of VolumeAttributes())')

    va = VolumeAttributes()

    try:
        va.opacityAttenuation = 1,2
        TestFOA('va.opacityAttenuation=1,2', LINE())
    except TypeError:
        TestPOA('va.opacityAttenuation=1,2')
        pass
    except:
        TestFOA('va.opacityAttenuation=1,2', LINE())
        pass

    fails = ['123', 1+2j, None, X]
    for i in range(len(fails)):
        try:
            va.opacityAttenuation = fails[i]
            TestFOA('va.opacityAttenuation=%s'%repr(fails[i]), LINE())
        except TypeError:
            TestPOA('va.opacityAttenuation=%s'%repr(fails[i]))
            pass
        except:
            TestFOA('va.opacityAttenuation=%s'%repr(fails[i]), LINE())
            pass

    for i in range(len(fails)):
        try:
            va.SetOpacityAttenuation(fails[i])
            TestFOA('va.SetOpacityAttenuation(%s)'%repr(fails[i]), LINE())
        except TypeError:
            TestPOA('va.SetOpacityAttenuation(%s)'%repr(fails[i]))
            pass
        except:
            TestFOA('va.SetOpacityAttenuation(%s)'%repr(fails[i]), LINE())
            pass

    works = [0, 1, -1, 5.5, 1.1E-479, 1.1E+479, True, False]
    for i in range(len(works)):
        try:
            va.opacityAttenuation = works[i]
            TestPOA('va.opacityAttenuation=%s'%repr(works[i]))
        except:
            TestFOA('va.opacityAttenuation=%s'%repr(works[i]), LINE()) 

    for i in range(len(works)):
        try:
            va.SetOpacityAttenuation(works[i])
            TestPOA('va.SetOpacityAttenuation(%s)'%repr(works[i]))
        except:
            TestFOA('va.SetOpacityAttenuation(%s)'%repr(works[i]), LINE()) 

def TestAssignmentToDouble():
    TestSection('Assignment to double member (of CylinderAttributes())')

    ca = CylinderAttributes()

    try:
        ca.radius = 1,2
        TestFOA('ca.radius=1,2', LINE())
    except TypeError:
        TestPOA('ca.radius=1,2')
        pass
    except:
        TestFOA('ca.radius=1,2', LINE())
        pass

    fails = ['123', 1+2j, None, X]
    for i in range(len(fails)):
        try:
            ca.radius = fails[i]
            TestFOA('ca.radius=%s'%repr(fails[i]), LINE())
        except TypeError:
            TestPOA('ca.radius=%s'%repr(fails[i]))
            pass
        except:
            TestFOA('ca.radius=%s'%repr(fails[i]), LINE())
            pass

    for i in range(len(fails)):
        try:
            ca.SetRadius(fails[i])
            TestFOA('ca.SetRadius(%s)'%repr(fails[i]), LINE())
        except TypeError:
            TestPOA('ca.SetRadius(%s)'%repr(fails[i]))
            pass
        except:
            TestFOA('ca.SetRadius(%s)'%repr(fails[i]), LINE())
            pass

    works = [0, 1, -1, 5.5, 1.1E-479, 1.1E+479, True, False]
    for i in range(len(works)):
        try:
            ca.radius = works[i]
            TestPOA('ca.radius=%s'%repr(works[i]))
        except:
            TestFOA('ca.radius=%s'%repr(works[i]), LINE()) 

    for i in range(len(works)):
        try:
            ca.SetRadius(works[i])
            TestPOA('ca.SetRadius(%s)'%repr(works[i]))
        except:
            TestFOA('ca.SetRadius(%s)'%repr(works[i]), LINE()) 

def TestAssignmentToString():
    TestSection('Assignment to string member (of CurveAttributes())')

    ca = CurveAttributes()

    try:
        ca.designator = "123","abc"
        TestFOA('ca.designator="123","abc"', LINE())
    except TypeError:
        TestPOA('ca.designator="123","abc"')
        pass
    except:
        TestFOA('ca.designator="123","abc"', LINE())
        pass

    fails = [0, 1, 1.1, 1+2j, None, X]
    for i in range(len(fails)):
        try:
            ca.designator = fails[i]
            TestFOA('ca.designator=%s'%repr(fails[i]), LINE())
        except TypeError:
            TestPOA('ca.designator=%s'%repr(fails[i]))
            pass
        except:
            TestFOA('ca.designator=%s'%repr(fails[i]), LINE())
            pass

    for i in range(len(fails)):
        try:
            ca.SetDesignator(fails[i])
            TestFOA('ca.SetDesignator(%s)'%repr(fails[i]), LINE())
        except TypeError:
            TestPOA('ca.SetDesignator(%s)'%repr(fails[i]))
            pass
        except:
            TestFOA('ca.SetDesignator(%s)'%repr(fails[i]), LINE())
            pass

    works = ['123', 'abc', '']
    for i in range(len(works)):
        try:
            ca.designator = works[i]
            TestPOA('ca.designator=%s'%repr(works[i]))
        except:
            TestFOA('ca.designator=%s'%repr(works[i]), LINE()) 

    for i in range(len(works)):
        try:
            ca.SetDesignator(works[i])
            TestPOA('ca.SetDesignator(%s)'%repr(works[i]))
        except:
            TestFOA('ca.SetDesignator(%s)'%repr(works[i]), LINE()) 

def TestAssignmentToGlyphType():
    TestSection('Assignment to GlyphType member (of MeshAttributes())')

    ma = MeshAttributes()

    # Test direct assignment with = operator
    try:
        ma.pointType = 1
        TestPOA('ma.pointType=1')
    except:
        TestFOA('ma.pointType=1', LINE())
        pass

    fails =  [    '123',      1+2j,      None,         X,         -1, 123123123123123123123123123123]
    excpts = [TypeError, TypeError, TypeError, TypeError, ValueError,                      TypeError]
    for i in range(len(fails)):
        try:
            ma.pointType = fails[i]
            TestFOA('ma.pointType=%s'%repr(fails[i]), LINE())
        except excpts[i]:
            TestPOA('ma.pointType=%s'%repr(fails[i]))
            pass
        except:
            TestFOA('ma.pointType=%s'%repr(fails[i]), LINE())
            pass

    for i in range(len(fails)):
        try:
            ma.SetPointType(fails[i])
            TestFOA('ma.SetPointType(%s)'%repr(fails[i]), LINE())
        except excpts[i]:
            TestPOA('ma.SetPointType(%s)'%repr(fails[i]))
            pass
        except:
            TestFOA('ma.SetPointType(%s)'%repr(fails[i]), LINE())
            pass

    works = [0, 1, 5, True, False, ma.Point]
    for i in range(len(works)):
        try:
            ma.pointType = works[i]
            TestPOA('ma.pointType=%s'%repr(works[i]))
        except:
            TestFOA('ma.pointType=%s'%repr(works[i]), LINE()) 

    for i in range(len(works)):
        try:
            ma.SetPointType(works[i])
            TestPOA('ma.SetPointType(%s)'%repr(works[i]))
        except:
            TestFOA('ma.SetPointType(%s)'%repr(works[i]), LINE()) 

def TestAssignmentToEnum():
    TestSection('Assignment to Enum member (of MeshAttributes())')

    ma = MeshAttributes()

    # Test direct assignment with = operator
    try:
        ma.smoothingLevel = 1
        TestPOA('ma.smoothingLevel=1')
    except:
        TestFOA('ma.smoothingLevel=1', LINE())
        pass

    fails  = [    '123',      1+2j,      None,         X,         -1,  123123123, 123123123123123123123123123123]
    excpts = [TypeError, TypeError, TypeError, TypeError, ValueError, ValueError,                      TypeError]
    for i in range(len(fails)):
        try:
            ma.smoothingLevel = fails[i]
            TestFOA('ma.smoothingLevel=%s'%repr(fails[i]), LINE())
        except excpts[i]:
            TestPOA('ma.smoothingLevel=%s'%repr(fails[i]))
            pass
        except:
            TestFOA('ma.smoothingLevel=%s'%repr(fails[i]), LINE())
            pass

    for i in range(len(fails)):
        try:
            ma.SetSmoothingLevel(fails[i])
            TestFOA('ma.SetSmoothingLevel(%s)'%repr(fails[i]), LINE())
        except excpts[i]:
            TestPOA('ma.SetSmoothingLevel(%s)'%repr(fails[i]))
            pass
        except:
            TestFOA('ma.SetSmoothingLevel(%s)'%repr(fails[i]), LINE())
            pass

    works = [0, 1, 2, True, False, ma.Fast]
    for i in range(len(works)):
        try:
            ma.smoothingLevel = works[i]
            TestPOA('ma.smoothingLevel=%s'%repr(works[i]))
        except:
            TestFOA('ma.smoothingLevel=%s'%repr(works[i]), LINE()) 

    for i in range(len(works)):
        try:
            ma.SetSmoothingLevel(works[i])
            TestPOA('ma.SmoothingLevel(%s)'%repr(works[i]))
        except:
            TestFOA('ma.SetSmoothingLevel(%s)'%repr(works[i]), LINE()) 

def TestAssignmentToUCharVector():
    TestSection('Assignment to ucharVector member (of MultiCurveAttributes())')

    mca = MultiCurveAttributes()

    # Test direct assignment with = operator
    try:
        mca.changedColors = 1,2,3
        TestPOA('mca.changedColors=1,2,3')
    except:
        TestFOA('mca.changedColors=1,2,3', LINE())
        pass

    fails =  [(1,123123123123123123123123123123,3),   (1,1+2j,3),   (1,X,3), (1,'b',3), (1,None,3),     '123']
    for i in range(len(fails)):
        try:
            mca.changedColors = fails[i]
            TestFOA('mca.changedColors=%s'%repr(fails[i]), LINE())
        except TypeError:
            TestPOA('mca.changedColors=%s'%repr(fails[i]))
            pass
        except:
            TestFOA('mca.changedColors=%s'%repr(fails[i]), LINE())
            pass

    for i in range(len(fails)):
        try:
            mca.SetChangedColors(fails[i])
            TestFOA('mca.SetChangedColors(%s)'%repr(fails[i]), LINE())
        except TypeError:
            TestPOA('mca.SetChangedColors(%s)'%repr(fails[i]))
            pass
        except:
            TestFOA('mca.SetChangedColors(%s)'%repr(fails[i]), LINE())
            pass

    works = [(1,2,3), tuple(X), (1,True,3), (1,False,3)]
    for i in range(len(works)):
        try:
            mca.changedColors = works[i]
            TestPOA('mca.changedColors=%s'%repr(works[i]))
        except:
            TestFOA('mca.changedColors=%s'%repr(works[i]), LINE()) 

    for i in range(len(works)):
        try:
            mca.SetChangedColors(*works[i])
            TestPOA('mca.SetChangedColors(%s)'%repr(works[i]))
        except:
            TestFOA('mca.SetChangedColors(%s)'%repr(works[i]), LINE()) 

def TestAssignmentToUCharArray():
    TestSection('Assignment to ucharArray member (of VolumeAttributes())')
    arr = [17,]*256

    va = VolumeAttributes()

    # Test assigning to individual entry via direct (operator =) assignment
    try:
        va.freeformOpacity = 3,17
        TestPOA('va.freeformOpacity=3,17')
    except:
        TestFOA('va.freeformOpacity=3,17', LINE())
        pass

    # Test assigning to individual entry via Set method 
    try:
        va.SetFreeformOpacity(3,17)
        TestPOA('va.SetFreeformOpacity(3,17)')
    except:
        TestFOA('va.SetFreeformOpacity(3,17)', LINE())
        pass

    # Test assigning to whole array via (operator =) assignment
    try:
        va.freeformOpacity = tuple(arr)
        TestPOA('va.freeformOpacity=tuple(arr)')
    except:
        TestFOA('va.freeformOpacity=tuple(arr)', LINE())
        pass

    # Test assigning to whole array via Set method 
    try:
        va.SetFreeformOpacity(*tuple(arr))
        TestPOA('va.SetFreeformOpacity(*tuple(arr))')
    except:
        TestFOA('va.SetFreeformOpacity(*tuple(arr))', LINE())
        pass

    # Test assigning to individual entry via direct (operator =) assignment
    # failures for type of second argument (color value)
    fails =  [ (3,None),  (3,1+2j),     (3,X), (3,'123'), (None,17), (1+2j,17),    (X,17),('123',17),    (-3,17),   (3,1700)]
    excpts = [TypeError, TypeError, TypeError, TypeError, TypeError, TypeError, TypeError, TypeError, IndexError, ValueError]
    for i in range(len(fails)):
        try:
            va.freeformOpacity = fails[i][0],fails[i][1]
            TestFOA('va.freeformOpacity=%s,%s'%(repr(fails[i][0]),repr(fails[i][1])), LINE())
        except excpts[i]:
            TestPOA('va.freeformOpacity=%s,%s'%(repr(fails[i][0]),repr(fails[i][1])))
            pass
        except:
            TestFOA('va.freeformOpacity=%s,%s'%(repr(fails[i][0]),repr(fails[i][1])), LINE())
            pass

    for i in range(len(fails)):
        try:
            va.SetFreeformOpacity(fails[i][0],fails[i][1])
            TestFOA('va.SetFreeformOpacity(%s,%s)'%(repr(fails[i][0]),repr(fails[i][1])), LINE())
        except excpts[i]:
            TestPOA('va.SetFreeformOpacity(%s,%s)'%(repr(fails[i][0]),repr(fails[i][1])))
            pass
        except:
            TestFOA('va.SetFreeformOpacity(%s,%s)'%(repr(fails[i][0]),repr(fails[i][1])), LINE())
            pass

    # Test assigning to whole member via direct (operator =) assignment
    try:
        va.freeformOpacity = (17,)*256
        TestPOA('va.freeformOpacity=(17,)*256')
    except:
        TestFOA('va.freeformOpacity=(17,)*256', LINE())
        pass

    # Test assigning to whole member via Set method 
    try:
        va.SetFreeformOpacity(*(17,)*256)
        TestPOA('va.SetFreeformOpacity((17,)*256)')
    except:
        TestFOA('va.SetFreeformOpacity((17,)*256)', LINE())
        pass

    # Test assigning to whole member via direct (operator =) assignment
    # failures for type of first argument (index)
    arr1 = copy.deepcopy(arr)
    arr2 = copy.deepcopy(arr)
    arr3 = copy.deepcopy(arr)
    arr4 = copy.deepcopy(arr)
    arr5 = copy.deepcopy(arr)
    arr1[3] = None
    arr2[3] = 1+2j
    arr3[3] = X
    arr4[3] = (1,2,3)
    arr5[3] = '123'
    fails = [tuple(arr1), tuple(arr2), tuple(arr3), tuple(arr4), tuple(arr5)]
    for i in range(len(fails)):
        try:
            va.freeformOpacity = fails[i]
            TestFOA('va.freeformOpacity=%s'%repr(fails[i][:7]).replace(')',', ...'), LINE())
        except TypeError:
            TestPOA('va.freeformOpacity=%s'%repr(fails[i][:7]).replace(')',', ...'))
            pass
        except:
            TestFOA('va.freeformOpacity=%s'%repr(fails[i][:7]).replace(')',', ...'), LINE())
            pass

    for i in range(len(fails)):
        try:
            va.SetFreeformOpacity(*fails[i])
            TestFOA('va.SetFreeformOpacity(%s)'%repr(fails[i][:7]).replace(')',', ...'), LINE())
        except TypeError:
            TestPOA('va.SetFreeformOpacity(%s)'%repr(fails[i][:7]).replace(')',', ...'))
            pass
        except:
            TestFOA('va.SetFreeformOpacity(%s)'%repr(fails[i][:7]).replace(')',', ...'), LINE())
            pass

def TestDirOutput(obj, minlen = 5, names = None):
    TestSection('behavior of dir()')
    try:
        x = [f for f in dir(obj) if not (f.startswith('__') and f.endswith('__'))]
        if minlen and len(x) < minlen:
            TestFOA('dir(%s): minlen: %d < %d'%(repr(obj),len(x),minlen), LINE()) 
        x  = [n for n in names if n in x]
        if len(x) != len(names):
            TestFOA('dir(%s): names: %s'%(repr(obj), names), LINE()) 
        TestPOA('dir(%s)'%repr())
    except:
        TestFOA('dir(%s)'%repr(obj), LINE()) 

# Class to facilitate stdout redirect for testing `help()`
class my_redirect_stdout(list):
    def __enter__(self):
        self._stdout = sys.stdout
        sys.stdout = self._stringio = io.StringIO()
        return self
    def __exit__(self, *args):
        self.extend(self._stringio.getvalue().splitlines())
        del self._stringio    # free up some memory
        sys.stdout = self._stdout

# Below import works only for Python > 3.4
# So, we use the class def above
# from contextlib import redirect_stdout
def TestHelpOutput(thing, minlen = 200, words = None):
    TestSection('behavior of help()')
    try:
        with my_redirect_stdout() as output:
            help(thing)
        if minlen and len(str(output)) < minlen:
            TestFOA('dir(%s): minlen: %d < %d'%(repr(thing),len(output),minlen), LINE()) 
        x  = [w for w in words if w in str(output)]
        if len(x) != len(words):
            TestFOA('dir(%s): words: %s'%(repr(thing), words), LINE()) 
    except:
        TestFOA('help(%s)'%repr(thing), LINE()) 
    
# Scalar assignments
# TestAssignmentToUChar() No instances in any .xml files
TestAssignmentToBool()
TestAssignmentToInt()
TestAssignmentToFloat()
TestAssignmentToDouble()
TestAssignmentToString()

TestAssignmentToGlyphType()
TestAssignmentToEnum()

TestAssignmentToTuple()

# Vector assignments
TestAssignmentToUCharVector()
#TestAssignmentToBoolVector() No instances in any .xml files
#TestAssignmentToIntVector()
#TestAssignmentToFloatVector()
#TestAssignmentToDoubleVector()

# Array assignments
TestAssignmentToUCharArray()
#TestAssignmentToBoolArray() No instances in any .xml files
#TestAssignmentToIntArray()
#TestAssignmentToFloatArray()
#TestAssignmentToDoubleArray()


# Test that dir(x) appears to work
#TestDirOutput(SILRestriction(), None, ['NumSets', 'TurnOnAll', 'Wholes', 'TopSets'])
#TestDirOutput(PseudocolorAttributes(), 50)
#TestDirOutput(ColorAttributeList(), None, ['AddColors', 'ClearColors', 'GetColors'])

# Test Help
#TestHelpOutput(AddPlot, None, ['plotType', 'variableName', 'inheritSIL'])
#TestHelpOutput(CreateDatabaseCorrelation, None,
#    ['IndexForIndexCorrelation', 'CycleCorrelation', 'StretchedIndexCorrelation'])
    
Exit()
