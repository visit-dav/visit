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
#  Modifications:
#    Kathleen Biagas, Tue July 27, 2021
#    Assigning Max32BitInt+1 to int on Windows causes TypeError, not
#    ValueError, so change expected results in those cases.
#
#    Kathleen Biagas, Friday Feb 9, 2024
#    Manually skipping some dir() and help() tests that fail with
#    Python 3.9.18 until #19264 is addressed.
#
# ----------------------------------------------------------------------------
import copy, io, numpy, sys

# Some useful global variables
X = [2,4,6]
NumPy2Int = numpy.array([1,2])
NumPy2Flt = numpy.array([1.1,2.2])
NumPy3Int = numpy.array([1,2,3])
NumPy3Flt = numpy.array([1.1,2.2,3.3])
NumPy4Int = numpy.array([1,2,3,4])
NumPy4Flt = numpy.array([1.1,2.2,3.3,4.4])
Max32BitInt = 2147483647
Max32BitInt1 = Max32BitInt+1
MaxIntAs32BitFloat = 16777216
MaxIntAs32BitFloat1 = MaxIntAs32BitFloat+1
MaxIntAs64BitFloat = 9007199254740992
MaxIntAs64BitFloat1 = MaxIntAs64BitFloat+1
Max32BitFloat  = 3.402823E+38
Max32BitFloatA = 3.402820E+37 # One order mag down from Max
Max32BitFloatB = 3.402823E+39 # One order mag up from Max
Min32BitFloat = 1.175494E-38

# version of repr that strips parens at end
def repr2(s):
    return repr(s).lstrip('(').rstrip(')')

def TestAssignmentToTuple():
    TestSection('Assignment to tuple, "point1", member (of CylinderAttributes())')

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
    fails = [(1,2), (1,2,3,4), '123', (1,1+2j,3), (1,X,3), (1,'b',3), (1,None,3), NumPy2Flt, NumPy4Flt]
    for i in range(len(fails)):
        try:
            ca.point1 = fails[i]
            TestFOA('ca.point1=%s'%repr2(fails[i]), LINE())
        except TypeError:
            TestPOA('ca.point1=%s'%repr2(fails[i]))
            pass
        except:
            TestFOA('ca.point1=%s'%repr2(fails[i]), LINE())
            pass

    for i in range(len(fails)):
        try:
            ca.SetPoint1(fails[i])
            TestFOA('ca.SetPoint1(%s)'%repr2(fails[i]), LINE())
        except TypeError:
            TestPOA('ca.SetPoint1(%s)'%repr2(fails[i]))
            pass
        except:
            TestFOA('ca.SetPoint1(%s)'%repr2(fails[i]), LINE())
            pass

    try:
        ca.point1 = 1,2,3
        TestPOA('ca.point1=1,2,3')
    except:
        TestFOA('ca.point1=1,2,3', LINE())
        pass

    works = [(1,2,3), (1.1,2.2,3.3), tuple(X), NumPy3Int, NumPy3Flt]
    for i in range(len(works)):
        try:
            ca.point1 = works[i]
            TestPOA('ca.point1=%s'%repr2(works[i]))
        except:
            TestFOA('ca.point1=%s'%repr2(works[i]), LINE())
            pass

    for i in range(len(works)):
        try:
            ca.SetPoint1(*works[i])
            TestPOA('ca.SetPoint1(%s)'%repr2(works[i]))
        except:
            TestFOA('ca.SetPoint1(%s)'%repr2(works[i]), LINE())
            pass

