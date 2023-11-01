# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

import math, os, string, sys
sys.path.append("../../../lib")                # for _simV2.so
sys.path.append("../../../sim/V2/swig/python") # for simV2.py

from simV2 import *

# Uncomment the next 2 lines for parallel
sys.path.append("../../../lib/site-packages") # for mpicom
import mpicom

#*****************************************************************************
# Functions: Extract functions
#
# Purpose:
#   Use Libsim plotting functions to make extracts.
#
# Programmer: Brad Whitlock
# Date:       Tue Sep  6 14:26:41 PDT 2016
#
# Modifications:
#
#*****************************************************************************

extract_options_exportFormat = "FieldViewXDB_1.0"
extract_options_writeUsingGroups = 0;
extract_options_writeGroupSize = 1;

def extract_set_options(fmt, writeUsingGroups, groupSize):
    extract_options_exportFormat = fmt
    extract_options_writeUsingGroups = writeUsingGroups
    extract_options_writeGroupSize = groupSize

def extract_err(err):
    ret = ""
    if err == 0:
        ret = "Success"
    elif err == -1:
        ret = "Could not create plot and draw it."
    elif err == -2:
        ret = "Export failed."
    else:
        ret = "?"
    return ret

def export_visit(filebase, extractvars):
    retval = -1
    opts = VISIT_INVALID_HANDLE;
    hvars = VisIt_NameList_alloc()
    if hvars != VISIT_INVALID_HANDLE:
        VisIt_NameList_addName(hvars, "default")
        for v in extractvars:
            VisIt_NameList_addName(hvars, v)

        opts = VisIt_OptionList_alloc()
        if opts != VISIT_INVALID_HANDLE:
            VisIt_OptionList_setValueI(opts, VISIT_EXPORT_WRITE_USING_GROUPS,
                                       extract_options_writeUsingGroups)
            VisIt_OptionList_setValueI(opts, VISIT_EXPORT_GROUP_SIZE,
                                       extract_options_writeGroupSize)

        if VisItExportDatabaseWithOptions(filebase, extract_options_exportFormat,
                                          hvars, opts) == VISIT_OKAY:
            retval = 0
        else:
            retval = -2

        if opts != VISIT_INVALID_HANDLE:
            VisIt_OptionList_free(opts)

        VisIt_NameList_free(hvars)

    return retval


def extract_slice_origin_normal(filebase, origin, normal, extractvars):
    retval = -1
    if len(filebase) == 0 or len(origin) != 3 or len(normal) != 3 or len(extractvars) == 0:
        return retval;

    if VisItAddPlot("Pseudocolor", extractvars[0]) == VISIT_OKAY:
        if VisItAddOperator("Slice", 0) == VISIT_OKAY:
            VisItSetOperatorOptionsI("axisType", 3) # arbitrary 
            VisItSetOperatorOptionsI("originType", 0) # point intercept 
            VisItSetOperatorOptionsDv("originPoint", origin, 3)
            VisItSetOperatorOptionsDv("normal", normal, 3)
            VisItSetOperatorOptionsB("project2d", 0)

            if VisItDrawPlots() == VISIT_OKAY:
                retval = export_visit(filebase, extractvars)

        VisItDeleteActivePlots()

    return retval

def extract_slice_3v(filebase, v0, v1, v2, extractvars):
    origin = [0,0,0]
    normal = [0,0,0]
    vec1 = [0,0,0]
    vec2 = [0,0,0]

    origin[0] = v0[0]
    origin[1] = v0[1]
    origin[2] = v0[2]

    vec1[0] = v1[0] - v0[0]
    vec1[1] = v1[1] - v0[1]
    vec1[2] = v1[2] - v0[2]
    mag = math.sqrt(vec1[0]*vec1[0] + vec1[1]*vec1[1] + vec1[2]*vec1[2])
    if mag > 0.:
        vec1[0] /= mag
        vec1[1] /= mag
        vec1[2] /= mag

    vec2[0] = v2[0] - v0[0]
    vec2[1] = v2[1] - v0[1]
    vec2[2] = v2[2] - v0[2]
    mag = math.sqrt(vec2[0]*vec2[0] + vec2[1]*vec2[1] + vec2[2]*vec2[2])
    if mag > 0.:
        vec2[0] /= mag
        vec2[1] /= mag
        vec2[2] /= mag

    normal[0] = vec1[1]*vec2[2] - vec1[2]*vec2[1]
    normal[1] = vec1[2]*vec2[0] - vec1[0]*vec2[2]
    normal[2] = vec1[0]*vec2[1] - vec1[1]*vec2[0]

    return extract_slice_origin_normal(filebase, origin, normal, extractvars)

