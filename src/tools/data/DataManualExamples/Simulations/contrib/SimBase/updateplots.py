#!/usr/bin/python
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
import sys,math,os

# Where to find simV2.py
VISIT_LIBPATH = os.environ.get('VISIT_LIBPATH',"../../../../lib")
# The directory above the bin/ directory.
VISIT_HOME = os.environ.get('VISIT_HOME',"../../../..")

from simbase import Simulation

class PlotSim(Simulation):

    def initialize(self, dims, ndims):
        # By default the sim starts running at startup.
        # self.set_runMode(False)
        
        self.rmesh_dims = dims
        self.rmesh_ndims = ndims
        self.cycle = 0
        self.time = 0.
        self.angle = 0.
        self.NPTS = 1000
        # X, Y and Z stored as three vectors NPTS long, rather than as
        # 3-tuples NPTS long.
        self.points = [ [0.0]*self.NPTS, [0.0]*self.NPTS, [0.0]*self.NPTS ]
        # Labels have to be fixed-length strings. Be sure to pad with
        # enough extra space/nulls. Nulls seems to be the preferred idiom.
        # Our labels here are of the form 'P-000'..'P-999'.
        self.labels = [ "P-%03d\0" % (i,) for i in range(self.NPTS) ]
 
        self.tick = True
        addplot = (
                'AddPlot("Pseudocolor", "zonal")',
                'DrawPlots()'
            )
        self.console['addplot'] = (self.visit_execute, addplot)
        self.console['tick on'] = (self.tick_on, None)
        self.console['tick off'] = (self.tick_off, None)
        self.callbacks['commands']['addplot'] = (self.visit_execute, addplot, 'GENERIC')
        
        self.callbacks['metadata'] = {
            'mesh':
                [
                    {
                        'Name':                 "mesh2d",
                        'MeshType':             "RECTILINEAR",
                        'TopologicalDimension': 2,
                        'SpatialDimension':     2,
                        'NumDomains':           1,
                        'DomainTitle':          "Domains",
                        'DomainPieceName':      "domain",
                        'NumGroups':            0,
                        'XUnits':               "cm",
                        'YUnits':               "cm",
                        'ZUnits':               "cm",
                        'XLabel':               "Width",
                        'YLabel':               "Height",
                        'ZLabel':               "Depth"
                    },
                    {
                        'Name':                 "point3d",
                        'MeshType':             "POINT",
                        'TopologicalDimension': 0,
                        'SpatialDimension':     3,
                        'NumDomains':           1,
                        'DomainTitle':          "Domains",
                        'DomainPieceName':      "domain",
                        'NumGroups':            0,
                        'XUnits':               "cm",
                        'YUnits':               "cm",
                        'ZUnits':               "cm",
                        'XLabel':               "Width",
                        'YLabel':               "Height",
                        'ZLabel':               "Depth"
                        }
                    ],
            'variable':
                [
                    {
                        'Name':                 "zonal",
                        'MeshName':             "mesh2d",
                        'Type':                 "SCALAR",
                        'Centering':            "ZONE"
                    },
                    {
                        'Name':                 "px",
                        'MeshName':             "point3d",
                        'Type':                 "SCALAR",
                        'Centering':            "NODE"
                    },
                    {
                        'Name':                 "py",
                        'MeshName':             "point3d",
                        'Type':                 "SCALAR",
                        'Centering':            "NODE"
                    },
                    {
                        'Name':                 "pz",
                        'MeshName':             "point3d",
                        'Type':                 "SCALAR",
                        'Centering':            "NODE"
                    },
                    {
                        'Name':                 "pointLabels",
                        'MeshName':             "point3d",
                        'Type':                 "LABEL",
                        'Centering':            "NODE"
                    }
                ],
            'curve':
                [
                    {
                        'Name':                 "sine",
                        'XLabel':               "Angle",
                        'XUnits':               "radians",
                        'YLabel':               "Amplitude",
                        'YUnits':               ""
                    }
                ],
            'expression':
                [
                    {
                        'Name':                 "zvec",
                        'Definition':           "{zonal, zonal}",
                        'Type':                 "VECTOR"
                    }
                ]
        }

        return
    
    def tick_on(self, arg, cmd, visit_args, cbdata):
        self.tick = True
        return
    
    def tick_off(self, arg, cmd, visit_args, cbdata):
        self.tick = False
        return
    
    def progress(self,running):
        if not self.tick: return
        if running:
            dot = '+'
        else:
            dot = '.'
            sys.stdout.write(dot)
            sys.stdout.flush()
        return
      
    def main_doRun(self,running):
        self.progress(running)
        self.cycle += 1
        self.time  += math.pi/10.
        self.angle += 0.05
        points = self.points
        # This might be an actual simulation. Your actual simulation would
        # go here.
        for i in range(self.NPTS - 1):
            t = float(i) / float(self.NPTS - 1)
            a = math.pi * 10.0 * t
            points[0][i] = t * math.cos(a + 0.5 * (t + 1.0)) * self.angle
            points[1][i] = t * math.sin(a + 0.5 * (t + 1.0)) * self.angle
            points[2][i] = t
        return
    
    def callback_modecycletime(self):
        return None, self.cycle, self.time
    
    def callback_mesh(self, domain, name, cbdata):
        h = None
        if name == "mesh2d":
            # If the data being displayed was derived from the "actual"
            # simulation data, but not the simulation data itself, then
            # the mesh might be built when requested.
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
                
            h = self.visit_rectilinear_mesh(self.VARDATATYPES['FLOAT'],
                                            minRealIndex, maxRealIndex,
                                            rmesh_x, rmesh_y
                                           )
        if name == 'point3d':
            # If this was "actual" simulation data, the calculations might
            # be done in the main loop.
            #
            # NOTE: "*self.points" causes Python to perform one level of
            # expansion, into x, y and z. This is the same syntax used
            # for expanding undeclared positional args.
            h = self.visit_point_mesh(self.VARDATATYPES['FLOAT'],
                                      *self.points
                                     )
        return h

    def callback_variable(self, domain, name, cbdata):
        h = None
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

            h = self.visit_variable(self.VARDATATYPES['FLOAT'], rmesh_zonal)

        if name == 'px':
            h = self.visit_variable(self.VARDATATYPES['FLOAT'], self.points[0])
        if name == 'py':
            h = self.visit_variable(self.VARDATATYPES['FLOAT'], self.points[1])
        if name == 'pz':
            h = self.visit_variable(self.VARDATATYPES['FLOAT'], self.points[2])

        if name == 'pointLabels':
            # As noted earlier, labels are fixed-length strings. The extra parameter
            # passes that length (since all labels will be the same length as the
            # first one).
            h = self.visit_variable(self.VARDATATYPES['CHAR'], self.labels, nComp=len(self.labels[0]))

        return h

    def callback_curve(self, name, cbdata):
        h = None
        if name == "sine":
            x = [0.] * 200
            y = [0.] * 200
        
            for i in range(200):
                angle = self.time + (float(i) / float(200-1)) * 4. * math.pi
                x[i] = angle
                y[i] = math.sin(x[i])
            
            h = self.visit_curve(self.VARDATATYPES['FLOAT'], x, y)

        return h

#
# Main program
#
def main():
    try:
        sim = PlotSim(VISIT_LIBPATH, VISIT_HOME)
    except ImportError as e:
        print("%s\n\nVISIT_LIBPATH is '%s', is this correct?" % (e, VISIT_LIBPATH), file=sys.stderr)
        sys.exit(1)
    # The second pair of arguments are passed to initialize() as kwargs.
    sim.Initialize("updateplots", "LLNL updateplots... reloaded",
                   dims=[50,50,1], ndims=2)
    sim.MainLoop()
    sim.Finalize()

if __name__ == '__main__':
    main()