def TestAssignmentToBool():
    TestSection('Assignment to bool member, "inverse", (of CylinderAttributes())')

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

    fails =  [    '123',      1+2j,         X,      None,          5]
    excpts = [TypeError, TypeError, TypeError, TypeError, ValueError]
    for i in range(len(fails)):
        try:
            ca.inverse = fails[i]
            TestFOA('ca.inverse=%s'%repr(fails[i]), LINE())
        except excpts[i]:
            TestPOA('ca.inverse=%s'%repr(fails[i]))
            pass
        except:
            TestFOA('ca.inverse=%s'%repr(fails[i]), LINE())
            pass

    for i in range(len(fails)):
        try:
            ca.SetInverse(fails[i])
            TestFOA('ca.SetInverse(%s)'%repr(fails[i]), LINE())
        except excpts[i]:
            TestPOA('ca.SetInverse(%s)'%repr(fails[i]))
            pass
        except:
            TestFOA('ca.SetInverse(%s)'%repr(fails[i]), LINE())
            pass

    works = [0, 1, True, False]
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
    TestSection('Assignment to int member, "samplesPerRay", (of VolumeAttributes())')

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

    fails =  [    '123',      1+2j,      None,         X, Max32BitInt1]
    if sys.platform.startswith("win"):
        excpts = [TypeError, TypeError, TypeError, TypeError, TypeError]
    else:
        excpts = [TypeError, TypeError, TypeError, TypeError, ValueError]
    for i in range(len(fails)):
        try:
            va.samplesPerRay = fails[i]
            TestFOA('va.samplesPerRay=%s'%repr(fails[i]), LINE())
        except excpts[i]:
            TestPOA('va.samplesPerRay=%s'%repr(fails[i]))
            pass
        except:
            TestFOA('va.samplesPerRay=%s'%repr(fails[i]), LINE())
            pass

    for i in range(len(fails)):
        try:
            va.SetSamplesPerRay(fails[i])
            TestFOA('va.SetSamplesPerRay(%s)'%repr(fails[i]), LINE())
        except excpts[i]:
            TestPOA('va.SetSamplesPerRay(%s)'%repr(fails[i]))
            pass
        except:
            TestFOA('va.SetSamplesPerRay(%s)'%repr(fails[i]), LINE())
            pass

    works = [0, 1, -1, 5, True, False, Max32BitInt]
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
    TestSection('Assignment to float member, "opacityAttenuation", (of VolumeAttributes())')

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

    fails =  [    '123',      1+2j,      None,         X, Max32BitFloatB]
    excpts = [TypeError, TypeError, TypeError, TypeError,       ValueError]
    for i in range(len(fails)):
        try:
            va.opacityAttenuation = fails[i]
            TestFOA('va.opacityAttenuation=%s'%repr(fails[i]), LINE())
        except excpts[i]:
            TestPOA('va.opacityAttenuation=%s'%repr(fails[i]))
            pass
        except:
            TestFOA('va.opacityAttenuation=%s'%repr(fails[i]), LINE())
            pass

    for i in range(len(fails)):
        try:
            va.SetOpacityAttenuation(fails[i])
            TestFOA('va.SetOpacityAttenuation(%s)'%repr(fails[i]), LINE())
        except excpts[i]:
            TestPOA('va.SetOpacityAttenuation(%s)'%repr(fails[i]))
            pass
        except:
            TestFOA('va.SetOpacityAttenuation(%s)'%repr(fails[i]), LINE())
            pass

    works = [0, 1, -1, 0.3, Max32BitFloatA, True, False]
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
    TestSection('Assignment to double member, "radius", (of CylinderAttributes())')

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
    TestSection('Assignment to string member, "designator", (of CurveAttributes())')

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
    TestSection('Assignment to GlyphType member, "pointType", (of MeshAttributes())')

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
    TestSection('Assignment to Enum member, "smoothingLevel", (of MeshAttributes())')

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
    TestSection('Assignment to ucharVector member, "changedColors",  (of MultiCurveAttributes())')

    mca = MultiCurveAttributes()

    # Test direct assignment with = operator
    try:
        mca.changedColors = 1,2,3
        TestPOA('mca.changedColors=1,2,3')
    except:
        TestFOA('mca.changedColors=1,2,3', LINE())
        pass

    fails = [(1,123123123123123123123123123123,3), (1,1+2j,3), (1,X,3), (1,'b',3), (1,None,3), ('123',)]
    for i in range(len(fails)):
        try:
            mca.changedColors = fails[i]
            TestFOA('mca.changedColors=%s'%repr2(fails[i]), LINE())
        except TypeError:
            TestPOA('mca.changedColors=%s'%repr2(fails[i]))
            pass
        except:
            TestFOA('mca.changedColors=%s'%repr2(fails[i]), LINE())
            pass

    for i in range(len(fails)):
        try:
            mca.SetChangedColors(*fails[i])
            TestFOA('mca.SetChangedColors(%s)'%repr2(fails[i]), LINE())
        except TypeError:
            TestPOA('mca.SetChangedColors(%s)'%repr2(fails[i]))
            pass
        except:
            TestFOA('mca.SetChangedColors(%s)'%repr2(fails[i]), LINE())
            pass

    works = [(1,2,3), tuple(X), (1,True,3), (1,False,3)]
    for i in range(len(works)):
        try:
            mca.changedColors = works[i]
            TestPOA('mca.changedColors=%s'%repr2(works[i]))
        except:
            TestFOA('mca.changedColors=%s'%repr2(works[i]), LINE()) 

    works += [NumPy3Int] # NP arrays only work via deref operator
    for i in range(len(works)):
        try:
            mca.SetChangedColors(*works[i])
            TestPOA('mca.SetChangedColors(%s)'%repr2(works[i]))
        except:
            TestFOA('mca.SetChangedColors(%s)'%repr2(works[i]), LINE()) 

