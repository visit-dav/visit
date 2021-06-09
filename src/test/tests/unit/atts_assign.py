# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  atts_assign.py
#
#  Tests:      Behavior of assignment for attribute objects
#
#  Mark C. Miller, Tue Jun  8 15:51:59 PDT 2021
#
# ----------------------------------------------------------------------------
def TestTupleAssignment():

    TestSection("Tuple assignment")

    ca = CylinderAttributes()
    X = [1,2,3]
    
    # Non-existent member name 'point'
    try:
        ca.point = 1,2,3
        TestFOA('NameError', LINE())
    except NameError:
        TestPOA('NameError')
        pass
    except:
        TestFOA('NameError', LINE())
        pass
    
    # CSV too short
    try:
        ca.point2 = 1,2
        TestFOA('Short CSV ValueError', LINE())
    except ValueError:
        TestPOA('Short CSV ValueError')
        pass
    except:
        TestFOA('Short CSV ValueError', LINE())
        pass
    
    # CSV too long
    try:
        ca.point2 = 1,2,3,4
        TestFOA('Long CSV ValueError', LINE())
    except ValueError:
        TestPOA('Long CSV ValueError')
        pass
    except:
        TestFOA('Long CSV ValueError', LINE())
        pass
    
    # tuple too short
    try:
        ca.point2 = (1,2)
        TestFOA('Short tuple ValueError', LINE())
    except ValueError:
        TestPOA('Short tuple ValueError')
        pass
    except:
        TestFOA('Short tuple ValueError', LINE())
        pass
    
    # tuple too long
    try:
        ca.point2 = (1,2,3,4)
        TestFOA('Long tuple ValueError', LINE())
    except ValueError:
        TestPOA('Long tuple ValueError')
        pass
    except:
        TestFOA('Long tuple ValueError', LINE())
        pass
    
    # assign string to tuple
    try:
        ca.point2 = 'mark'
        TestFOA('tuple=string TypeError', LINE())
    except TypeError:
        TestPOA('tuple=string TypeError')
        pass
    except:
        TestFOA('tuple=string TypeError', LINE())
        pass
    
    # assign array to scalar members
    try:
        ca.point2 = (X,X,X)
        TestFOA('scalar-member=array TypeError', LINE())
    except TypeError:
        TestPOA('scalar-member=array TypeError')
        pass
    except:
        TestFOA('scalar-member=array TypeError', LINE())
        pass
    
    # assign string to scalar members
    try:
        ca.point2 = ('a','b','c')
        TestFOA('scalar-member=string TypeError', LINE())
    except TypeError:
        TestPOA('scalar-member=string TypeError')
        pass
    except:
        TestFOA('scalar-member=string TypeError', LINE())
        pass
    
    try:
        # correct CSV length and valid member types
        ca.point2 = 1,2,3
        # correct tuple length and valid member types
        ca.point2 = (1,2,3)
        # correct tuple length and valid member types
        ca.point2 = (1.1,2.2,3.3)
        # correct tuple length and valid member types
        ca.point2 = tuple(X)
        TestPOA('Valid tuple assignment')
    except:
        TestFOA('Valid tuple assignment', LINE())
        pass

def TestScalarAssignment():
    TestSection("Scalar assignment")

    ca = CylinderAttributes()

    # assign string to int
    try:
        ca.inverse = "mark"
        TestFOA('int=string TypeError', LINE())
    except TypeError:
        TestPOA('int=string TypeError')
        pass
    except:
        TestFOA('int=string TypeError', LINE())
        pass

    # assign double to int
    try:
        ca.inverse = 5.5
        TestFOA('int=double TypeError', LINE())
    except TypeError:
        TestPOA('int=double TypeError')
        pass
    except:
        TestFOA('int=double TypeError', LINE())
        pass

    # assign string to double
    try:
        ca.radius = "mark"
        TestFOA('double=string TypeError', LINE())
    except TypeError:
        TestPOA('double=string TypeError')
        pass
    except:
        TestFOA('double=string TypeError', LINE())
        pass

    # valid assignments
    try:
        ca.inverse = 1
        ca.inverse = 5
        ca.inverse = True
        ca.radius = -1
        ca.radius = 5.5
        ca.radius = True
        TestPOA('Valid scalar assignment')
    except:
        TestFOA('Valid scalar assignment', LINE()) 

TestTupleAssignment()
TestScalarAssignment()
    
Exit()
