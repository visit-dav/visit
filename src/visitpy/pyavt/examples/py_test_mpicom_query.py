# VisIt Python Query 
#
# Tests VisIt's mpicom python MPI wrapper.
#
#
# Usage:
#  Load in the Query window or run with provided driver:
#   visit -o path/to/rect2d.silo -nowin -cli -s visit_mpicom_test.py
#
class Dog:
    """
    A simple class used to test mpi ops on a python object.
    """
    def __init__(self,name,age):
        self.name = name
        self.age  = age
    def age_in_dog_years(self):
        return self.age * 7
    def __str__(self):
        return "My name is %s and I am %d years old." % (self.name,self.age)
    def __cmp__(self,other):
        try:
            if other.name == self.name and other.age == self.age:
                return 0
            else:
                return 1
        except:
                return 1

class MPIComTest(SimplePythonQuery):
    """
    Python query that tests mpicom functionality.
    """
    def __init__(self):
        SimplePythonQuery.__init__(self)
        self.name = "MPIComTest"
        self.description = "Testing VisIt's embedded mpicom python module."
    def test(self,name,curr,expected):
        """
        A basic assert equal like helper.
        """
        rank = mpicom.rank()
        res = "passed"
        if curr != expected:
            res = "failed"
            self.failures += 1
        rtxt = "%d:%s=%s:%s\n" % (rank,name,repr(str(curr)),res)
        self.results += rtxt
    def exe_tests(self):
        """
        Executes main mpi tests.
        """
        self.rank = mpicom.rank()
        self.size = mpicom.size()
        if mpicom.parallel():
            self.test("libname",os.path.split(mpicom.__file__)[1],"mpicom.so")
        else:
            self.test("libname",os.path.split(mpicom.__file__)[1],"mpistub.pyc")
        self.test_broadcast()
        self.test_reduce()
        self.test_p2p()
        self.test_gather()
        self.test_scatter()
        #self.test_alltoall()
    def test_reduce(self):
        print self.rank,":test_reduce"
        # test min reduce
        self.test("min",mpicom.min(self.rank),0)
        # test max reduce
        self.test("max",mpicom.max(self.rank),self.size-1)
        # test sum reduce
        self.test("sum",mpicom.sum(self.rank),(self.size)*(self.size-1)/2)
        # test mult reduce
        self.test("mult",mpicom.mult(self.rank),0)
        # test broadcast of integer
    def test_broadcast(self):
        print self.rank,":test_broadcast"
        if self.rank == 0:
            self.test("broadcast",mpicom.broadcast(1),None)
        else:
            self.test("broadcast",mpicom.broadcast(),1)
        # test broadcast of double
        if self.rank == 0:
            self.test("broadcast",mpicom.broadcast(3.1415),None)
        else:
            self.test("broadcast",mpicom.broadcast(),3.1415)
        # test broadcast of python string
        if self.rank == 0:
            self.test("broadcast",mpicom.broadcast("Test"),None)
        else:
            self.test("broadcast",mpicom.broadcast(),"Test")
        # create instance of Dog class to send
        fluffy = Dog("fluffy",3)
        # test broadcast of python object
        if self.rank == 0:
            self.test("broadcast",mpicom.broadcast(fluffy),None)
        else:
            self.test("broadcast",mpicom.broadcast(),fluffy)
    def test_p2p(self):
        print self.rank,":test_p2p"
        # test send/recv with python object
        fluffy = Dog("fluffy",3)
        if self.size > 1:
            fluffy.age = 5
            if self.rank == 0:
                self.test("send",mpicom.send(fluffy,1),None)
            elif self.rank == 1:
                self.test("recv",mpicom.recv(0),fluffy)
            if self.rank == 0:
                self.test("sendrecv",mpicom.sendrecv(self.rank,1,1),1)
            elif self.rank == 1:
                self.test("sendrecv",mpicom.sendrecv(self.rank,0,0),0)
    def test_gather(self):
        print self.rank,":test_gather"
        res = [ i for i in range(self.size)]
        self.test("gather",mpicom.gather(self.rank),res)
    def test_scatter(self):
        print self.rank,":test_scatter"
        if self.rank == 0:
            msg = [ i for i in range(self.size)]
            self.test("scatter",mpicom.scatter(msg),self.rank)
        else:
            self.test("scatter",mpicom.scatter(),self.rank)
    def test_alltoall(self):
        print self.rank,"alltoall"
        msg = [ self.rank] * self.size
        res = [ i for i in range(self.size)]
        self.test("alltoall",mpicom.alltoall(msg),res)
    def post_execute(self):
        self.failures = 0
        self.results  = ""
        self.exe_tests()
        # collect all of the results from other procs
        res = mpicom.gather(self.results)
        rtxt = ""
        for v in res:
            rtxt += v
        self.set_result_text(rtxt)
        self.set_result_value(mpicom.sum(self.failures))

py_filter = MPIComTest