def TestAssignmentToIntVector():
    TestSection('Assignment to intVector member, "index", (of OnionPeelAttributes())')

    opa = OnionPeelAttributes()

    # Test direct assignment with = operator
    try:
        opa.index = 1,2,3
        TestPOA('opa.index=1,2,3')
    except:
        TestFOA('opa.index=1,2,3', LINE())
        pass

    fails =  [(Max32BitInt1,), (1+2j,), ('b',), (None,), (1,Max32BitInt1,3),
              (1,1+2j,3), (1,X,3), (1,'b',3), (1,None,3)]
    if sys.platform.startswith("win"):
        excpts = [TypeError, TypeError, TypeError, TypeError, TypeError,
                  TypeError, TypeError, TypeError, TypeError]
    else:
        excpts = [ValueError, TypeError, TypeError, TypeError, ValueError,
                  TypeError, TypeError, TypeError, TypeError]
    for i in range(len(fails)):
        try:
            opa.index = fails[i]
            TestFOA('opa.index=%s'%repr2(fails[i]), LINE())
        except excpts[i]:
            TestPOA('opa.index=%s'%repr2(fails[i]))
            pass
        except:
            TestFOA('opa.index=%s'%repr2(fails[i]), LINE())
            pass

    for i in range(len(fails)):
        try:
            opa.SetIndex(*fails[i])
            TestFOA('opa.SetIndex(%s)'%repr2(fails[i]), LINE())
        except excpts[i]:
            TestPOA('opa.SetIndex(%s)'%repr2(fails[i]))
            pass
        except:
            TestFOA('opa.SetIndex(%s)'%repr2(fails[i]), LINE())
            pass

    works = [(1,2,3), X, tuple(X), (1,True,3), (1,False,3), (1,Max32BitInt,3)]
    for i in range(len(works)):
        try:
            opa.index = works[i]
            TestPOA('opa.index=%s'%repr2(works[i]))
        except:
            TestFOA('opa.index=%s'%repr2(works[i]), LINE()) 

    works += [NumPy3Int] # NP Arrays work only via deref operator
    for i in range(len(works)):
        try:
            opa.SetIndex(*works[i])
            TestPOA('opa.SetIndex(%s)'%repr2(works[i]))
        except:
            TestFOA('opa.SetIndex(%s)'%repr2(works[i]), LINE()) 
    
