import sys, math
import numpy as sp

# this code is largely inspired from Brad Whitlock's updateplots.py

# this code is similar in nature and algorithm to the F90 code contributed earlier
# see tools/DataManualExamples/Simulations/contrib
# this is a very first attempt to use some numpy arrays.
# A grid of size (M+2)x(M+2) is allocated.
# the working buffer is of size MxM
#

# no need for that if you set your PYTHONPATH to
# /path-to/current/linux-x86_64/lib from your install directory

#sys.path.append("/local/apps/VisIt/2.3.0/current/linux-x86_64/lib")    # for _simV2.so
#sys.path.append("/local/apps/VisItSrc/src/sim/V2/swig/python")         # for simV2.py

sys.path.append('/local/apps/VisIt/2.3.0/current/linux-x86_64/lib')
from simV2 import *
import mpicom  # only for parallel

class Simulation:
    def __init__(self):
        self.done = 0
        self.cycle = 0
        self.time = 0.
        self.runMode = VISIT_SIMMODE_STOPPED
        self.par_size = 1
        self.par_rank = 0
        self.savingFiles = 0
        self.saveCounter = 0
        self.M = 20
        self.MP = self.M # to be redefined if parallel
        self.rmesh_dims = [self.M+2, self.M+2, 1]
        self.dx = 1.0/(self.rmesh_dims[0]-1)
        self.rmesh_ndims = 2
        self.commands = ("halt", "step", "run", "addplot")
        self.v = sp.zeros([self.rmesh_dims[0],self.rmesh_dims[1]]) # includes 2 ghosts
        self.vnew = sp.zeros([self.M, self.M])

    def Initialize(self, simname, visitdir):
        VisItOpenTraceFile("trace.%d.txt" % self.par_rank)
        VisItSetDirectory(visitdir)
        VisItSetupEnvironment()
        if self.par_rank == 0:
            VisItInitializeSocketAndDumpSimFile(simname, 
                "Heat Equation solver in Python", "/path/to/where/visit/was/started",
                 None, None, None)
        self.rmesh_dims = [self.MP+2, self.M+2, 1]
        self.v = sp.zeros([self.rmesh_dims[0],self.rmesh_dims[1]]) # includes 2 ghosts
        self.vnew = sp.zeros([self.MP, self.M])
        self.set_initial_bc()

    def set_initial_bc(self):
        if self.par_size > 1:
          if self.par_rank == 0:
            for j in range(self.rmesh_dims[1]):
              self.v[0,j] = math.sin(math.pi*j*self.dx)
          if self.par_rank == (self.par_size-1):
            for j in range(self.rmesh_dims[1]):
              self.v[-1,j] = math.sin(math.pi*j*self.dx)* math.exp(-math.pi)
        else:
          #first (bottom) row
          for j in range(self.rmesh_dims[1]):
            self.v[0,j] = math.sin(math.pi*j*self.dx)
          #last (top) row
          self.v[-1,:] = self.v[0,:]* math.exp(-math.pi)

    def Finalize(self):
        VisItCloseTraceFile()

    def SimulateOneTimestep(self):
        self.cycle = self.cycle + 1
        self.time = self.time + 1.
        #if self.par_rank == 0:
            #print "Simulating time step: cycle=%d, time=%g" % (self.cycle,self.time)

        self.vnew = 0.25 * ( self.v[2:, 1:-1]  + # north neighbor
                             self.v[0:-2, 1:-1] + # south neighbor
                             self.v[1:-1, 2:] + # east neighbor
                             self.v[1:-1, :-2]) # west neighbor
        # copy now vnew to the interior region of v, leaving the boundary walls untouched.
        self.v[1:-1,1:-1] = self.vnew.copy()

        if self.par_size >1:
          # if in parallel, exchange ghost cells now
          # define who is my neighbor above and below
          below = self.par_rank-1
          above = self.par_rank+1
          if self.par_rank == 0:
            below = mpicom.MPI_PROC_NULL   # tells MPI not to perform send/recv
          if self.par_rank == (self.par_size-1):
            above = mpicom.MPI_PROC_NULL   # should only receive/send from/to below
          ret1 = mpicom.sendrecv(self.v[-2,], above, below)
          if ret1 != None:
            self.v[0,] = ret1
          ret1 = mpicom.sendrecv(self.v[1,], below, above)
          if ret1 != None:
            self.v[-1,] = ret1

        VisItTimeStepChanged()
        VisItUpdatePlots()
        if self.savingFiles:
            filename = "Heat%04d.jpg" % self.saveCounter
            if VisItSaveWindow(filename, 800, 800, VISIT_IMAGEFORMAT_JPEG) == VISIT_OKAY:
                self.saveCounter = self.saveCounter + 1
                if self.par_rank == 0:
                    print("Saved",filename)
            else:
                print("The image could not be saved to",filename)

    def ProcessVisItCommand(self):
        return (VisItProcessEngineCommand() == VISIT_OKAY)

    def DoPrompt(self):
        if self.par_rank == 0:
            sys.stdout.write("command>")
            sys.stdout.flush()

    def ProcessConsoleCommand(self):
        cmd = self.GetInputFromConsole()

        if cmd == "quit":
            self.done = 1
        elif cmd == "halt":
            self.runMode = VISIT_SIMMODE_STOPPED
        elif cmd == "step":
            self.SimulateOneTimestep()
        elif cmd == "run":
            self.runMode = VISIT_SIMMODE_RUNNING
        elif cmd == "saveon":
            self.savingFiles = 1
        elif cmd == "saveoff":
            self.savingFiles = 0
        elif cmd == "addplot":
            VisItExecuteCommand('AddPlot("Pseudocolor", "temperature")\n')
            VisItExecuteCommand('DrawPlots()\n')

    def GetInputFromConsole(self):
        return VisItReadConsole()
        
    def GetInputFromVisIt(self, blocking):
        console = sys.stdin.fileno()
        return VisItDetectInputWithTimeout(blocking, 100000, console)

    def MainLoop(self):
        self.DoPrompt()
        while self.done == 0:
            blocking = 1
            if self.runMode == VISIT_SIMMODE_RUNNING:
                blocking = 0

            visitstate = self.GetInputFromVisIt(blocking)

            if visitstate == 0:
                self.SimulateOneTimestep()
            elif visitstate == 1:
                if VisItAttemptToCompleteConnection() == VISIT_OKAY:
                    print("VisIt connected")
                    self.runMode = VISIT_SIMMODE_STOPPED

                    self.ConnectCallbacks()
                else:
                    print("VisIt did not connect")
            elif visitstate == 2:
                if not self.ProcessVisItCommand():
                    VisItDisconnect()
                    self.runMode = VISIT_SIMMODE_RUNNING
            elif visitstate == 3:
                self.ProcessConsoleCommand()
                self.DoPrompt()
            else:
                print("Error: ", visitstate)

    def ConnectCallbacks(self):
        VisItSetCommandCallback(self.ControlCommandCallback, 0)
        VisItSetGetMetaData(self.GetMetaData, None)
        VisItSetGetMesh(self.GetMesh, 0)
        VisItSetGetVariable(self.GetVariable, 0)

    #
    # Data access and control functions
    #

    def ControlCommandCallback(self, cmd, args, cbdata):
        if cmd == "halt":
            self.runMode = VISIT_SIMMODE_STOPPED
        elif cmd == "step":
            self.SimulateOneTimestep()
        elif cmd == "run":
            self.runMode = VISIT_SIMMODE_RUNNING
        elif cmd == "addplot":
            VisItExecuteCommand('AddPlot("Pseudocolor", "temperature")\n')
            VisItExecuteCommand('DrawPlots()\n')

    def GetMetaData(self, cbdata):
        md = VisIt_SimulationMetaData_alloc()
        if md != VISIT_INVALID_HANDLE:
            VisIt_SimulationMetaData_setCycleTime(md, self.cycle, self.time)
            # Add mesh metadata.
            mmd = VisIt_MeshMetaData_alloc()
            if mmd != VISIT_INVALID_HANDLE:
                # Set the mesh's properties.
                VisIt_MeshMetaData_setName(mmd, "mesh")
                VisIt_MeshMetaData_setMeshType(mmd, VISIT_MESHTYPE_RECTILINEAR)
                VisIt_MeshMetaData_setTopologicalDimension(mmd, 2)
                VisIt_MeshMetaData_setSpatialDimension(mmd, 2)
                VisIt_MeshMetaData_setNumDomains(mmd, self.par_size)
                VisIt_MeshMetaData_setDomainTitle(mmd, "Domains")
                VisIt_MeshMetaData_setDomainPieceName(mmd, "domain")
                VisIt_MeshMetaData_setNumGroups(mmd, 0)
                VisIt_MeshMetaData_setXUnits(mmd, "cm")
                VisIt_MeshMetaData_setYUnits(mmd, "cm")
                VisIt_MeshMetaData_setXLabel(mmd, "Width")
                VisIt_MeshMetaData_setYLabel(mmd, "Height")
                VisIt_SimulationMetaData_addMesh(md, mmd)

            # Add a variable.
            vmd = VisIt_VariableMetaData_alloc()
            if vmd != VISIT_INVALID_HANDLE:
                VisIt_VariableMetaData_setName(vmd, "temperature")
                VisIt_VariableMetaData_setMeshName(vmd, "mesh")
                VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_SCALAR)
                VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_NODE)
                VisIt_SimulationMetaData_addVariable(md, vmd)

            # Add some commands
            for c in self.commands:
                cmd = VisIt_CommandMetaData_alloc()
                if cmd != VISIT_INVALID_HANDLE:
                    VisIt_CommandMetaData_setName(cmd, c)
                    VisIt_SimulationMetaData_addGenericCommand(md, cmd)
        return md

    def GetMesh(self, domain, name, cbdata):
        h = VISIT_INVALID_HANDLE
        if name == "mesh":
            h = VisIt_RectilinearMesh_alloc()
            if h != VISIT_INVALID_HANDLE:
                minRealIndex = [0,0,0]
                maxRealIndex = [0,0,0]
                maxRealIndex[0] = self.rmesh_dims[1]-1
                if self.par_rank == (self.par_size-1):
                  maxRealIndex[1] = self.rmesh_dims[0]-1
                else:
                  maxRealIndex[1] = self.rmesh_dims[0]-2
                
                maxRealIndex[2] = self.rmesh_dims[2]-1

                rmesh_x = sp.linspace(0.0, 1.0, self.rmesh_dims[1])
                rmesh_y = sp.linspace(self.par_rank*self.MP*self.dx, ((self.par_rank+1)*self.MP+1)*self.dx, self.rmesh_dims[0])
                hx = VisIt_VariableData_alloc()
                hy = VisIt_VariableData_alloc()
                VisIt_VariableData_setDataD(hx, VISIT_OWNER_SIM, 1,
                                            rmesh_x.size, rmesh_x.tolist())
                VisIt_VariableData_setDataD(hy, VISIT_OWNER_SIM, 1,
                                            rmesh_y.size, rmesh_y.tolist())
                VisIt_RectilinearMesh_setCoordsXY(h, hx, hy)
                VisIt_RectilinearMesh_setRealIndices(h, minRealIndex, maxRealIndex)
        return h

    def GetVariable(self, domain, name, cbdata):
        h = VISIT_INVALID_HANDLE
        if name == "temperature":
            nTuples = self.v.size
            h = VisIt_VariableData_alloc()
            VisIt_VariableData_setDataD(h, VISIT_OWNER_SIM, 1,
                nTuples, self.v.flatten().tolist())
        return h


