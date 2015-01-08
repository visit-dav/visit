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
 file: imagick.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 10/14/2010
 description:
    Provides flow filters that wrap image magick functionaliy.

    Requires an install of the imagemagick command line tools.
"""

import os
from os.path import join as pjoin
from ..core import Filter, Context, PropertyTree, sexe


class ImagickExecuteError(Exception):
    def __init__(self,output):
        self.output = output
    def __str__(self):
        return self.output

class ImagickContext(Context):
    context_type = "imagick"
    def set_working_dir(self,dname,create=True):
        self.wdir = os.path.abspath(dname)
        if create and not os.path.isdir(self.wdir):
            os.mkdir(wdir)
    def working_dir(self):
        return self.wdir

def gen_output_file_name(node):
    obase = node.name
    if node.params.has_property("obase"):
        obase = node.params.obase
    # note, we can check for abs path here ...
    # provide an option besides usign the working_dir
    ofname = pjoin(node.context.working_dir(), obase)
    return ofname + "%s_%s_output.png" % (obase,node.state_vector.idx)

def imagick_exe(cmd,ofname):
    ret, out = sexe(cmd + " " + ofname,ret_output=True)
    if ret != 0:
        raise ImagickExecuteError(out)
    return ofname

class ImageGenerator(Filter):
    filter_type    = "gen"
    input_ports    = []
    default_params = {"width":0,
                      "height":0,
                      "color":"black"}
    output_port    = True
    def execute(self):
        p      = self.params
        cmd    = "convert -size %dx%d xc:%s " % (p.width,p.height,p.color)
        return imagick_exe(cmd,gen_output_file_name(self))

class ImageResize(Filter):
    filter_type    = "resize"
    input_ports    = ["in"]
    default_params = { "width":0,
                       "height":0}
    output_port    = True
    def execute(self):
        p      = self.params
        cmd    = "convert -resize %dx%d %s " % (p.width,p.height,self.input("in"))
        return imagick_exe(cmd,gen_output_file_name(self))


class ImageOver(Filter):
    filter_type    = "over"
    input_ports    = ["over","under"]
    default_params = {"x_offset":0,
                      "y_offset":0}
    output_port    = True
    def execute(self):
        p   = self.params
        cmd = "composite -geometry +%d+%d %s %s " % (p.x_offset,p.y_offset,
                                                     self.input("over"),
                                                     self.input("under"))
        return imagick_exe(cmd,gen_output_file_name(self))

class ImageBlend(Filter):
    filter_type    = "blend"
    input_port     = ["over","under"]
    default_params = {"x_offset":0,
                      "y_offset":0,
                      "percent":0}
    output_port    = True
    def execute(self):
        p   = self.params
        cmd = "composite -blend %f -geometry +%d+%d %s %s " % (p.percent,
                                                               p.x_offset,
                                                               p.y_offset,
                                                               self.input("over"),
                                                               self.input("under"))
        return imagick_exe(cmd,gen_output_file_name(self))

class ImageCrop(Filter):
    filter_type    = "crop"
    input_ports    = ["in"]
    default_params = {"x_offset":0,
                      "y_offset":0,
                      "width":0,
                      "height":0}
    output_port    = True
    def execute(self):
        p = self.params
        cmd = "convert -crop %dx%d+%d+%d %s "  % (p.width,
                                                  p.height,
                                                  p.x_offset,
                                                  p.y_offset,
                                                  self.input("in"))
        return imagick_exe(cmd,gen_output_file_name(self))

class ImageAppend(Filter):
    filter_type    = "append"
    input_ports    = ["in_a","in_b"]
    default_params = {"direction":"hz"}
    output_port    = True
    def execute(self):
        p = self.params
        d = p.direction
        op = "+"
        if d == "vz":
            op = "-"
        cmd = "convert %s %s %sappend " % (self.input("in_a"),self.input("in_b"),op)
        return imagick_exe(cmd,gen_output_file_name(self))


filters = [ImageGenerator, ImageResize, ImageOver, ImageBlend, ImageCrop, ImageAppend]
contexts = [ImagickContext]