def TestAssignmentToDoubleVector():
    TestSection('Assignment to doubleVector member, "values", (of ContourAttributes())')

    ca = ContourAttributes()

    # Test direct assignment with = operator
    try:
        ca.contourValue = 1,2,3
        TestPOA('ca.contourValue=1,2,3')
    except:
        TestFOA('ca.contourValue=1,2,3', LINE())
        pass

    fails = [(1+2j,), ('b',), (None,), (1,1+2j,3), (1,X,3), (1,'b',3), (1,None,3)]
    for i in range(len(fails)):
        try:
            ca.contourValue = fails[i]
            TestFOA('ca.contourValue=%s'%repr2(fails[i]), LINE())
        except TypeError:
            TestPOA('ca.contourValue=%s'%repr2(fails[i]))
            pass
        except:
            TestFOA('ca.contourValue=%s'%repr2(fails[i]), LINE())
            pass

    for i in range(len(fails)):
        try:
            ca.SetContourValue(*fails[i])
            TestFOA('ca.SetContourValue(%s)'%repr2(fails[i]), LINE())
        except TypeError:
            TestPOA('ca.SetContourValue(%s)'%repr2(fails[i]))
            pass
        except:
            TestFOA('ca.SetContourValue(%s)'%repr2(fails[i]), LINE())
            pass

    works = [(1,2,3), X, tuple(X), (1,True,3), (1,False,3)]
    for i in range(len(works)):
        try:
            ca.contourValue = works[i]
            TestPOA('ca.contourValue=%s'%repr2(works[i]))
        except:
            TestFOA('ca.contourValue=%s'%repr2(works[i]), LINE()) 

    works += [NumPy3Flt] # NP Arrays work only via deref operator
    for i in range(len(works)):
        try:
            ca.SetContourValue(*works[i])
            TestPOA('ca.SetContourValue(%s)'%repr2(works[i]))
        except:
            TestFOA('ca.SetContourValue(%s)'%repr2(works[i]), LINE()) 
    
def TestAssignmentToUCharArray():
    TestSection('Assignment to ucharArray member, "freeformOpacity", (of VolumeAttributes())')
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

    # Test star-deref of tuple
    for i in range(len(fails)):
        try:
            va.SetFreeformOpacity(*fails[i])
            TestFOA('va.SetFreeformOpacity%s'%repr(fails[i][:7]).replace(')',', ...)'), LINE())
        except TypeError:
            TestPOA('va.SetFreeformOpacity%s'%repr(fails[i][:7]).replace(')',', ...)'))
            pass
        except:
            TestFOA('va.SetFreeformOpacity%s'%repr(fails[i][:7]).replace(')',', ...)'), LINE())
            pass

    # Test just passing the tuple
    for i in range(len(fails)):
        try:
            va.SetFreeformOpacity(fails[i])
            TestFOA('va.SetFreeformOpacity(fails[%d])'%i, LINE())
        except TypeError:
            TestPOA('va.SetFreeformOpacity(fails[%d])'%i)
            pass
        except:
            TestFOA('va.SetFreeformOpacity(fails[%d])'%i, LINE())
            pass

def TestAssignmentToIntArray():
    TestSection('Assignment to intArray member, "reflections", (of ReflectAttributes())')

    ra = ReflectAttributes()

    # Test assigning via (operator =) assignment
    try:
        ra.reflections = 0,1,0,1,0,1,0,1
        TestPOA('ra.reflections=0,1,0,1,0,1,0,1')
    except:
        TestFOA('ra.reflections=0,1,0,1,0,1,0,1', LINE())
        pass

    fails =  [(0,1,None,1,0,1,0,1), (0,1,1+2j,1,0,1,0,1), (0,1,X,1,0,1,0,1),
              (0,1,Max32BitInt1,1,0,1,0,1), (0,1,'123',1,0,1,0,1),
              (0,1,0,1,0,1,0,1,1), (0,1,0,1,0,1,0)]
    if sys.platform.startswith("win"):
        excpts = [TypeError, TypeError, TypeError, TypeError, TypeError, TypeError, TypeError]
    else: 
        excpts = [TypeError, TypeError, TypeError, ValueError, TypeError, TypeError, TypeError]
    for i in range(len(fails)):
        try:
            ra.reflections = fails[i]
            TestFOA('ra.reflections=%s'%repr2(fails[i]), LINE())
        except excpts[i]:
            TestPOA('ra.reflections=%s'%repr2(fails[i]))
            pass
        except:
            TestFOA('ra.reflections=%s'%repr2(fails[i]), LINE())
            pass

    for i in range(len(fails)):
        try:
            ra.SetReflections(*fails[i])
            TestFOA('ra.SetReflections(%s)'%repr2(fails[i]), LINE())
        except excpts[i]:
            TestPOA('ra.SetReflections(%s)'%repr2(fails[i]))
            pass
        except:
            TestFOA('ra.SetReflections(%s)'%repr2(fails[i]), LINE())
            pass

    NumPyArray = numpy.array([0,1,0,1,0,1,0,1])
    works = [(0,1,0,1,0,1,0,1), (-1,100,-1,100,-1,100,-1,100), (0,True,False,1,0,1,0,1), (0,1,Max32BitInt,1,0,1,0,1), NumPyArray]
    for i in range(len(works)):
        try:
            ra.reflections = works[i]
            TestPOA('ra.reflections=%s'%repr2(works[i]))
        except:
            TestFOA('ra.reflections=%s'%repr2(works[i]), LINE())

    for i in range(len(works)):
        try:
            ra.SetReflections(*works[i])
            TestPOA('ra.SetReflections(%s)'%repr2(works[i]))
        except:
            TestFOA('ra.SetReflections(%s)'%repr2(works[i]), LINE()) 

