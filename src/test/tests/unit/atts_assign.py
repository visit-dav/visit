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

# Scalar assignments
TestAssignmentToBool()
TestAssignmentToInt()
TestAssignmentToFloat()
TestAssignmentToDouble()
TestAssignmentToString()
TestAssignmentToTuple()

# Array assignments

# Vector assignments

    
Exit()
