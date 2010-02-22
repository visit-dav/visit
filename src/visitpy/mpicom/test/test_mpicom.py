#!/usr/bin/env python
###############################################################################
# Program: test_mpicom.py
#
# Purpose: Tests for the pyavt.mpicom python module. 
#
# Programmer: Cyrus Harrison
# Creation: Wed Jan  7 14:00:41 PST 2009
#
# Notes: 
#   Build and install the pyavt module into your python install.
#   Make sure you have the proper ld_lib path set so mpi can be loaded.
#   Test with 'mpirun -np 4 python test_mpicom.py'
#
# Modifications:
#
###############################################################################

import sys
sys.path.append("/home/harrison37/hdrive/dev/branches/1002_python_filters/src/lib/")
import mpicom

failures = 0

def test(name,curr,expected):
    """
    A basic assert equal helper.
    """
    rank = mpicom.rank()
    res = "passed"
    if curr != expected:
        res = "failed"
        failures += 1
    print "%d:%s=%s:%s" % (rank,name,repr(str(curr)),res)

# fire up mpi
mpicom.init(sys.argv)
# get the current process id and total # of processors
rank = mpicom.rank()
sz   = mpicom.size()

# test min reduce
test("min",mpicom.min(rank),0)
# test max reduce
test("max",mpicom.max(rank),sz-1)
# test sum reduce
test("sum",mpicom.sum(rank),(sz)*(sz-1)/2)
# test mult reduce
test("mult",mpicom.mult(rank),0)

# test broadcast of integer
if rank == 0:
    test("broadcast",mpicom.broadcast(1),None)
else:
    test("broadcast",mpicom.broadcast(),1)

# test broadcast of double
if rank == 0:
    test("broadcast",mpicom.broadcast(3.1415),None)
else:
    test("broadcast",mpicom.broadcast(),3.1415)

# test broadcast of python string
if rank == 0:
    test("broadcast",mpicom.broadcast("Test"),None)
else:
    test("broadcast",mpicom.broadcast(),"Test")

# create a simple class to test mpi comm of a python object.
class Person:
    def __init__(self,name,age):
        self.name = name
        self.age  = age
    def __str__(self):
        return "My name is %s and I am %d years old." % (self.name,self.age)
    def __cmp__(self,other):
        if other.name == self.name and other.age == self.age:
            return 0
        else:
            return 1

# create instance of Person class to send
cyrus  = Person("Cyrus",29)

# test broadcast of python object
if rank == 0:
    test("broadcast",mpicom.broadcast(cyrus),None)
else:
    test("broadcast",mpicom.broadcast(),cyrus)

# test send/recv with python object
if sz > 1:
    cyrus.age = 30
    if rank == 0:
        test("send",mpicom.send(cyrus,1),None)
    elif rank == 1:
        test("recv",mpicom.recv(0),cyrus)

# shutdown mpi
mpicom.finalize()

# report any failures
if failures == 0:
    print "%d:All Tests Passed" % rank
else:
    print "%d:# of Failures = %d" % (rank,failures)