def TestAssignmentToFloatArray():
    TestSection('Assignment to floatArray member, "center", (of RadialResampleAttributes())')

    rra = RadialResampleAttributes()

    # Test assigning via (operator =) assignment
    try:
        rra.center = 0,1,2
        TestPOA('rra.center=0,1,2')
    except:
        TestFOA('rra.center=0,1,2', LINE())
        pass
    try:
        rra.center = 0,1
        TestFOA('rra.center=0,1', LINE())
    except:
        TestPOA('rra.center=0,1')
        pass
    try:
        rra.center = 0,1,2,3
        TestFOA('rra.center=0,1,2,3', LINE())
    except:
        TestPOA('rra.center=0,1,2,3')
        pass

    fails =  [(0,1), (0,1,2,3), (0,None,2), (0,1+2j,2), (0,X,2), (0,'123',2), (0, Max32BitFloatB,2)]
    excpts = [TypeError, TypeError, TypeError, TypeError, TypeError, TypeError, ValueError]
    for i in range(len(fails)):
        try:
            rra.center = fails[i]
            TestFOA('rra.center=%s'%repr2(fails[i]), LINE())
        except excpts[i]:
            TestPOA('rra.center=%s'%repr2(fails[i]))
            pass
        except:
            TestFOA('rra.center=%s'%repr2(fails[i]), LINE())
            pass

    for i in range(len(fails)):
        try:
            rra.SetCenter(*fails[i])
            TestFOA('rra.SetCenter(%s)'%repr2(fails[i]), LINE())
        except excpts[i]:
            TestPOA('rra.SetCenter(%s)'%repr2(fails[i]))
            pass
        except:
            TestFOA('rra.SetCenter(%s)'%repr2(fails[i]), LINE())
            pass

    works = [(1,2,3), (1.1,2.2,3.3), tuple(X), (1,True,3), (1,False,3), (1,Max32BitFloatA,3), NumPy3Flt]
    for i in range(len(works)):
        try:
            rra.center = works[i]
            TestPOA('rra.center=%s'%repr2(works[i]))
        except:
            TestFOA('rra.center=%s'%repr2(works[i]), LINE())

    for i in range(len(works)):
        try:
            rra.SetCenter(*works[i])
            TestPOA('rra.SetCenter(%s)'%repr2(works[i]))
        except:
            TestFOA('rra.SetCenter(%s)'%repr2(works[i]), LINE()) 