def extract_slice(filebase, axis, intercept, extractvars):
    retval = -1
    if len(filebase) == 0 or len(extractvars) == 0:
        return retval

    if VisItAddPlot("Pseudocolor", extractvars[0]) == VISIT_OKAY:
        if VisItAddOperator("Slice", 0) == VISIT_OKAY:
            VisItSetOperatorOptionsI("axisType", axis)
            VisItSetOperatorOptionsI("originType", 1) # intercept 
            VisItSetOperatorOptionsD("originIntercept", intercept)
            VisItSetOperatorOptionsB("project2d", 0)

            if VisItDrawPlots() == VISIT_OKAY:
                retval = export_visit(filebase, extractvars)

        VisItDeleteActivePlots()

    return retval

def extract_iso(filebase, isovar, isovalues, extractvars):
    retval = -1
    if len(filebase) == 0 or len(isovar) == 0 or len(isovalues) == 0 or len(extractvars) == 0:
         return -1

    if VisItAddPlot("Contour", isovar) == VISIT_OKAY:
        VisItSetPlotOptionsI("contourMethod", 1) # value
        VisItSetPlotOptionsDv("contourValue", isovalues, len(isovalues))
        if VisItDrawPlots() == VISIT_OKAY:
            retval = export_visit(filebase, extractvars)

        VisItDeleteActivePlots()

    return retval


#*****************************************************************************
# Class: Batch
#
# Purpose:
#   This class implements a simple batch simulation using simV2.
#
# Programmer: Brad Whitlock
# Date:       Tue Sep  6 14:26:41 PDT 2016
#
# Modifications:
#
#*****************************************************************************

