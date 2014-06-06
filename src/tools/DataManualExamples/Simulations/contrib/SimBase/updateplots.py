#!/usr/bin/python
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
import sys,math

# Where to find simV2.py
#VISIT_LIB_PATH = "/opt/downloads/visit/build_visit/visit2.7.2/src/lib"
VISIT_LIB_PATH = "../../../../lib"
# The directory above the bin/ directory.
#VISIT_BASE_DIR = "/opt/downloads/visit/build_visit/visit2.7.2/src"
VISIT_BASE_DIR = "../../../.."

from simbase import Simulation

class PlotSim(Simulation):

    def initialize(self):
        self.cycle = 0
        self.time = 0.
        self.angle = 0.
        self.NPTS = 1000
        self.points = [ [0.0]*self.NPTS, [0.0]*self.NPTS, [0.0]*self.NPTS ]
        self.labels = [ "%03d\0" % (i,) for i in range(self.NPTS) ]
        print "len labels: %d" %len(self.labels)
        print "len labels[0]: %d" %len(self.labels[0])

        self.tick = True
        self.console['tick on'] = (self.tick_on, None)
        self.console['tick off'] = (self.tick_off, None)

        self.callbacks['metadata'] = {
            'mesh':
                [
                    {
                        'Name':                 "point2d",
                        'MeshType':             "POINT",
                        'TopologicalDimension': 0,
                        'SpatialDimension':     2,
                        'NumDomains':           1,
                        'DomainTitle':          "Domains",
                        'DomainPieceName':      "domain",
                        'NumGroups':            0,
                        'XUnits':               "cm",
                        'YUnits':               "cm",
                        'XLabel':               "Width",
                        'YLabel':               "Height",
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
                        'Name':                 "z2d",
                        'MeshName':             "point2d",
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
            # 'curve': [],
            # 'expression': []
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
        if name == 'point2d':
            h = self.visit_point_mesh(self.VARDATATYPES['FLOAT'],
                                      self.points[0], self.points[1]
                                     )
        if name == 'point3d':
            h = self.visit_point_mesh(self.VARDATATYPES['FLOAT'],
                                      *self.points
                                     )
        return h

    def callback_variable(self, domain, name, cbdata):
        h = None
        if name == 'px':
            h = self.visit_variable(self.VARDATATYPES['FLOAT'], self.points[0])
        if name == 'py':
            h = self.visit_variable(self.VARDATATYPES['FLOAT'], self.points[1])
        if name == 'pz':
            h = self.visit_variable(self.VARDATATYPES['FLOAT'], self.points[2])
        if name == 'z2d':
            h = self.visit_variable(self.VARDATATYPES['FLOAT'], self.points[2])
        if name == 'pointLabels':
            h = self.visit_variableArray(self.VARDATATYPES['CHAR'], len(self.labels[0]), self.labels)
        return h

#
# Main program
#
def main():
    sim = PlotSim(VISIT_LIB_PATH, VISIT_BASE_DIR)
    # The second pair of arguments are passed to initialize() as kwargs.
    sim.Initialize("pointmesh", "PointMesh POC")
    sim.MainLoop()
    sim.Finalize()

if __name__ == '__main__':
    main()
