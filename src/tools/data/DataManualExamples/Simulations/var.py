# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

import math, os, sys, time
sys.path.append("../../../lib")                # for _simV2.so
sys.path.append("../../../sim/V2/swig/python") # for simV2.py

from simV2 import *

#*****************************************************************************
# Class: Simulation
#
# Purpose:
#   This class implements a simple simulation using simV2.  It is the
#   python version of the 'var.c' example.
#
# Programmer: Kathleen Biagas 
# Date:       Jun 5, 2014
#
# Modifications:
#
#*****************************************************************************

class Simulation:
    def __init__(self):
        self.done = 0
        self.cycle = 0
        self.time = 0.
        self.rmesh_dims   = [4, 5, 1]
        self.rmesh_x      = [0., 1., 2.5, 5.]
        self.rmesh_y      = [0., 2., 2.25, 2.55, 5.]
        self.cmesh_dims   = [4, 3, 2];
        self.cmesh_x      = [0.,1.,2.,3., 0.,1.,2.,3., 0.,1.,2.,3.,
                             0.,1.,2.,3., 0.,1.,2.,3., 0.,1.,2.,3.]
        self.cmesh_y      = [ 0.5,0.,0.,0.5, 1.,1.,1.,1., 1.5,2.,2.,1.5,
                              0.5,0.,0.,0.5, 1.,1.,1.,1., 1.5,2.,2.,1.5 ]
        self.cmesh_z      = [ 0.,0.,0.,0., 0.,0.,0.,0., 0.,0.,0.,0.,
                              1.,1.,1.,1., 1.,1.,1.,1., 1.,1.,1.,1. ]

        self.zonal        = [1.,2.,3.,4.,5.,6.,7.,8.,9.,10.,11.,12.]
        self.zonal_vector = [ [1.,2.],[3.,4.],[5.,6.],[7.,8.],[9.,10.],[11.,12.],
                              [13.,14.],[15.,16.],[17.,18.],[19.,20.],[21.,22.],[23.,24.] ]
        self.zonal_array  = [ [1., 2.,0.,0.],[3.,4.,0.,0.],[5.,6.,0.,0.],[7.,8.,0.,0.],
                              [9.,10.,0.,0.],[11.,12.,0.,0.],[13.,14.,0.,0.],[15.,16.,0.,0.],
                              [17.,18.,0.,0.],[19.,20.,0.,0.],[21.,22.,0.,0.],[23.,24.,0.,0.] ]

        # each label needs to have same size, and all need to be NUL terminated. 
        # This variable has 7 components
        self.zonal_labels = ["zone1\0\0", "zone2\0\0", "zone3\0\0", "zone4\0\0", "zone5\0\0", "zone6\0\0", "zone7\0\0", "zone8\0\0", "zone9\0\0", "zone10\0", "zone11\0", "zone12\0"]

        self.nodal        = [ [[1.,2.,3.,4.],[5.,6.,7.,8.],[9.,10.,11.,12]],
                              [[13.,14.,15.,16.],[17.,18.,19.,20.],[21.,22.,23.,24.]] ]
        self.nodal_vector = [ [ [[ 0., 1., 2.],[ 3., 4., 5.],[ 6., 7., 8.],[ 9.,10.,11.]],
                                [[12.,13.,14.],[15.,16.,17.],[18.,19.,20.],[21.,22.,23.]],
                                [[24.,25.,26.],[27.,28.,29.],[30.,31.,32.],[33.,34.,35.]] ],
                              [ [[36.,37.,38.],[39.,40.,41.],[42.,43.,44.],[45.,46.,47.]],
                                [[48.,49.,50.],[51.,52.,53.],[54.,55.,56.],[57.,58.,59.]],
                                [[60.,61.,62.],[63.,64.,65.],[66.,67.,68.],[69.,70.,71]] ] ]
        self.nodal_array  = [ 
                   [ [[ 0.,1.,2.,0.],[3.,4.,5.,0.],[6.,7.,8.,0.],[9.,10.,11.,0.]],
                     [[12.,13.,14.,0.],[15.,16.,17.,0.],[18.,19.,20.,0.],[21.,22.,23.,0.]],
                     [[24.,25.,26.,0.],[27.,28.,29.,0.],[30.,31.,32.,0.],[33.,34.,35.,0.]] ],
                   [ [[36.,37.,38.,0.],[39.,40.,41.,0.],[42.,43.,44.,0.],[45.,46.,47.,0.]],
                     [[48.,49.,50.,0.],[51.,52.,53.,0.],[54.,55.,56.,0.],[57.,58.,59.,0.]],
                     [[60.,61.,62.,0.],[63.,64.,65.,0.],[66.,67.,68.,0.],[69.,70.,71.,0.]] ] ]

        self.runMode = VISIT_SIMMODE_STOPPED
        self.commands = ("halt", "step", "run", "update")

    def Initialize(self, simname, visitdir):
        VisItOpenTraceFile("trace.txt")
        VisItSetDirectory(visitdir)
        VisItSetupEnvironment()
        VisItInitializeSocketAndDumpSimFile(simname, 
            "Python simulation", "/path/to/where/visit/was/started",
            None, None, None)

    def Finalize(self):
        VisItCloseTraceFile()

    def SimulateOneTimestep(self):
        self.cycle = self.cycle + 1
        self.time = self.time + 0.0134
        print("Simulating time step: cycle=%d, time=%lg" %(self.cycle, self.time))
        time.sleep(1)

    def ProcessVisItCommand(self):
        return (VisItProcessEngineCommand() == VISIT_OKAY)

    def DoPrompt(self):
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
        elif cmd == "update":
            VisItTimeStepChanged()
            VisItUpdatePlots()

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
        elif cmd == "update":
            VisItTimeStepChanged()
            VisItUpdatePlots()


    def GetMetaData(self, cbdata):
        md = VisIt_SimulationMetaData_alloc()
        if md != VISIT_INVALID_HANDLE:
            # Add 2d mesh metadata.
            m1 = VisIt_MeshMetaData_alloc()
            if m1 != VISIT_INVALID_HANDLE:
                # Set the mesh's properties.
                VisIt_MeshMetaData_setName(m1, "mesh2d")
                VisIt_MeshMetaData_setMeshType(m1, VISIT_MESHTYPE_RECTILINEAR)
                VisIt_MeshMetaData_setTopologicalDimension(m1, 2)
                VisIt_MeshMetaData_setSpatialDimension(m1, 2)
                VisIt_MeshMetaData_setXUnits(m1, "cm")
                VisIt_MeshMetaData_setYUnits(m1, "cm")
                VisIt_MeshMetaData_setXLabel(m1, "Width")
                VisIt_MeshMetaData_setYLabel(m1, "Height")

                VisIt_SimulationMetaData_addMesh(md, m1)

            # Add 3d mesh metadata.
            m2 = VisIt_MeshMetaData_alloc()
            if m2 != VISIT_INVALID_HANDLE:
                # Set the mesh's properties.
                VisIt_MeshMetaData_setName(m2, "mesh3d")
                VisIt_MeshMetaData_setMeshType(m2, VISIT_MESHTYPE_CURVILINEAR)
                VisIt_MeshMetaData_setTopologicalDimension(m2, 3)
                VisIt_MeshMetaData_setSpatialDimension(m2, 3)
                VisIt_MeshMetaData_setXUnits(m2, "cm")
                VisIt_MeshMetaData_setYUnits(m2, "cm")
                VisIt_MeshMetaData_setZUnits(m2, "cm")
                VisIt_MeshMetaData_setXLabel(m2, "Width")
                VisIt_MeshMetaData_setYLabel(m2, "Height")
                VisIt_MeshMetaData_setZLabel(m2, "Depth")
            
                VisIt_SimulationMetaData_addMesh(md, m2)

            # Add a zonal scalar variable on mesh2d
            vmd1 = VisIt_VariableMetaData_alloc()
            if vmd1 != VISIT_INVALID_HANDLE:
                VisIt_VariableMetaData_setName(vmd1, "zonal_scalar")
                VisIt_VariableMetaData_setMeshName(vmd1, "mesh2d")
                VisIt_VariableMetaData_setType(vmd1, VISIT_VARTYPE_SCALAR)
                VisIt_VariableMetaData_setCentering(vmd1, VISIT_VARCENTERING_ZONE)
    
                VisIt_SimulationMetaData_addVariable(md, vmd1)

            # Add a nodal scalar variable on mesh3d
            vmd2 = VisIt_VariableMetaData_alloc()
            if vmd2 != VISIT_INVALID_HANDLE:
                VisIt_VariableMetaData_setName(vmd2, "nodal_scalar")
                VisIt_VariableMetaData_setMeshName(vmd2, "mesh3d")
                VisIt_VariableMetaData_setType(vmd2, VISIT_VARTYPE_SCALAR)
                VisIt_VariableMetaData_setCentering(vmd2, VISIT_VARCENTERING_NODE)
    
                VisIt_SimulationMetaData_addVariable(md, vmd2)

            # Add a zonal vector variable on mesh2d
            vmd3 = VisIt_VariableMetaData_alloc()
            if vmd3 != VISIT_INVALID_HANDLE:
                VisIt_VariableMetaData_setName(vmd3, "zonal_vector")
                VisIt_VariableMetaData_setMeshName(vmd3, "mesh2d")
                VisIt_VariableMetaData_setType(vmd3, VISIT_VARTYPE_VECTOR)
                VisIt_VariableMetaData_setCentering(vmd3, VISIT_VARCENTERING_ZONE)
    
                VisIt_SimulationMetaData_addVariable(md, vmd3)

            # Add a nodal vector variable on mesh3d
            vmd4 = VisIt_VariableMetaData_alloc()
            if vmd4 != VISIT_INVALID_HANDLE:
                VisIt_VariableMetaData_setName(vmd4, "nodal_vector")
                VisIt_VariableMetaData_setMeshName(vmd4, "mesh3d")
                VisIt_VariableMetaData_setType(vmd4, VISIT_VARTYPE_VECTOR)
                VisIt_VariableMetaData_setCentering(vmd4, VISIT_VARCENTERING_NODE)
    
                VisIt_SimulationMetaData_addVariable(md, vmd4)

            # Add a zonal label variable on mesh2d
            vmd5 = VisIt_VariableMetaData_alloc()
            if vmd5 != VISIT_INVALID_HANDLE:
                VisIt_VariableMetaData_setName(vmd5, "zonal_label")
                VisIt_VariableMetaData_setMeshName(vmd5, "mesh2d")
                VisIt_VariableMetaData_setType(vmd5, VISIT_VARTYPE_LABEL)
                VisIt_VariableMetaData_setCentering(vmd5, VISIT_VARCENTERING_ZONE)
    
                VisIt_SimulationMetaData_addVariable(md, vmd5)

            # Add a zonal array variable on mesh2d
            vmd6 = VisIt_VariableMetaData_alloc()
            if vmd6 != VISIT_INVALID_HANDLE:
                VisIt_VariableMetaData_setName(vmd6, "zonal_array")
                VisIt_VariableMetaData_setMeshName(vmd6, "mesh2d")
                VisIt_VariableMetaData_setType(vmd6, VISIT_VARTYPE_ARRAY)
                VisIt_VariableMetaData_setCentering(vmd6, VISIT_VARCENTERING_ZONE)
                VisIt_VariableMetaData_setNumComponents(vmd6, 4)
    
                VisIt_SimulationMetaData_addVariable(md, vmd6)

            # Add a nodal array variable on mesh3d
            vmd7 = VisIt_VariableMetaData_alloc()
            if vmd7 != VISIT_INVALID_HANDLE:
                VisIt_VariableMetaData_setName(vmd7, "nodal_array")
                VisIt_VariableMetaData_setMeshName(vmd7, "mesh3d")
                VisIt_VariableMetaData_setType(vmd7, VISIT_VARTYPE_ARRAY)
                VisIt_VariableMetaData_setCentering(vmd7, VISIT_VARCENTERING_NODE)
                VisIt_VariableMetaData_setNumComponents(vmd7, 4)
    
                VisIt_SimulationMetaData_addVariable(md, vmd7)

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
                hx = VisIt_VariableData_alloc()
                hy = VisIt_VariableData_alloc()
                VisIt_VariableData_setDataD(hx, VISIT_OWNER_SIM, 1, self.rmesh_dims[0], self.rmesh_x)
                VisIt_VariableData_setDataD(hy, VISIT_OWNER_SIM, 1, self.rmesh_dims[1], self.rmesh_y)
                VisIt_RectilinearMesh_setCoordsXY(h, hx, hy)
        elif name == "mesh3d":
            h = VisIt_CurvilinearMesh_alloc()
            if h != VISIT_INVALID_HANDLE:
                nn = self.cmesh_dims[0] * self.cmesh_dims[1] * self.cmesh_dims[2]
                hx = VisIt_VariableData_alloc()
                hy = VisIt_VariableData_alloc()
                hz = VisIt_VariableData_alloc()
                VisIt_VariableData_setDataD(hx, VISIT_OWNER_SIM, 1, nn, self.cmesh_x)
                VisIt_VariableData_setDataD(hy, VISIT_OWNER_SIM, 1, nn, self.cmesh_y)
                VisIt_VariableData_setDataD(hz, VISIT_OWNER_SIM, 1, nn, self.cmesh_z)
                VisIt_CurvilinearMesh_setCoordsXYZ(h, self.cmesh_dims, hx, hy, hz)
        return h

    def GetVariable(self, domain, name, cbdata):
        h = VISIT_INVALID_HANDLE
        h = VisIt_VariableData_alloc()
        nZTuples = (self.rmesh_dims[0]-1) * (self.rmesh_dims[1]-1)
        nNTuples = self.cmesh_dims[0] * self.cmesh_dims[1] * self.cmesh_dims[2]

        if name == "zonal_scalar":
            VisIt_VariableData_setDataD(h, VISIT_OWNER_SIM, 1, nZTuples, self.zonal)
        elif name == "nodal_scalar":
            VisIt_VariableData_setDataD(h, VISIT_OWNER_SIM, 1, nNTuples, self.nodal)
        elif name == "zonal_vector":
            VisIt_VariableData_setDataD(h, VISIT_OWNER_SIM, 2, nZTuples, self.zonal_vector)
        elif name == "nodal_vector":
            VisIt_VariableData_setDataD(h, VISIT_OWNER_SIM, 3, nNTuples, self.nodal_vector)
        elif name == "zonal_label":
            VisIt_VariableData_setDataC(h, VISIT_OWNER_SIM, 7, nZTuples, self.zonal_labels)
        elif name == "zonal_array":
            VisIt_VariableData_setDataD(h, VISIT_OWNER_SIM, 4, nZTuples, self.zonal_array)
        elif name == "nodal_array":
            VisIt_VariableData_setDataD(h, VISIT_OWNER_SIM, 4, nNTuples, self.nodal_array)
        else:
            VisIt_VariableData_free(h)
            h = VISIT_INVALID_HANDLE
         
        return h


#
# Main program
#
def main():
    sim = Simulation()
    sim.Initialize("var", "../../..")
    sim.MainLoop()
    sim.Finalize()

main()