class Batch(object):
    def __init__(self):
        super(Batch, self).__init__()
        self.VISIT_COMMAND_PROCESS = 0
        self.VISIT_COMMAND_SUCCESS = 1
        self.VISIT_COMMAND_FAILURE = 2

        self.done = 0
        self.par_size = 1
        self.par_rank = 0
        self.maxcycles = 1000000
        self.cycle = 0
        self.time = 0.
        self.runMode = VISIT_SIMMODE_RUNNING #STOPPED
        self.format = "FieldViewXDB_1.0"
        self.domains = [1,1,1]
        self.dims = [50,50,50]
        self.extents = [0., 10., 0., 10., 0., 10.]
        self.groupSize = -1
        self.export = 1
        self.render = 0
        self.image_width = 1920 / 2
        self.image_height = 1080 / 2
        self.x = None
        self.y = None
        self.z = None
        self.q = None
        self.commands = ("halt", "step", "run", "addplot")
        self.tracefile = None

    def Execute(self):
        """The main function where we start to execute the simulation."""
        self.Initialize()

        options = "-plotplugins Contour,Mesh,Pseudocolor -operatorplugins Slice,Isosurface,Threshold -noconfig"

        # Check for command line arguments.
        i = 0
        argc = len(sys.argv)
        while i < argc:
            if i+1<argc:
                if sys.argv[i] == "-dims":
                    self.dims = [int(x) for x in string.split(sys.argv[i+1],",")]
                    i = i + 2
                elif i+1<argc and sys.argv[i] == "-domains":
                    self.domains = [int(x) for x in string.split(sys.argv[i+1],",")]
                    i = i + 2
                elif sys.argv[i] == "-maxcycles":
                    self.maxcycles = int(sys.argv[i+1])
                    i = i + 2
                elif sys.argv[i] == "-dir":
                    # Specify the path to VisIt installation.
                    VisItSetDirectory(sys.argv[i+1])
                    i = i + 2
                elif sys.argv[i] == "-groupsize":
                    self.groupSize = int(sys.argv[i+1])
                    i = i + 2
                elif sys.argv[i] == "-trace":
                    self.tracefile = "%s.%d.log" % (sys.argv[i+1], self.par_rank)
                    VisItOpenTraceFile(self.tracefile)
                    i = i + 2
                elif sys.argv[i] == "-format":
                    strncpy(self.format, sys.argv[i+1], 30)
                    i = i + 2
                elif sys.argv[i] == "-export":
                    self.export = int(sys.argv[i+1])
                    i = i + 2
                elif sys.argv[i] == "-render":
                    self.render = int(sys.argv[i+1])
                    i = i + 2
                elif sys.argv[i] == "-image-width":
                    self.image_width = int(sys.argv[i+1])
                    i = i + 2
                elif sys.argv[i] == "-image-height":
                    self.image_height = int(sys.argv[i+1])
                    i = i + 2
                else:
                    if len(options) > 0:
                        options = options + " " + sys.argv[i]
                    else:
                        options = sys.argv[i]
                    i = i + 1
            else:
                if len(options) > 0:
                    options = options + " " + sys.argv[i]
                else:
                    options = sys.argv[i]
                i = i + 1

        if self.domains[0]*self.domains[1]*self.domains[2] != self.par_size:
            if self.par_rank == 0:
                print("The number of domains must match the number of ranks.\n")
            self.Finalize()
            return -1

        if len(options) > 0:
            VisItSetOptions(options)
  
        # Only read the environment on rank 0. This could happen before MPI_Init if
        # we are using an MPI that does not like to let us spawn processes but we
        # would not know our processor rank.

        env = ""
        if self.par_rank == 0:
            env = VisItGetEnvironment()
        VisItSetupEnvironment2(env)

        # Call the main loop.
        self.mainloop_batch()

        self.Finalize()
        return 0

    def Initialize(self):
        """Initialize the simulation."""
        return

    def Finalize(self):
        """Finalize the simulation."""
        if self.tracefile != None:
            VisItCloseTraceFile()
        VisItDisconnect()
        return

    def ConnectCallbacks(self):
        """Connects data adaptor callbacks for Libsim."""
        VisItSetGetMetaData(self.GetMetaData, None)
        VisItSetGetMesh(self.GetMesh, 0)
        VisItSetGetVariable(self.GetVariable, 0)

    def DomainIJK(self):
        """Determines the IJK ids for the current rank's domain."""
        NXY = self.domains[0]*self.domains[1]
        kdom = self.par_rank / (NXY)
        jdom = (self.par_rank % NXY) / self.domains[0]
        idom = (self.par_rank % NXY) % self.domains[0]
        return (idom,jdom,kdom)

    def DataUpdate(self):
        """Updates the data arrays for the current time step."""
        npts = self.dims[0] * self.dims[1] * self.dims[2]
        ncells = (self.dims[0]-1) * (self.dims[1]-1) * (self.dims[2]-1)

        if self.x == None:
            self.x = [0.] * npts
            self.y = [0.] * npts
            self.z = [0.] * npts
            self.q = [0.] * ncells

            idom,jdom,kdom = self.DomainIJK()

            # Init coordinates
            offset = [0.,0.,0.]
            offset[0] = (self.extents[1] - self.extents[0]) * idom;
            offset[1] = (self.extents[3] - self.extents[2]) * jdom;
            offset[2] = (self.extents[5] - self.extents[4]) * kdom;

            index = 0
            for k in range(self.dims[2]):
                tz = float(k) / float(self.dims[2] - 1)
                z = (1.-tz)*self.extents[4] + tz*self.extents[5]
                for j in range(self.dims[1]):
                    ty = float(j) / float(self.dims[1] - 1)
                    y = (1.-ty)*self.extents[2] + ty*self.extents[3]
                    for i in range(self.dims[0]):
                        tx = float(i) / float(self.dims[0] - 1)
                        x = (1.-tx)*self.extents[0] + tx*self.extents[1]
                        self.x[index] = x + offset[0]
                        self.y[index] = y + offset[1]
                        self.z[index] = z + offset[2]
                        index = index + 1

        # Update q
        for k in range(self.dims[2]-1):
            for j in range(self.dims[1]-1):
                for i in range(self.dims[0]-1):
                    srcIndex = k*self.dims[0]*self.dims[1] +\
                               j*self.dims[0] +\
                               i
                    destIndex = k*(self.dims[0]-1)*(self.dims[1]-1) +\
                                j*(self.dims[0]-1) +\
                                i

                    self.q[destIndex] = math.sin(self.x[srcIndex] + self.time)

    def mainloop_batch(self):
        """The main loop where we iterate until maxcycles is met."""
        extractvars = ("q", "xc", "radius", "dom")
        origin = (5., 5., 5.)
        normal = (0., 0.707, 0.707)
        isos = (5., 11., 18.)
        v0 = (1.,1.,1.)
        v1 = (5., 1.5, 7.)
        v2 = (8., 2., 5.)

        # Explicitly load VisIt runtime functions and install callbacks.
        VisItInitializeRuntime()
        self.ConnectCallbacks()

        while self.cycle < self.maxcycles:
            # Update the simulation's data.
            self.DataUpdate()

            # Tell VisIt that some metadata changed.
            VisItTimeStepChanged()

            if self.export:
                # Make some extracts.
                filebase = "slice3v_%04d" % self.cycle
                err = extract_slice_3v(filebase, v0, v1, v2, extractvars)
                if self.par_rank == 0:
                    print("slice3v export returned %s" % extract_err(err))
    
                filebase = "sliceON_%04d" % self.cycle
                err = extract_slice_origin_normal(filebase, origin, normal, extractvars)
                if self.par_rank == 0:
                    print("sliceON export returned %s" % extract_err(err))

                filebase = "sliceX_%04d" % self.cycle
                err = extract_slice(filebase, 0, 0.5, extractvars)
                if self.par_rank == 0:
                    print("sliceX export returned %s" % extract_err(err))

                filebase = "sliceY_%04d" % self.cycle
                err = extract_slice(filebase, 1, 2.5, extractvars)
                if self.par_rank == 0:
                    print("slice export returned %s" % extract_err(err))

                filebase = "sliceZ_%04d" % self.cycle
                err = extract_slice(filebase, 2, 5., extractvars)
                if self.par_rank == 0:
                    print("sliceZ export returned %s" % extract_err(err))

                filebase = "iso_%04d" % self.cycle
                err = extract_iso(filebase, "radius", isos, extractvars)
                if self.par_rank == 0:
                    print("iso export returned %s" % extract_err(err))

            if self.render:
                filename = "batch%04d.png" % self.cycle
 
                VisItAddPlot("Contour", "d")
                #VisItAddPlot("Mesh", "mesh")
                VisItDrawPlots()
                if VisItSaveWindow(filename, self.image_width, self.image_height, VISIT_IMAGEFORMAT_PNG) == VISIT_OKAY:
                    if self.par_rank == 0:
                        print("Saved", filename)
                elif self.par_rank == 0:
                    print("The image could not be saved to ", filename)
                VisItDeleteActivePlots()

            self.cycle = self.cycle + 1
            self.time = self.time + math.pi / 10.

    ############################################################################
    # Adaptor Functions
    ############################################################################
    def GetMetaData(self, cbdata):
        """Adaptor callback function for returning metadata."""
        md = VisIt_SimulationMetaData_alloc()
        if md != VISIT_INVALID_HANDLE:
            # Add mesh metadata.
            mmd = VisIt_MeshMetaData_alloc()
            if mmd != VISIT_INVALID_HANDLE:
                # Set the mesh's properties.
                VisIt_MeshMetaData_setName(mmd, "mesh")
                VisIt_MeshMetaData_setMeshType(mmd, VISIT_MESHTYPE_CURVILINEAR)
                VisIt_MeshMetaData_setTopologicalDimension(mmd, 3)
                VisIt_MeshMetaData_setSpatialDimension(mmd, 3)
                VisIt_MeshMetaData_setNumDomains(mmd, self.domains[0]*self.domains[1]*self.domains[2])
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
                VisIt_VariableMetaData_setName(vmd, "q")
                VisIt_VariableMetaData_setMeshName(vmd, "mesh")
                VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_SCALAR)
                VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_ZONE)
                VisIt_SimulationMetaData_addVariable(md, vmd)
            for var in ("xc", "radius", "dom", "d"):
                vmd = VisIt_VariableMetaData_alloc()
                if vmd != VISIT_INVALID_HANDLE:
                    VisIt_VariableMetaData_setName(vmd, var)
                    VisIt_VariableMetaData_setMeshName(vmd, "mesh")
                    VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_SCALAR)
                    VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_NODE)
                    VisIt_SimulationMetaData_addVariable(md, vmd)
        return md

    def GetMesh(self, domain, name, cbdata):
        """Adaptor callback function for returning the mesh."""
        h = VISIT_INVALID_HANDLE
        if name == "mesh":
            h = VisIt_CurvilinearMesh_alloc()
            if h != VISIT_INVALID_HANDLE:
                npts = self.dims[0]*self.dims[1]*self.dims[2]
                hx = VisIt_VariableData_alloc()
                hy = VisIt_VariableData_alloc()
                hz = VisIt_VariableData_alloc()
                VisIt_VariableData_setDataF(hx, VISIT_OWNER_SIM, 1, npts, self.x)
                VisIt_VariableData_setDataF(hy, VISIT_OWNER_SIM, 1, npts, self.y)
                VisIt_VariableData_setDataF(hz, VISIT_OWNER_SIM, 1, npts, self.z)
                VisIt_CurvilinearMesh_setCoordsXYZ(h, self.dims, hx, hy, hz)
        return h

    def GetVariable(self, domain, name, cbdata):
        """Adaptor callback function for returning variables."""
        npts = self.dims[0] * self.dims[1] * self.dims[2]
        ncells = (self.dims[0]-1) * (self.dims[1]-1) * (self.dims[2]-1)
        h = VISIT_INVALID_HANDLE
        if name == "q":
            h = VisIt_VariableData_alloc()
            VisIt_VariableData_setDataF(h, VISIT_OWNER_SIM, 1, ncells, self.q)
        elif name == "xc":
            h = VisIt_VariableData_alloc()
            VisIt_VariableData_setDataF(h, VISIT_OWNER_SIM, 1, npts, self.x)
        elif name == "radius":
            h = VisIt_VariableData_alloc()
            rad = [0.] * npts
            for index in range(npts):
                rad[index] = math.sqrt(self.x[index]*self.x[index] + 
                                       self.y[index]*self.y[index] + 
                                       self.z[index]*self.z[index])
            # Use copy since rad will go out of scope
            VisIt_VariableData_setDataD(h, VISIT_OWNER_COPY, 1, npts, rad)
        elif name == "d":
            h = VisIt_VariableData_alloc()
            rad = [0.] * npts
            for index in range(npts):
                rad[index] = math.sin(self.time + 
                                      0.25 * math.sqrt(self.x[index]*self.x[index] + 
                                       self.y[index]*self.y[index] + 
                                       self.z[index]*self.z[index]))
            # Use copy since rad will go out of scope
            VisIt_VariableData_setDataD(h, VISIT_OWNER_COPY, 1, npts, rad)
        elif name == "dom":
            h = VisIt_VariableData_alloc()
            dom = [0.] * npts
            for index in range(npts):
                dom[index] = domain
            # Use copy since dom will go out of scope
            VisIt_VariableData_setDataD(h, VISIT_OWNER_COPY, 1, npts, dom)
        return h