def TestAssignmentToDoubleArray():
    TestSection('Assignment to doubleArray member, "materialProperties", (of VolumeAttributes())')

    va = VolumeAttributes()

    # Test assigning via (operator =) assignment
    try:
        va.materialProperties = 0,1,2,3
        TestPOA('va.materialProperties=0,1,2,3')
    except:
        TestFOA('va.materialProperites=0,1,2,3', LINE())
        pass
    try:
        va.materialProperties = 0,1,2
        TestFOA('va.materialProperties=0,1,2', LINE())
    except:
        TestPOA('va.materialProperties=0,1,2')
        pass
    try:
        va.materialProperties = 0,1,2,3,4
        TestFOA('va.materialProperties=0,1,2,3,4', LINE())
    except:
        TestPOA('va.materialProperties=0,1,2,3,4')
        pass

    fails = [(0,1), (0,1,2,3,4), (0,None,2,3), (0,1+2j,2,3), (0,X,2,3), (0,'123',2,3)]
    for i in range(len(fails)):
        try:
            va.materialProperties = fails[i]
            TestFOA('va.materialProperties=%s'%repr2(fails[i]), LINE())
        except TypeError:
            TestPOA('va.materialProperties=%s'%repr2(fails[i]))
            pass
        except:
            TestFOA('va.materialProperties=%s'%repr2(fails[i]), LINE())
            pass

    for i in range(len(fails)):
        try:
            va.SetMaterialProperties(*fails[i])
            TestFOA('va.SetMaterialProperties(%s)'%repr2(fails[i]), LINE())
        except TypeError:
            TestPOA('va.SetMaterialProperties(%s)'%repr2(fails[i]))
            pass
        except:
            TestFOA('va.SetMaterialProperties(%s)'%repr2(fails[i]), LINE())
            pass

    works = [(1,2,3,4), (1.1,2.2,3.3,4.4), (1,True,3,4), (1,False,3,4)]
    for i in range(len(works)):
        try:
            va.materialProperties = works[i]
            TestPOA('va.materialProperties=%s'%repr2(works[i]))
        except:
            TestFOA('va.materialProperties=%s'%repr2(works[i]), LINE())

    NumPyArray = numpy.array([1.1,2.2,3.3,4.4])
    works += [NumPyArray]
    for i in range(len(works)):
        try:
            va.SetMaterialProperties(*works[i])
            TestPOA('va.SetMaterialProperties(%s)'%repr2(works[i]))
        except:
            TestFOA('va.SetMaterialProperties(%s)'%repr2(works[i]), LINE()) 

def TestColorAttributeStuff():
    TestSection('ColorAttribute stuff')

    cla = ColorAttributeList()
    ca = ColorAttribute()

    fails = [(0,1,2), (0,1,2,3,4), (0,None,2,3), (0,1+2j,2,3), (0,X,2,3),
             (0,'123',2,3), (0,-1,2,3), (0,256,2,3)]
    excpts = [TypeError, TypeError, TypeError, TypeError, TypeError, TypeError, ValueError, ValueError]
    for i in range(len(fails)):
        try:
            ca.color = fails[i]
            TestFOA('ca.color=%s'%repr2(fails[i]), LINE())
        except excpts[i]:
            TestPOA('ca.color=%s'%repr2(fails[i]))
            pass
        except:
            TestFOA('ca.color=%s'%repr2(fails[i]), LINE())
            pass

    for i in range(len(fails)):
        try:
            ca.SetColor(*fails[i])
            TestFOA('ca.SetColor(%s)'%repr2(fails[i]), LINE())
        except excpts[i]:
            TestPOA('ca.SetColor(%s)'%repr2(fails[i]))
            pass
        except:
            TestFOA('ca.SetColor(%s)'%repr2(fails[i]), LINE())
            pass

    try:
        ca.color = (5,5,5,5)
        cla.AddColors(ca)
        ca.color = (255,0,0,255)
        cla.AddColors(ca)
        TestPOA('cla.AddColors')
    except:
        TestFOA('cla.AddColors', LINE())
        pass

    try:
        cla.colors
        TestFOA('cla.colors', LINE())
    except NameError:
        TestPOA('cla.colors')
    except:
        TestFOA('cla.colors', LINE())
        pass

    try:
        if cla.GetColors(0).color != (5,5,5,5) or cla.GetColors(1).color != (255,0,0,255):
           raise ValueError
        TestPOA('cla.GetColors(0)')
    except:
        TestFOA('cla.Getcolors(0)', LINE())
        pass

    try:
        cla.GetColors(2)
        TestFOA('cla.Getcolors(2)', LINE())
    except ValueError:
        TestPOA('cla.GetColors(2)')
    except:
        TestFOA('cla.Getcolors(2)', LINE())
        pass
        
