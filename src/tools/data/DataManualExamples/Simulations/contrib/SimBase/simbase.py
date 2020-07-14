"""Simulation Base Class

Handles the basics of setting up a simulation. Actual simulations
will want to subclass this with a class which defines actual
data and metadata, and also with something that will run 
algorithm epochs.

Concepts are taken from, and occasionally idioms borrowed from,
the updateplots.py sample. Routines are named similarly where
that makes sense.
"""

# Portions derived from works
# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Copyright (c) 2014 Fred Morris, Tacoma WA.
# All rights reserved.
# Redistribution  and  use  in  source  and  binary  forms, with  or  without
# modification, are permitted provided that the following conditions are met:
#
#  - Redistributions of  source code must  retain the above copyright notice,
#    this list of conditions and the following disclaimer.
#  - Redistributions in binary form must reproduce the above copyright notice,
#    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
#    documentation and/or other materials provided with the distribution.
#  - Neither the name of the copyright holder nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
# ARE  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR  CONTRIBUTORS  BE
# LIABLE   FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
# CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT  LIMITED  TO,  PROCUREMENT  OF
# SUBSTITUTE  GOODS OR SERVICES;  LOSS  OF USE, DATA, OR PROFITS; OR  BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY  THEORY  OF  LIABILITY,  WHETHER  IN
# CONTRACT,  STRICT  LIABILITY,  OR  TORT  (INCLUDING  NEGLIGENCE OR OTHERWISE)
# ARISING  IN ANY WAY OUT OF  THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#*****************************************************************************

import sys, os