#*****************************************************************************
# Class: ParallelBatch
#
# Purpose:
#   Extend the batch simulation for parallel.
#
# Programmer: Brad Whitlock
# Date:       Tue Sep  6 14:26:41 PDT 2016
#
# Modifications:
#
#*****************************************************************************

class ParallelBatch(Batch):
    def __init__(self):
        super(ParallelBatch, self).__init__()

    def Initialize(self):
        mpicom.init(sys.argv)
        VisItSetBroadcastIntFunction(self.broadcast_int)
        VisItSetBroadcastStringFunction(self.broadcast_string)
        self.par_size = mpicom.size()
        self.par_rank = mpicom.rank()
        VisItSetParallel(self.par_size  >1)
        VisItSetParallelRank(self.par_rank)

    def Finalize(self):
        if self.tracefile != None:
            VisItCloseTraceFile()
        VisItDisconnect()
        mpicom.finalize()

    def ConnectCallbacks(self):
        super(ParallelBatch, self).ConnectCallbacks()
        VisItSetWorkerProcessCallback(self.worker_process_callback)
        VisItSetGetDomainList(self.GetDomainList, 0)

    ############################################################################
    # Adaptor Functions
    ############################################################################

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

    def GetDomainList(self, name, cbdata):
        h = VisIt_DomainList_alloc()
        if h != VISIT_INVALID_HANDLE:
            hdl = VisIt_VariableData_alloc()
            VisIt_VariableData_setDataI(hdl, VISIT_OWNER_VISIT, 1, 1, [self.par_rank])
            VisIt_DomainList_setDomains(h, self.par_size, hdl)
        return h

#
# Main program
#
def main():
    # Serial simulation.
    sim = Batch()

    # Parallel simulation.
    #sim = ParallelBatch()

    # Execute the simulation
    sim.Execute()

main()