def TestDirOutput(obj, names):
    try:
        x = [f for f in dir(obj) if not (f.startswith('__') and f.endswith('__'))]
        y = [n for n in names if n in x]
        if len(y) != len(names):
            raise Exception
        x = repr(obj)
        if x.startswith('<built-in'):
            x = x.strip('<>').split()[2]
        else:
            x = x.strip('<>').split()[0]
        TestPOA('dir(%s)'%x)
    except:
        TestFOA('dir(%s)'%x, LINE()) 

#
# Test that dir(x) appears to work
#
def TestDir(global_dir_result):
    TestSection('behavior of dir()')

    #
    # Testing of global dir() behavior can't be done from within a func
    #
    global_dir_checks = ['AddPlot', 'DeleteAllPlots', 'Launch', 'GetMetaData']
    y = [n for n in global_dir_checks if n in global_dir_result]
    if len(y) != len(global_dir_checks):
        TestFOA('dir()', LINE()) 
    else:
        TestPOA('dir()')

    #
    # Test a random handful of object level dirs
    #

    """ 
    # These fail with Python 3.9.18, see bug ticket #19264.
    # Impossible to add them to skip list due to unhandled exceptions
    # So they are being skipped here with a note added to the ticket.
    TestDirOutput(SILRestriction(), ['NumSets', 'TurnOnAll', 'Wholes'])
    TestDirOutput(PseudocolorAttributes(), ['GetCentering', 'GetColorTableName',
        'GetLightingFlag', 'GetLimitsMode', 'GetMax', 'SetCentering',
        'SetColorTableName', 'SetLegendFlag', 'SetLimitsMode'])
    TestDirOutput(ColorAttributeList(), ['AddColors', 'ClearColors', 'GetColors'])
    """


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
def TestHelpOutput(thing, words = None):
    try:
        with my_redirect_stdout() as output:
            help(thing)
        x  = [w for w in words if w in str(output)]
        if len(x) != len(words):
            raise Exception
        x = repr(thing)
        if x.startswith('<built-in'):
            x = x.strip('<>').split()[2]
        else:
            x = x.strip('<>').split()[0]
        TestPOA('help(%s)'%x)
    except:
        TestFOA('help(%s)'%x, LINE()) 

def TestHelp():    
    TestSection('behavior of help()')

    TestHelpOutput(AddPlot, ['plotType', 'variableName', 'inheritSIL'])
    TestHelpOutput(CreateDatabaseCorrelation,
        ['IndexForIndexCorrelation', 'CycleCorrelation', 'StretchedIndexCorrelation'])
    TestHelpOutput(SILRestriction(),[]) # should not except

    """
    # These tests fail with Python 3.9.18 see bug ticket #19264.
    # due to nature of thee way these tests are named
    # (different name for failure than success),
    # they need to be skipped manually until #19264 is addressed
    TestHelpOutput(SILRestriction, ['GlobalAttributes', 'SetPlotSILRestriction',
        'TurnDomainsOff', 'TurnDomainsOn', 'TurnMaterialsOff', 'TurnMaterialsOn'])
    TestHelpOutput('wholes', ['SILRestriction'])
    TestHelpOutput('tensor', ['DefineArrayExpression', 'DefineTensorExpression',
        'LCSAttributes', 'SPHResampleAttributes', 'TensorAttributes'])
    """

#
# Scalar assignments
#
# TestAssignmentToUChar() No instances in any .xml files
TestAssignmentToBool()
TestAssignmentToInt()
TestAssignmentToFloat()
TestAssignmentToDouble()
TestAssignmentToString()
TestAssignmentToGlyphType()
TestAssignmentToEnum()
TestAssignmentToTuple()

#
# Vector assignments
#
TestAssignmentToUCharVector()
#TestAssignmentToBoolVector() No instances in any .xml files
TestAssignmentToIntVector()
#TestAssignmentToFloatVector() No instances in any .xml files
TestAssignmentToDoubleVector()

#
# Array assignments
#
TestAssignmentToUCharArray()
#TestAssignmentToBoolArray() No instances in any .xml files
TestAssignmentToIntArray()
TestAssignmentToFloatArray()
TestAssignmentToDoubleArray()

#
# Attribute Assignments
#
TestColorAttributeStuff()

#
# Dir behavior
#
TestDir(dir())

#
# Help() behavior
#
TestHelp()
    
Exit()
