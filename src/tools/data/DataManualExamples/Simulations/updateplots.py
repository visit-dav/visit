# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

import math, os, sys
sys.path.append("../../../lib")                # for _simV2.so
sys.path.append("../../../sim/V2/swig/python") # for simV2.py

from simV2 import *

# Uncomment the next 2 lines for parallel
#sys.path.append("../../../lib/site-packages") # for mpicom
#import mpicom

#*****************************************************************************
# Class: Simulation
#
# Purpose:
#   This class implements a simple simulation using simV2.
#
# Programmer: Brad Whitlock
# Date:       Fri Mar 18 14:24:17 PDT 2011
#
# Modifications:
#  Cyrus Harrison, Tue Aug  7 09:50:58 PDT 2012
#  Change a few instances of 'sim' to 'self'.
#
#  Kathleen Biagas, Mon Oct 17 14:31:42 PDT 2016
#  Added call to 'VisItFinalize' at completion of MainLoop to prevent crash
#  on exit when running in parallel.
#
#  Kathleen Biagas, Fri Sep 10 10:27:23 PDT 2021
#  Added exportVTK option. Set runMode to STOPPED.
#
#*****************************************************************************

class Simulation:
    def __init__(self):
        self.done = 0
        self.cycle = 0
        self.time = 0.
        self.runMode = VISIT_SIMMODE_STOPPED #RUNNING
        self.par_size = 1
        self.par_rank = 0
        self.savingFiles = 0
        self.saveCounter = 0
        self.rmesh_dims = [50,50,1]
        self.rmesh_ndims = 2
        self.commands = ("halt", "step", "run", "addplot", "saveon", "saveoff", "exportVTK")

    def Initialize(self, simname, visitdir):
        VisItOpenTraceFile("trace.%d.txt" % self.par_rank)
        VisItSetDirectory(visitdir)
        VisItSetupEnvironment()
        if self.par_rank == 0:
            VisItInitializeSocketAndDumpSimFile(simname, 
                "Python simulation", "/path/to/where/visit/was/started",
                 None, None, None)

    def Finalize(self):
        VisItCloseTraceFile()

    def export_vtk(self):
        filename="updateplots%04d" %self.saveCounter
        opts = VISIT_INVALID_HANDLE;
        hvars = VisIt_NameList_alloc()
        if hvars != VISIT_INVALID_HANDLE:
            VisIt_NameList_addName(hvars, "default")

            opts = VisIt_OptionList_alloc()
            if opts != VISIT_INVALID_HANDLE:
                # FileFormat 0: Legacy ASCII
                # FileFormat 1: Legacy Binary
                # FileFormat 2: XML ASCII
                # FileFormat 3: XML Binary
                VisIt_OptionList_setValueE(opts, "FileFormat", 3)

                if VisItExportDatabaseWithOptions(filename, "VTK_1.0",
                                              hvars, opts) == VISIT_OKAY:
                    self.saveCounter += 1
                    print("Exported",filename)
                else:
                    print("The db could not be exported to",filename)
            else:
                print("could not allocate options for export")

            if opts != VISIT_INVALID_HANDLE:
                VisIt_OptionList_free(opts)

            VisIt_NameList_free(hvars)

    def SimulateOneTimestep(self):
        self.cycle = self.cycle + 1
        self.time = self.time + math.pi/10.
        if self.par_rank == 0:
            print("Simulating time step: cycle=%d, time=%g" % (self.cycle,self.time))
        VisItTimeStepChanged()
        VisItUpdatePlots()
        if self.savingFiles:
            filename = "updateplots%04d.jpg" % self.saveCounter
            if VisItSaveWindow(filename, 800, 800, VISIT_IMAGEFORMAT_JPEG) == VISIT_OKAY:
                self.saveCounter += 1
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
            VisItExecuteCommand('AddPlot("Pseudocolor", "zonal")\n')
            VisItExecuteCommand('DrawPlots()\n')
        elif cmd == "exportVTK":
            self.export_vtk()

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
        VisItFinalize()

    def ConnectCallbacks(self):
        VisItSetCommandCallback(self.ControlCommandCallback, 0)
        VisItSetGetMetaData(self.GetMetaData, None)
        VisItSetGetMesh(self.GetMesh, 0)
        VisItSetGetVariable(self.GetVariable, 0)
        VisItSetGetCurve(self.GetCurve, 0)

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
            VisItExecuteCommand('AddPlot("Pseudocolor", "zonal")\n')
            VisItExecuteCommand('DrawPlots()\n')

    def GetMetaData(self, cbdata):
        md = VisIt_SimulationMetaData_alloc()
        if md != VISIT_INVALID_HANDLE:
            # Add mesh metadata.
            mmd = VisIt_MeshMetaData_alloc()
            if mmd != VISIT_INVALID_HANDLE:
                # Set the mesh's properties.
                VisIt_MeshMetaData_setName(mmd, "mesh2d")
                VisIt_MeshMetaData_setMeshType(mmd, VISIT_MESHTYPE_RECTILINEAR)
                VisIt_MeshMetaData_setTopologicalDimension(mmd, 2)
                VisIt_MeshMetaData_setSpatialDimension(mmd, 2)
                VisIt_MeshMetaData_setNumDomains(mmd, self.par_size)
                VisIt_MeshMetaData_setDomainTitle(mmd, "Domains")
                VisIt_MeshMetaData_setDomainPieceName(mmd, "domain")
                VisIt_MeshMetaData_setNumGroups(mmd, 0)
                VisIt_MeshMetaData_setXUnits(mmd, "cm")
                VisIt_MeshMetaData_setYUnits(mmd, "cm")
                VisIt_MeshMetaData_setZUnits(mmd, "cm")
                VisIt_MeshMetaData_setXLabel(mmd, "Width")
                VisIt_MeshMetaData_setYLabel(mmd, "Height")
                VisIt_MeshMetaData_setZLabel(mmd, "Depth")

                VisIt_SimulationMetaData_addMesh(md, mmd)

            # Add a variable.
            vmd = VisIt_VariableMetaData_alloc()
            if vmd != VISIT_INVALID_HANDLE:
                VisIt_VariableMetaData_setName(vmd, "zonal")
                VisIt_VariableMetaData_setMeshName(vmd, "mesh2d")
                VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_SCALAR)
                VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_ZONE)
    
                VisIt_SimulationMetaData_addVariable(md, vmd)

            # Add a curve variable.
            cmd = VisIt_CurveMetaData_alloc()
            if cmd != VISIT_INVALID_HANDLE:
                VisIt_CurveMetaData_setName(cmd, "sine")
                VisIt_CurveMetaData_setXLabel(cmd, "Angle")
                VisIt_CurveMetaData_setXUnits(cmd, "radians")
                VisIt_CurveMetaData_setYLabel(cmd, "Amplitude")
                VisIt_CurveMetaData_setYUnits(cmd, "")

                VisIt_SimulationMetaData_addCurve(md, cmd)

            # Add an expression.
            emd = VisIt_ExpressionMetaData_alloc()
            if emd != VISIT_INVALID_HANDLE:
                VisIt_ExpressionMetaData_setName(emd, "zvec")
                VisIt_ExpressionMetaData_setDefinition(emd, "{zonal, zonal}")
                VisIt_ExpressionMetaData_setType(emd, VISIT_VARTYPE_VECTOR)

                VisIt_SimulationMetaData_addExpression(md, emd)

            # Add some commands
            for c in self.commands:
                cmd = VisIt_CommandMetaData_alloc()
                if cmd != VISIT_INVALID_HANDLE:
                    VisIt_CommandMetaData_setName(cmd, c)
                    VisIt_SimulationMetaData_addGenericCommand(md, cmd)
        return md

    def GetMesh(self, domain, name, cbdata):
        h = VISIT_INVALID_HANDLE
        if name == "mesh2d":
            h = VisIt_RectilinearMesh_alloc()
            if h != VISIT_INVALID_HANDLE:
                minRealIndex = [0,0,0]
                maxRealIndex = [0,0,0]

                maxRealIndex[0] = self.rmesh_dims[0]-1;
                maxRealIndex[1] = self.rmesh_dims[1]-1;
                maxRealIndex[2] = self.rmesh_dims[2]-1;

                rmesh_x = []
                for i in range(self.rmesh_dims[0]):
                    t = float(i) / float(self.rmesh_dims[0]-1)
                    rmesh_x = rmesh_x + [t * 5. - 2.5 + 5. * domain]
                rmesh_y = []
                for i in range(self.rmesh_dims[1]):
                    t = float(i) / float(self.rmesh_dims[1]-1)
                    rmesh_y = rmesh_y + [t * 5. - 2.5]

                hx = VisIt_VariableData_alloc()
                hy = VisIt_VariableData_alloc()
                VisIt_VariableData_setDataD(hx, VISIT_OWNER_VISIT, 1, self.rmesh_dims[0], rmesh_x)
                VisIt_VariableData_setDataD(hy, VISIT_OWNER_VISIT, 1, self.rmesh_dims[1], rmesh_y)
                VisIt_RectilinearMesh_setCoordsXY(h, hx, hy)
                VisIt_RectilinearMesh_setRealIndices(h, minRealIndex, maxRealIndex)
        return h

    def GetVariable(self, domain, name, cbdata):
        h = VISIT_INVALID_HANDLE
        if name == "zonal":
            sx = -2.5  + domain * 5.
            ex = sx + 5.
            sy = -2.5
            ey = sy + 5.

            # Calculate a zonal variable that moves around.
            rmesh_zonal = []
            angle = self.time
            xpos = 2.5 * math.cos(angle)
            ypos = 2.5 * math.sin(angle)
            for j in range(self.rmesh_dims[1]-1):
                ty = float(j) / float(self.rmesh_dims[1]-1-1)
                cellY = (1.-ty)*sy + ey*ty
                dY = cellY - ypos
                for i in range(self.rmesh_dims[0]-1):
                    tx = float(i) / float(self.rmesh_dims[0]-1-1)
                    cellX = (1.-tx)*sx + ex*tx
                    dX = cellX - xpos
                    rmesh_zonal = rmesh_zonal + [math.sqrt(dX * dX + dY * dY)]

            nTuples = (self.rmesh_dims[0]-1) * (self.rmesh_dims[1]-1)
            h = VisIt_VariableData_alloc()
            VisIt_VariableData_setDataD(h, VISIT_OWNER_VISIT, 1,
                nTuples, rmesh_zonal)
        return h

    def GetCurve(self, name, cbdata):
        h = VISIT_INVALID_HANDLE;
        if name == "sine":
            h = VisIt_CurveData_alloc()
            if h != VISIT_INVALID_HANDLE:
                x = [0.] * 200
                y = [0.] * 200
        
                for i in range(200):
                    angle = self.time + (float(i) / float(200-1)) * 4. * math.pi
                    x[i] = angle
                    y[i] = math.sin(x[i])

                # Give the arrays to VisIt. VisIt will free them.
                hxc = VisIt_VariableData_alloc()
                hyc = VisIt_VariableData_alloc()
                VisIt_VariableData_setDataD(hxc, VISIT_OWNER_VISIT, 1, 200, x)
                VisIt_VariableData_setDataD(hyc, VISIT_OWNER_VISIT, 1, 200, y)
                VisIt_CurveData_setCoordsXY(h, hxc, hyc)
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
#   Brad Whitlock, Fri Mar 15 11:46:21 PDT 2013
#   Make it use VisItSetupEnvironment2.
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

        VisItOpenTraceFile("trace.%d.txt" % self.par_rank)
        VisItSetDirectory(visitdir)

        # Set up the environment
        env = ""
        if self.par_rank == 0:
            env = VisItGetEnvironment()
        VisItSetupEnvironment2(env)

        # Write the sim file.
        if self.par_rank == 0:
            VisItInitializeSocketAndDumpSimFile(simname, 
                "Python simulation", "/path/to/where/visit/was/started",
                 None, None, None)

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
        VisItSetWorkerProcessCallback(self.worker_process_callback)
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

    def worker_process_callback(self):
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
    sim = Simulation()
    # update this to point to the correct location of visit's bin directory (don't include bin)
    sim.Initialize("updateplots", "../../..")
    #sim = ParallelSimulation()
    #sim.Initialize("updateplots_par", "../../..")
    sim.MainLoop()
    sim.Finalize()

main()
