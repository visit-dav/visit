# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

import math, os, sys
sys.path.append("../../../lib")                # for _simV2.so
sys.path.append("../../../sim/V2/swig/python") # for simV2.py

from simV2 import *

#*****************************************************************************
# Class: Simulation
#
# Purpose:
#   This class implements a simple material simulation using simV2.
#
# Programmer: Brad Whitlock
# Date:       Fri Mar 18 14:24:17 PDT 2011
#
# Modifications:
#
#*****************************************************************************

class Simulation:
    def __init__(self):
        self.done = 0
        self.cycle = 0
        self.time = 0.
        self.runMode = VISIT_SIMMODE_STOPPED
        self.par_size = 1
        self.par_rank = 0

        self.XMIN = 0.
        self.XMAX = 4.
        self.YMIN = 0.
        self.YMAX = 3.
        self.rmesh_dims = [5,4,1]
        self.rmesh_ndims = 2

        self.matNames = ["Water", "Membrane", "Air"]
        # The matlist table indicates the material numbers that are found in
        # each cell. Every 3 numbers indicates the material numbers in a cell.
        # A material number of 0 means that the material entry is not used.
        self.matlist = (
                ((3,0,0),(2,3,0),(1,2,0),(1,0,0)),
                ((3,0,0),(2,3,0),(1,2,0),(1,0,0)),
                ((3,0,0),(2,3,0),(1,2,3),(1,2,0))
        )
        # The mat_vf table indicates the material volume fractions that are
        # found in a cell.
        self.mat_vf = (
                ((1.,0.,0.),(0.75,0.25,0.),  (0.8125,0.1875, 0.),(1.,0.,0.)),
                ((1.,0.,0.),(0.625,0.375,0.),(0.5625,0.4375,0.), (1.,0.,0.)),
                ((1.,0.,0.),(0.3,0.7,0.),    (0.2,0.4,0.4),      (0.55,0.45,0.))
        )
        # Make a constant field of 1. on the mesh. We'll break up the 1.0 into
        # pieces in the mixvar.
        C1=1.
        C2=2.
        C3=3.
        C4=4.
        self.zonal_scalar = (
            C1*1., C2*1., C3*1., C4*1.,
            C1*1., C2*1., C3*1., C4*1.,
            C1*1., C2*1., C3*1., C4*1.
        )
        # The mesh has some mixed cells. Mixed scalars are only defined for
        # those mixed cells. That means that clean cells have no entries
        # in the mixed scalar array. For this example, we're reproducing the
        # volume fractions for the mixed cells as the mixvar data. All cells in
        # mesh have a comment indicating their location, even if they add no data.
        # Cells that have data provide their data after the comment. See the
        # mat_vf array in the GetMaterial function to draw comparisons.
        self.mixvar = (
            C2*0.75,C2*0.25,   C3*0.8125,C3*0.1875, 
            C2*0.625,C2*0.375, C3*0.5625,C3*0.4375, 
            C2*0.3,C2*0.7,     C3*0.2,C3*0.4,C3*0.4, C4*0.55,C4*0.45
        )
        self.commands = ("halt", "step", "run", "update")

    def Initialize(self, visitdir):
        VisItOpenTraceFile("trace.txt")
        VisItSetDirectory(visitdir)

        VisItSetupEnvironment()

        VisItInitializeSocketAndDumpSimFile("material", 
            "Python material simulation", "/path/to/where/visit/was/started",
            None, None, None)

    def SimulateOneTimestep(self):
        self.cycle = self.cycle + 1
        self.time = self.time + 0.0134
        print("Simulating time step: cycle=%d, time=%g" % (self.cycle,self.time))

    def DoPrompt(self):
        if self.par_rank == 0:
            sys.stdout.write("command>")
            sys.stdout.flush()

    def ProcessConsoleCommand(self):
        cmd = ""
        if self.par_rank == 0:
            cmd = VisItReadConsole()

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

    def MainLoop(self):
        self.DoPrompt()
        while self.done == 0:
            blocking = 1
            if self.runMode == VISIT_SIMMODE_RUNNING:
                blocking = 0

            console = sys.stdin.fileno()
            # console = -1
            #visitstate = VisItDetectInput(blocking, console)
            visitstate = VisItDetectInputWithTimeout(blocking, 100000, console)

            if visitstate == 0:
                self.SimulateOneTimestep()
            elif visitstate == 1:
                if VisItAttemptToCompleteConnection() == VISIT_OKAY:
                    print("VisIt connected")
                    self.runMode = VISIT_SIMMODE_STOPPED

                    VisItSetCommandCallback(self.ControlCommandCallback, 0)
                    VisItSetGetMetaData(self.GetMetaData, 0)
                    VisItSetGetMesh(self.GetMesh, 0)
                    VisItSetGetMaterial(self.GetMaterial, 0)
                    VisItSetGetVariable(self.GetVariable, 0)
                    VisItSetGetMixedVariable(self.GetMixedVariable, 0)
                else:
                    print("VisIt did not connect")
            elif visitstate == 2:
                if not VisItProcessEngineCommand():
                    VisItDisconnect()
                    self.runMode = VISIT_SIMMODE_RUNNING
            elif visitstate == 3:
                self.ProcessConsoleCommand()
                self.DoPrompt()
            else:
                print("Error: ", visitstate)
        VisItCloseTraceFile()

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
            # Add mesh metadata.
            mmd = VisIt_MeshMetaData_alloc()
            if mmd != VISIT_INVALID_HANDLE:
                # Set the mesh's properties.
                VisIt_MeshMetaData_setName(mmd, "mesh2d")
                VisIt_MeshMetaData_setMeshType(mmd, VISIT_MESHTYPE_RECTILINEAR)
                VisIt_MeshMetaData_setTopologicalDimension(mmd, 2)
                VisIt_MeshMetaData_setSpatialDimension(mmd, 2)
                VisIt_MeshMetaData_setNumDomains(mmd, 1)
                VisIt_MeshMetaData_setDomainTitle(mmd, "Domains")
                VisIt_MeshMetaData_setDomainPieceName(mmd, "domain")
                VisIt_MeshMetaData_setNumGroups(mmd, 0)
                VisIt_MeshMetaData_setXUnits(mmd, "cm")
                VisIt_MeshMetaData_setYUnits(mmd, "cm")
                VisIt_MeshMetaData_setXLabel(mmd, "Width")
                VisIt_MeshMetaData_setYLabel(mmd, "Height")

                VisIt_SimulationMetaData_addMesh(md, mmd)

            # Add a material
            mat = VisIt_MaterialMetaData_alloc()
            if mat != VISIT_INVALID_HANDLE:
                VisIt_MaterialMetaData_setName(mat, "Material")
                VisIt_MaterialMetaData_setMeshName(mat, "mesh2d")
                VisIt_MaterialMetaData_addMaterialName(mat, self.matNames[0])
                VisIt_MaterialMetaData_addMaterialName(mat, self.matNames[1])
                VisIt_MaterialMetaData_addMaterialName(mat, self.matNames[2])

                VisIt_SimulationMetaData_addMaterial(md, mat)

            # Add a variable.
            vmd = VisIt_VariableMetaData_alloc()
            if vmd != VISIT_INVALID_HANDLE:
                VisIt_VariableMetaData_setName(vmd, "scalar")
                VisIt_VariableMetaData_setMeshName(vmd, "mesh2d")
                VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_SCALAR)
                VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_ZONE)
    
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
        if name == "mesh2d":
            h = VisIt_RectilinearMesh_alloc()
            if h != VISIT_INVALID_HANDLE:
                rmesh_x = []
                for i in range(self.rmesh_dims[0]):
                    t = float(i) / float(self.rmesh_dims[0]-1)
                    rmesh_x = rmesh_x + [(1.-t)*self.XMIN + t*self.XMAX]
                rmesh_y = []
                for i in range(self.rmesh_dims[1]):
                    t = float(i) / float(self.rmesh_dims[1]-1)
                    rmesh_y = rmesh_y + [(1.-t)*self.YMIN + t*self.YMAX]

                hx = VisIt_VariableData_alloc()
                hy = VisIt_VariableData_alloc()
                VisIt_VariableData_setDataD(hx, VISIT_OWNER_VISIT, 1, self.rmesh_dims[0], rmesh_x)
                VisIt_VariableData_setDataD(hy, VISIT_OWNER_VISIT, 1, self.rmesh_dims[1], rmesh_y)
                VisIt_RectilinearMesh_setCoordsXY(h, hx, hy)
        return h

    def GetMaterial(self, domain, name, cbdata):
        # Allocate a VisIt_MaterialData
        h = VisIt_MaterialData_alloc()
        if h != VISIT_INVALID_HANDLE:
            # Tell the object we'll be adding cells to it using add*Cell functions 
            VisIt_MaterialData_appendCells(h, (self.rmesh_dims[0]-1)*(self.rmesh_dims[1]-1))

            # Fill in the VisIt_MaterialData
            matnos = [0] * 3
            matnos[0] = VisIt_MaterialData_addMaterial(h, self.matNames[0])
            matnos[1] = VisIt_MaterialData_addMaterial(h, self.matNames[1])
            matnos[2] = VisIt_MaterialData_addMaterial(h, self.matNames[2])
    
            cell = 0
            cellmat = [0]*10
            cellmatvf = [0]*10
            for j in range(self.rmesh_dims[1]-1):
                for i in range(self.rmesh_dims[0]-1):
                    nmats = 0
                    for m in (0,1,2):
                        if self.matlist[j][i][m] > 0:
                            cellmat[nmats] = matnos[self.matlist[j][i][m] - 1]
                            cellmatvf[nmats] = self.mat_vf[j][i][m]
                            nmats = nmats + 1
                    if nmats > 1:
                        VisIt_MaterialData_addMixedCell(h, cell, cellmat, cellmatvf, nmats)
                    else:
                        VisIt_MaterialData_addCleanCell(h, cell, cellmat[0])
                    cell = cell + 1
        return h

    def GetVariable(self, domain, name, cbdata):
        h = VISIT_INVALID_HANDLE
        if name == "scalar":
            nTuples = (self.rmesh_dims[0]-1) * (self.rmesh_dims[1]-1)
            h = VisIt_VariableData_alloc()
            VisIt_VariableData_setDataD(h, VISIT_OWNER_VISIT, 1,
                nTuples, self.zonal_scalar)
        return h

    def GetMixedVariable(self, domain, name, cbdata):
        h = VISIT_INVALID_HANDLE
        if name == "scalar":
            h = VisIt_VariableData_alloc()
            VisIt_VariableData_setDataD(h, VISIT_OWNER_VISIT, 1,
                len(self.mixvar), self.mixvar)
        return h

#
# Main program
#
def main():
    sim = Simulation()
    sim.Initialize("../../..")
    sim.MainLoop()

main()