class Simulation(object):
    """Simulation Base Class. Subclass this.
    
    The Initialize and Finalize methods in this class implement
    the paradigm that there is
      * A Primary Method, which is capitalized and belongs to the base class
      * A Secondary Overridable method, which is not capitalized and is
        present for the convenience of subclasses.
    Methods which implement this paradigm accept additional kwargs
    which will be passed to the subclasses' method.
    
    The MainLoop method implements the paradigm that there is the Primary
    Method (MainLoop), and a number of main_ functions which you will 
    probably want to override. In particular:
      * main_doRun: This is where your compute kernel should run an epoch.
      * main_doVisItConnect: This is where you could connect callbacks.
      * main_doConsoleCommand: This is where you can implement console_commands.
    
    HERE'S A SUGGESTION: Use self.context to store your kernel's context. I
    promise never to step on that.
    
    The following reference arrays are defined on self in lieu of using
    the VISIT_ equivalences. They are defined as instance variables to
    get around the fact that we're importing simV2 during __init__.
    
        MESHTYPES:      VISIT_MESHTYPE_***
        VARTYPES:       VISIT_VARTYPE_***
        CENTERINGS:     VISIT_VARCENTERING_***
        COMMANDS:       Function pointers to VisIt_SimulationMetaData_add***Command
        VARDATATYPES:   Function pointers to VisIt_VariableData_setData*
        DATAOWNERS:     VISIT_OWNER_***
    """
    
    def __init__(self, lib_path, visit_base):
        """The most important things that this does are to import simV2
        and to save the VisIt base directory.
        
        Parameters:
          lib_path:     Path to simv2.py. It is probably a good idea to set
                        an environment variable VISIT_LIBPATH and pull from
                        that.
          visit_base:   Base path to the VisIt installation. In practice,
                        this is one directory above the bin/ directory that
                        VisIt runs out of. It is probably a good idea to set
                        an environment variable VISIT_HOME and pull from that.
        """
        # This is really such a glorious hack. Shout out to Quashie...
        if lib_path not in sys.path: sys.path.insert(1,lib_path)
        import simV2
        globals()['simV2'] = simV2
        
        self.visit_base = visit_base
        
        self.MESHTYPES = {
            'UNKNOWN':      simV2.VISIT_MESHTYPE_UNKNOWN,
            'RECTILINEAR':  simV2.VISIT_MESHTYPE_RECTILINEAR,
            'CURVILINEAR':  simV2.VISIT_MESHTYPE_CURVILINEAR,
            'UNSTRUCTURED': simV2.VISIT_MESHTYPE_UNSTRUCTURED,
            'POINT':        simV2.VISIT_MESHTYPE_POINT,
            'CSG':          simV2.VISIT_MESHTYPE_CSG,
            'AMR':          simV2.VISIT_MESHTYPE_AMR
            }
        self.VARTYPES = {
            'UNKNOWN':      simV2.VISIT_VARTYPE_UNKNOWN,
            'SCALAR':       simV2.VISIT_VARTYPE_SCALAR,
            'VECTOR':       simV2.VISIT_VARTYPE_VECTOR,
            'TENSOR':       simV2.VISIT_VARTYPE_TENSOR,
            'SYMMETRIC_TENSOR': simV2.VISIT_VARTYPE_SYMMETRIC_TENSOR,
            'MATERIAL':     simV2.VISIT_VARTYPE_MATERIAL,
            'MATSPECIES':   simV2.VISIT_VARTYPE_MATSPECIES,
            'LABEL':        simV2.VISIT_VARTYPE_LABEL,
            'ARRAY':        simV2.VISIT_VARTYPE_ARRAY,
            'MESH':         simV2.VISIT_VARTYPE_MESH,
            'CURVE':        simV2.VISIT_VARTYPE_CURVE
            }
        self.CENTERINGS = {
            'NODE':         simV2.VISIT_VARCENTERING_NODE,
            'ZONE':         simV2.VISIT_VARCENTERING_ZONE
            }
        self.COMMANDS = {
            # I don't know what a custom command is, unfortunately...
            #'CUSTOM':       simV2.VisIt_SimulationMetaData_addCustomCommand,
            'GENERIC':      simV2.VisIt_SimulationMetaData_addGenericCommand
            }
        self.VARDATATYPES = {
            'CHAR':         simV2.VisIt_VariableData_setDataC,
            'INTEGER':      simV2.VisIt_VariableData_setDataI,
            # In Python, floats are doubles.
            'FLOAT':        simV2.VisIt_VariableData_setDataD
            }
        self.DATAOWNERS = {
            'SIM':          simV2.VISIT_OWNER_SIM,
            'VISIT':        simV2.VISIT_OWNER_VISIT,
            'COPY':         simV2.VISIT_OWNER_COPY
            }
        
        return
    
    #*********************************************************
    # UTILITY METHODS
    
    def truth_to_0_1(self,truth):
        """Returns 1 if truth is true, 0 if false."""
        if truth: return 1
        return 0
    
    def print_to_console(self,message):
        """Prints to the console and reissues the console prompt."""
        print()
        print(message)
        self.main_doPrompt()
        return
    
    #*********************************************************
    # SETTERS

    def set_runMode(self,running):
        """Sets self.runMode to the appropriate VISIT_ constant."""
        if running:
            self.runMode = simV2.VISIT_SIMMODE_RUNNING
        else:
            self.runMode = simV2.VISIT_SIMMODE_STOPPED
        return
    
    #*********************************************************
    
    def Initialize(self,sim_name,sim_description,**kwargs):
        """Call this first.
    
        By default the simulation comes with the following console
        and callback commands:
    
            * quit (console only)
            * halt
            * step
            * run
            
        Keyword arguments are passed to initialize()
        """
        self.done = False
        self.trace_qualifier = os.getpid()
        self.sim_name = sim_name
        self.sim_description = sim_description
        self.sim_path = os.getcwd()
        self.command_prompt = "command>"
        self.console_timeout = 100000   # Seconds? Milliseconds? Don't know.
        self.runMode = simV2.VISIT_SIMMODE_RUNNING
        self.console = {
                "quit":         (self.cmd_quit, None),
                "halt":         (self.cmd_halt, None),
                "step":         (self.cmd_step, None),
                "run":          (self.cmd_run, None)
            }
        self.callbacks = {
                "commands":     {
                    "halt":         (self.cmd_halt, None, 'GENERIC'),
                    "step":         (self.cmd_step, None, 'GENERIC'),
                    "run":          (self.cmd_run,  None, 'GENERIC')
                },
            }
        self.initialize(**kwargs)
        if self.trace_qualifier is not None: simV2.VisItOpenTraceFile("trace.%d.txt" % self.trace_qualifier)
        simV2.VisItSetDirectory(self.visit_base)
        if not simV2.VisItSetupEnvironment():
            print("VisItSetupEnvironment: could not get environment\n\nVISIT_HOME is '%s', is this correct?" % \
                (self.visit_base,), file=sys.stderr)
            sys.exit(1)
        # We don't have to worry about the rank, because we only ever run a single compute kernel.
        if not simV2.VisItInitializeSocketAndDumpSimFile(
                        self.sim_name, self.sim_description, self.sim_path,
                        None, None, None   ):
            print('VisItInitializeSocketAndDumpSimFile: failed for some reason', file=sys.stderr)
            sys.exit(1)
        return
   
    def initialize(self,**kwargs):
        """This is the subclass-overridable companion to Initialize().
        The keyword arguments are what were supplied in the call to
        Initialize().
        
        Specific values which you might want to override:
            trace_qualifier:    Used while naming the trace file. Defaults to
                                the process PID. If set to None no trace file
                                is opened.
            sim_path:           It is assumed that the simulation was started
                                from the current working directory, i.e.
                                os.getcwd()
            console:            Define the console commands.
            callbacks:          Define the callbacks and specific values to be
                                exposed. Otherwise you are going to need to
                                override main_doVisItConnect and do a heap o'
                                work.
        
        callbacks in turn has substructures:
            commands:           Defines commands which can be triggered from the
                                VisIt sim control panel.
            metadata:           Defines/names metadata for which callbacks should
                                be made. The following types of metadata are
                                supported: mesh, variable, curve expression.
                                See the source for callback_metadata() method
                                for enumeration of the properties which can be
                                set for specific types of metadata.
        """
        return
    
    def MainLoop(self):
        """This is the main loop. It does not have a subclass-overridable companion,
        however it has a number of main_ methods which are intended to be overridden.
        
        It runs until self.done is true.
        """
        self.main_doPrompt()
        while not self.done:
            visitstate = simV2.VisItDetectInputWithTimeout(
                            self.truth_to_0_1(self.runMode == simV2.VISIT_SIMMODE_STOPPED),
                            self.console_timeout,
                            sys.stdin.fileno()
                )
            if visitstate == 0:
                self.cmd_step(None,None,None,None)
            elif visitstate == 1:
                if simV2.VisItAttemptToCompleteConnection() == simV2.VISIT_OKAY:
                    self.runMode = simV2.VISIT_SIMMODE_STOPPED
                    self.main_doVisItConnect()
            elif visitstate == 2:
                if simV2.VisItProcessEngineCommand() != simV2.VISIT_OKAY:
                    simV2.VisItDisconnect()
                    self.runMode = simV2.VISIT_SIMMODE_RUNNING
            elif visitstate == 3:
                cmd = simV2.VisItReadConsole()
                self.main_doConsoleCommand(cmd)
                self.main_doPrompt()
            else:
                self.main_visitstateError(visitstate)
        return

    def main_doPrompt(self):
        """Displays a command prompt on stdout."""
        # Don't have to worry about the rank, because we only ever run a single kernel.
        sys.stdout.write(self.command_prompt)
        sys.stdout.flush()
        return
    
    def main_doRun(self,running):
        """Your compute kernel will want to override this, this is where you will
        do work!
        
        Parameters:
            running:    This will be True if VisIt believes the simulation is
                        running.
        """
        return
    
    def main_doVisItConnect(self):
        """Your compute kernel may want to override this. This is where you will
        connect callbacks. There are helper methods to assist with this. The
        default automagically invokes some default callbacks based on some
        data definitions, and presumably if you're not doing anything too fancy
        the callbacks themselves can be defined with some... errrm... definitions.
        """
        simV2.VisItSetCommandCallback(self.callback_command_, 0)
        simV2.VisItSetGetMetaData(self.callback_metadata_, None)
        simV2.VisItSetGetMesh(self.callback_mesh_, 0)
        simV2.VisItSetGetVariable(self.callback_variable_, 0)
        simV2.VisItSetGetCurve(self.callback_curve_, 0)
        return
    
    def main_doConsoleCommand(self, cmd):
        """Processes console commands.
        
        Parameters:
            cmd: The command read from the console.
        """
        if cmd in self.console:
            self.console[cmd][0](self.console[cmd][1], None, None, None)
        return
    
    def main_visitstateError(self, visitstate):
        """Called when the main loop visitstate contains an unexpected value."""
        self.print_to_console("visitstate error: %s" % (visitstate,))
        return
    
    def cmd_run(self, arg, cmd, visit_args, cbdata):
        """Sets the runMode to VISIT_SIMMODE_RUNNING"""
        self.runMode = simV2.VISIT_SIMMODE_RUNNING
        return
    
    def cmd_halt(self, arg, cmd, visit_args, cbdata):
        """Sets the runMode to VISIT_SIMMODE_STOPPED"""
        self.runMode = simV2.VISIT_SIMMODE_STOPPED
        return
    
    def cmd_step(self, arg, cmd, visit_args, cbdata):
        """Runs one epoch."""
        self.main_doRun(self.runMode == simV2.VISIT_SIMMODE_RUNNING)
        simV2.VisItTimeStepChanged()
        simV2.VisItUpdatePlots()
        return
    
    def cmd_quit(self, arg, cmd, visit_args, cbdata):
        """Sets the done flag which causes the main loop to exit."""
        self.done = True
        return
    
    def visit_execute(self, commands, cmd, visit_args, cbdata):
        """Executes the commands"""
        for command in commands:
            simV2.VisItExecuteCommand(command + '\n')
        return

    def callback_command_(self, cmd, visit_args, cbdata):
        """A wrapper around callback_command, making it easier to
        override in a subclass.
        
        This is the method which is actually registered as a
        callback.
        """
        self.callback_command(cmd, visit_args, cbdata)
        return
    
    def callback_command(self, cmd, visit_args, cbdata):
        """Overridable method."""
        commands = self.callbacks['commands']
        if cmd in commands:
            commands[cmd][0](commands[cmd][1], cmd, visit_args, cbdata)
        return
    
    def callback_metadata_(self, cbdata):
        """This is the method which is actually registered as a callback."""
        return self.callback_metadata(cbdata)
    
    def callback_metadata(self, cbdata):
        """Overridable method. (You don't have to.)
        
        This method will declare your metadata, if you pass it an
        appropriate structure.
        """
        metadata = self.callbacks['metadata']
        md = simV2.VisIt_SimulationMetaData_alloc()
        if md == simV2.VISIT_INVALID_HANDLE: return md
        mode,cycle,elapsed = self.callback_modecycletime()
        if mode is not None:
            if mode:
                current_mode = VISIT_SIMMODE_RUNNING
            else:
                current_mode = VISIT_SIMMODE_STOPPED
            simV2.VisIt_SimulationMetaData_setMode(md,current_mode)
        else:
            simV2.VisIt_SimulationMetaData_setMode(md,self.runMode)
        if (cycle is not None) and (elapsed is not None):
            simV2.VisIt_SimulationMetaData_setCycleTime(md,cycle,elapsed)
        if 'mesh' in metadata:
            for mesh in metadata['mesh']:
                mmd = simV2.VisIt_MeshMetaData_alloc()
                if mmd != simV2.VISIT_INVALID_HANDLE:
                    if 'Name' in mesh:
                        simV2.VisIt_MeshMetaData_setName(mmd, mesh['Name'])
                    if 'MeshType' in mesh:
                        simV2.VisIt_MeshMetaData_setMeshType(mmd, self.MESHTYPES[mesh['MeshType']])
                    if 'TopologicalDimension' in mesh:
                        simV2.VisIt_MeshMetaData_setTopologicalDimension(mmd, mesh['TopologicalDimension'])
                    if 'SpatialDimension' in mesh:
                        simV2.VisIt_MeshMetaData_setSpatialDimension(mmd, mesh['SpatialDimension'])
                    if 'NumDomains' in mesh:
                        simV2.VisIt_MeshMetaData_setNumDomains(mmd, mesh['NumDomains'])
                    if 'DomainTitle' in mesh:
                        simV2.VisIt_MeshMetaData_setDomainTitle(mmd, mesh['DomainTitle'])
                    if 'DomainPieceName' in mesh:
                        simV2.VisIt_MeshMetaData_setDomainPieceName(mmd, mesh['DomainPieceName'])
                    if 'NumGroups' in mesh:
                        simV2.VisIt_MeshMetaData_setNumGroups(mmd, mesh['NumGroups'])
                    if 'XUnits' in mesh:
                        simV2.VisIt_MeshMetaData_setXUnits(mmd, mesh['XUnits'])
                    if 'YUnits' in mesh:
                        simV2.VisIt_MeshMetaData_setYUnits(mmd, mesh['YUnits'])
                    if 'ZUnits' in mesh:
                        simV2.VisIt_MeshMetaData_setZUnits(mmd, mesh['ZUnits'])
                    if 'XLabel' in mesh:
                        simV2.VisIt_MeshMetaData_setXLabel(mmd, mesh['XLabel'])
                    if 'YLabel' in mesh:
                        simV2.VisIt_MeshMetaData_setYLabel(mmd, mesh['YLabel'])
                    if 'ZLabel' in mesh:
                        simV2.VisIt_MeshMetaData_setZLabel(mmd, mesh['ZLabel'])
                    simV2.VisIt_SimulationMetaData_addMesh(md, mmd)
        if 'variable' in metadata:
            for variable in metadata['variable']:
                vmd = simV2.VisIt_VariableMetaData_alloc()
                if vmd != simV2.VISIT_INVALID_HANDLE:
                    if 'Name' in variable:
                        simV2.VisIt_VariableMetaData_setName(vmd, variable['Name'])
                    if 'MeshName' in variable:
                        simV2.VisIt_VariableMetaData_setMeshName(vmd, variable['MeshName'])
                    if 'Type' in variable:
                        simV2.VisIt_VariableMetaData_setType(vmd, self.VARTYPES[variable['Type']])
                    if 'Centering' in variable:
                        simV2.VisIt_VariableMetaData_setCentering(vmd, self.CENTERINGS[variable['Centering']])
                    simV2.VisIt_SimulationMetaData_addVariable(md, vmd)
        if 'curve' in metadata:
            for curve in metadata['curve']:
                cmd = simV2.VisIt_CurveMetaData_alloc()
                if cmd != simV2.VISIT_INVALID_HANDLE:
                    if 'Name' in curve:
                        simV2.VisIt_CurveMetaData_setName(cmd, curve['Name'])
                    if 'XLabel' in curve:
                        simV2.VisIt_CurveMetaData_setXLabel(cmd, curve['XLabel'])
                    if 'XUnits' in curve:
                        simV2.VisIt_CurveMetaData_setXUnits(cmd, curve['XUnits'])
                    if 'YLabel' in curve:
                        simV2.VisIt_CurveMetaData_setYLabel(cmd, curve['YLabel'])
                    if 'YUnits' in curve:
                        simV2.VisIt_CurveMetaData_setYUnits(cmd, curve['YUnits'])
                    simV2.VisIt_SimulationMetaData_addCurve(md, cmd)
        if 'expression' in metadata:
            for expression in metadata['expression']:
                emd = simV2.VisIt_ExpressionMetaData_alloc()
                if emd != simV2.VISIT_INVALID_HANDLE:
                    if 'Name' in expression:
                        simV2.VisIt_ExpressionMetaData_setName(emd, expression['Name'])
                    if 'Definition' in expression:
                        simV2.VisIt_ExpressionMetaData_setDefinition(emd, expression['Definition'])
                    if 'Type' in expression:
                        simV2.VisIt_ExpressionMetaData_setType(emd, self.VARTYPES[expression['Type']])
                    simV2.VisIt_SimulationMetaData_addExpression(md, emd)
        if 'commands' in self.callbacks:
            for command in self.callbacks['commands']:
                cmd = simV2.VisIt_CommandMetaData_alloc()
                if cmd != simV2.VISIT_INVALID_HANDLE:
                    simV2.VisIt_CommandMetaData_setName(cmd, command)
                    self.COMMANDS[self.callbacks['commands'][command][2]](md, cmd)
                
        return md
    
    def callback_modecycletime(self):
        """Returns a triple of:
        
            mode:       True if running, false if not. If you just want this to
                        follow self.simMode, return None.
            cycle:      The epoch.
            time:       The "elapsed time"... presumably in seconds, don't really know.
        """
        return None,None,None

    def callback_mesh_(self, domain, name, cbdata):
        """A wrapper around callback_mesh, making it easier to
        override in a subclass.
        
        This is the method which is actually registered as a
        callback.
        """
        h = self.callback_mesh(domain, name, cbdata)
        if h is None: h = simV2.VISIT_INVALID_HANDLE
        return h
    
    def callback_mesh(self, domain, name, cbdata):
        """Overridable method."""
        return simV2.VISIT_INVALID_HANDLE
        
    def callback_variable_(self, domain, name, cbdata):
        """A wrapper around callback_variable, making it easier to
        override in a subclass.
        
        This is the method which is actually registered as a
        callback.
        """
        h = self.callback_variable(domain, name, cbdata)
        if h is None: h = simV2.VISIT_INVALID_HANDLE
        return h
    
    def callback_variable(self, domain, name, cbdata):
        """Overridable method."""
        return simV2.VISIT_INVALID_HANDLE

    def callback_curve_(self, name, cbdata):
        """A wrapper around callback_curve, making it easier to
        override in a subclass.
        
        This is the method which is actually registered as a
        callback.
        """
        h = self.callback_curve(name, cbdata)
        if h is None: h = simV2.VISIT_INVALID_HANDLE
        return h
    
    def callback_curve(self, name, cbdata):
        """Overridable method."""
        return simV2.VISIT_INVALID_HANDLE

    # TODO: Change the owner to 'SIM' and see what happens.
    def visit_rectilinear_mesh(self, data_func, min_real_idx, max_real_idx, data_x, data_y, data_z=None, owner='VISIT'):
        """Creates a rectilinear mesh.
        
        The handling for owner other that VISIT_OWNER_VISIT is complete and
        utterly untested guesswork.
        
        data_func is one of VARDATATYPES. NOTE: In Python, floats are doubles.
        """
        h = simV2.VisIt_RectilinearMesh_alloc()
        if h == simV2.VISIT_INVALID_HANDLE: return h

        hx = simV2.VisIt_VariableData_alloc()
        data_func(hx,self.DATAOWNERS[owner],1,len(data_x),data_x)
        hy = simV2.VisIt_VariableData_alloc()
        data_func(hy,self.DATAOWNERS[owner],1,len(data_y),data_y)

        if data_z is None:
            simV2.VisIt_RectilinearMesh_setCoordsXY(h,hx,hy)
        else:
            hz = simV2.VisIt_VariableData_alloc()
            data_func(hz,self.DATAOWNERS[owner],1,len(data_z),data_z)
            simV2.VisIt_RectilinearMesh_setCoordsXYZ(h,hx,hy,hz)

        simV2.VisIt_RectilinearMesh_setRealIndices(h,min_real_idx,max_real_idx)

        return h
    
    # TODO: Change the owner to 'SIM' and see what happens.
    def visit_point_mesh(self, data_func, data_x, data_y, data_z=None, owner='VISIT'):
        """Creates a point mesh.
        
        The handling for owner other that VISIT_OWNER_VISIT is complete and
        utterly untested guesswork.
        
        data_func is one of VARDATATYPES. NOTE: In Python, floats are doubles.
        """
        h = simV2.VisIt_PointMesh_alloc()
        if h == simV2.VISIT_INVALID_HANDLE: return h

        hx = simV2.VisIt_VariableData_alloc()
        data_func(hx,self.DATAOWNERS[owner],1,len(data_x),data_x)
        hy = simV2.VisIt_VariableData_alloc()
        data_func(hy,self.DATAOWNERS[owner],1,len(data_y),data_y)

        if data_z is None:
            simV2.VisIt_PointMesh_setCoordsXY(h,hx,hy)
        else:
            hz = simV2.VisIt_VariableData_alloc()
            data_func(hz,self.DATAOWNERS[owner],1,len(data_z),data_z)
            simV2.VisIt_PointMesh_setCoordsXYZ(h,hx,hy,hz)

        return h
        
    # TODO: Change the owner to 'SIM' and see what happens.
    def visit_variable(self, data_func, data, owner='VISIT', nComp=1):
        """Creates a variable.
        
        nComp determines the "number of components". For a precise definition
        see the documentation. For Floats and Ints this should typically be
        1 (no stride). For character strings (such as labels), this is the
        length of the label; it is required that each string be the same length.
        """
        h = simV2.VisIt_VariableData_alloc()
        data_func(h,self.DATAOWNERS[owner],nComp,len(data),data)
        return h
    
    # TODO: Change the owner to 'SIM' and see what happens.
    def visit_curve(self, data_func, data_x, data_y, data_z=None, owner='VISIT'):
        """Creates a curve."""
        h = simV2.VisIt_CurveData_alloc()
        if h == simV2.VISIT_INVALID_HANDLE: return h
        
        hx = simV2.VisIt_VariableData_alloc()
        data_func(hx,self.DATAOWNERS[owner],1,len(data_x),data_x)
        hy = simV2.VisIt_VariableData_alloc()
        data_func(hy,self.DATAOWNERS[owner],1,len(data_y),data_y)
        
        if data_z is None:
            simV2.VisIt_CurveData_setCoordsXY(h,hx,hy)
        else:
            hz = simV2.VisIt_VariableData_alloc()
            data_func(hz,self.DATAOWNERS[owner],1,len(data_z),data_z)
            simV2.VisIt_CurveData_setCoordsXYZ(h,hx,hy,hz)
                
        return h
    
    def Finalize(self,**kwargs):
        """Most inportant thing that this does is to call VisItCloseTraceFile
        if trace_qualifier is not set to None.
        """
        self.finalize(**kwargs)
        if self.trace_qualifier is not None: simV2.VisItCloseTraceFile()
        return

    def finalize(self,**kwargs):
        """This is the subclass-overridable companion to Finalize."""
        return
    
