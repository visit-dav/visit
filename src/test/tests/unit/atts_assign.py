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
ca = CylinderAttributes()
X = [1,2,3]

try:
    ca.point = 1,2,3
    TestFOA('NameError', LINE())
except NameError:
    TestPOA('NameError')
    pass
except:
    TestFOA('NameError', LINE())
    pass

try:
    ca.point2 = 1,2
    TestFOA('Unadorned ValueError', LINE())
except ValueError:
    TestPOA('Unadorned ValueError')
    pass
except:
    TestFOA('Unadorned ValueError', LINE())
    pass

try:
    ca.point2 = (1,2)
    TestFOA('Too small ValueError', LINE())
except ValueError:
    TestPOA('Too small ValueError')
    pass
except:
    TestFOA('Too small ValueError', LINE())
    pass

try:
    ca.point2 = (1,2,3,4)
    TestFOA('Too large ValueError', LINE())
except ValueError:
    TestPOA('Too large ValueError')
    pass
except:
    TestFOA('Too large ValueError', LINE())
    pass

try:
    ca.point2 = 'mark'
    TestFOA('Unadorned string TypeError', LINE())
except TypeError:
    TestPOA('Unadorned string TypeError')
    pass
except:
    TestFOA('Unadorned string TypeError', LINE())
    pass

try:
    ca.point2 = (X,X,X)
    TestFOA('Array value TypeError', LINE())
except TypeError:
    TestPOA('Array value TypeError')
    pass
except:
    TestFOA('Array value TypeError', LINE())
    pass

try:
    ca.point2 = ('a','b','c')
    TestFOA('String value TypeError', LINE())
except TypeError:
    TestPOA('String value TypeError')
    pass
except:
    TestFOA('String value TypeError', LINE())
    pass

try:
    ca.point2 = (1,2,3)
    ca.point2 = (1.1,2.2,3.3)
    ca.point2 = 1,2,3
    ca.point2 = tuple(X)
    TestPOA('Valid assignment')
except:
    TestFOA('Valid assignment', LINE())
    pass

Exit()
