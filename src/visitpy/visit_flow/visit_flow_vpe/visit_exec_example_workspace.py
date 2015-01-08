#*****************************************************************************
#
# Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
# Produced at the Lawrence Livermore National Laboratory
# LLNL-CODE-442911
# All rights reserved.
#
# This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
# full copyright notice is contained in the file COPYRIGHT located at the root
# of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
#
# Redistribution  and  use  in  source  and  binary  forms,  with  or  without
# modification, are permitted provided that the following conditions are met:
#
#  - Redistributions of  source code must  retain the above  copyright notice,
#    this list of conditions and the disclaimer below.
#  - Redistributions in binary form must reproduce the above copyright notice,
#    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
#    documentation and/or other materials provided with the distribution.
#  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
#    be used to endorse or promote products derived from this software without
#    specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
# ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
# LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
# DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
# SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
# CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
# LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
# OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
# DAMAGE.
#*****************************************************************************
"""
 file: visit_exec_flow_workspace.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 1/04/2012
 description:
   Driver script that executes a flow workspace on 'rect2d.silo'.

"""

import os
import visit
from os.path import join as pjoin
from visit_flow_vpe import *

def save_window():
    visit.ResetView()
    v = visit.GetView3D()
    v.RotateAxis(0,-90)
    visit.SetView3D(v)
    swatts= visit.SaveWindowAttributes()
    swatts.outputToCurrentDirectory = 1
    swatts.outputDirectory = "."
    swatts.fileName = "chunk_render"
    swatts.family = 0
    swatts.format = swatts.PNG
    swatts.width = 1024
    swatts.height = 1024
    swatts.screenCapture = 0
    swatts.saveTiled = 0
    swatts.quality = 100
    swatts.progressive = 0
    swatts.binary = 0
    swatts.stereo = 0
    swatts.compression = swatts.PackBits
    swatts.forceMerge = 0
    swatts.resConstraint = swatts.NoConstraint
    swatts.advancedMultiWindowSave = 0
    visit.SetSaveWindowAttributes(swatts)
    visit.SaveWindow()

def main():
    args   = Argv()
    wfile  = args[0]
    dbfile = pjoin("tests","_data","rect2d.silo")
    fset = "pyocl_ops"
    plat = 0
    dev  = 0
    if len(args) > 1:
        dbfile = args[1]
    if len(args) > 2:
        fset = args[2]
    if len(args) > 3:
        plat  = int(args[3])
    if len(args) > 4:
        dev   = int(args[4])
    visit.OpenDatabase(dbfile)
    define_flow_vpe("flow",
                    expr=wfile,
                    filter_set=fset,
                    platform_id=plat,
                    device_id=dev)
    visit.AddPlot("Pseudocolor","flow")
    visit.DrawPlots()
    #visit.ClearAllWindows()
    #visit.DrawPlots()

if __visit_script_file__ == __visit_source_file__:
    main()
    if "-save" in Argv():
        save_window()
    if "-nowin" in sys.argv:
        sys.exit(0)