#*****************************************************************************
# Class: ParallelSimulation
#
# Purpose:
#   This subclass of Simulation overrides some input handling methods and 
#   provides a few extra callbacks to provide an MPI-parallel simulation.
#
# Programmer: Brad Whitlock
# Date:       Fri Mar 18 14:24:17 PDT 2011
#
# Modifications:
#
#*****************************************************************************

class ParallelSimulation(Simulation):
    def __init__(self):
        Simulation.__init__(self)
        self.VISIT_COMMAND_PROCESS = 0
        self.VISIT_COMMAND_SUCCESS = 1
        self.VISIT_COMMAND_FAILURE = 2

    # Override Initialize for parallel
    def Initialize(self, simname, visitdir):
        mpicom.init(sys.argv)
        VisItSetBroadcastIntFunction(self.broadcast_int)
        VisItSetBroadcastStringFunction(self.broadcast_string)
        self.par_size = mpicom.size()
        self.par_rank = mpicom.rank()
        VisItSetParallel(self.par_size  >1)
        VisItSetParallelRank(self.par_rank)
        self.par_size = mpicom.size()
        self.par_rank = mpicom.rank()
        self.MP = self.M / self.par_size
        Simulation.Initialize(self, simname, visitdir)
        print(self.par_rank, self.v)

    def GetInputFromConsole(self):
        cmd = ""
        if self.par_rank == 0:
            cmd = VisItReadConsole()
            mpicom.broadcast(cmd)
        else: 
            cmd = mpicom.broadcast()
        return cmd

    def GetInputFromVisIt(self, blocking):
        s = 0
        if self.par_rank == 0:
            console = sys.stdin.fileno()
            s = VisItDetectInputWithTimeout(blocking, 100000, console)
            mpicom.broadcast(s)
        else:
            s = mpicom.broadcast()
        return s

    def Finalize(self):
        Simulation.Finalize(self)
        mpicom.finalize()

    def ConnectCallbacks(self):
        Simulation.ConnectCallbacks(self)
        VisItSetSlaveProcessCallback(self.slave_process_callback)
        VisItSetGetDomainList(self.GetDomainList, 0)

    def GetDomainList(self, name, cbdata):
        h = VisIt_DomainList_alloc()
        if h != VISIT_INVALID_HANDLE:
            hdl = VisIt_VariableData_alloc()
            VisIt_VariableData_setDataI(hdl, VISIT_OWNER_VISIT, 1, 1, [self.par_rank])
            VisIt_DomainList_setDomains(h, self.par_size, hdl)
        return h

    def broadcast_int(self, ival, sender):
        if self.par_rank == 0:
            ret = mpicom.broadcast(ival)
        else:
            ret = mpicom.broadcast()
        return ret

    def broadcast_string(self, sval, slen, sender):
        if self.par_rank == 0:
            ret = mpicom.broadcast(sval)
        else:
            ret = mpicom.broadcast()
        return ret

    def slave_process_callback(self):
        s = self.VISIT_COMMAND_PROCESS
        if self.par_rank == 0:
            mpicom.broadcast(s)
        else:
            mpicom.broadcast()

    def ProcessVisItCommand(self):
        if self.par_rank == 0:
            success = VisItProcessEngineCommand()
            if success == VISIT_OKAY:
                mpicom.broadcast(self.VISIT_COMMAND_SUCCESS)
                return 1
            else:
                mpicom.broadcast(self.VISIT_COMMAND_FAILURE)
                return 0
        else:
            while 1:
                command = mpicom.broadcast()
                if command == self.VISIT_COMMAND_PROCESS:
                    VisItProcessEngineCommand()
                elif command == self.VISIT_COMMAND_SUCCESS:
                    return 1
                elif command == self.VISIT_COMMAND_FAILURE:
                    return 0

#
# Main program
#
def main():
    #sim = Simulation()
    #sim.Initialize("heat_equation", "../../..")
    sim = ParallelSimulation()
    sim.Initialize("heat_equation_par", "../../..")
    sim.MainLoop()
    sim.Finalize()

main()
