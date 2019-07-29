# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

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
    return ofname + "%s_%s_output.png" % (obase,node.state_vector.index())

def imagick_exe(cmd,ofname):
    ret, out = sexe(cmd + " " + ofname,ret_output=True)
    if ret != 0:
        raise ImagickExecuteError(out)
    return ofname

class ImageFill(Filter):
    filter_type    = "fill"
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
    default_params = {"x":0,
                      "y":0}
    output_port    = True
    def execute(self):
        p   = self.params
        cmd = "composite -geometry +%d+%d %s %s " % (p.x,p.y,
                                                     self.input("over"),
                                                     self.input("under"))
        return imagick_exe(cmd,gen_output_file_name(self))

class ImageBlend(Filter):
    filter_type    = "blend"
    input_port     = ["over","under"]
    default_params = {"x":0,
                      "y":0,
                      "percent":0}
    output_port    = True
    def execute(self):
        p   = self.params
        cmd = "composite -blend %f -geometry +%d+%d %s %s " % (p.percent,
                                                               p.x,
                                                               p.y,
                                                               self.input("over"),
                                                               self.input("under"))
        return imagick_exe(cmd,gen_output_file_name(self))

class ImageCrop(Filter):
    filter_type    = "crop"
    input_ports    = ["in"]
    default_params = {"x":0,
                      "y":0,
                      "width":0,
                      "height":0}
    output_port    = True
    def execute(self):
        p = self.params
        cmd = "convert -crop %dx%d+%d+%d %s "  % (p.width,
                                                  p.height,
                                                  p.x,
                                                  p.y,
                                                  self.input("in"))
        return imagick_exe(cmd,gen_output_file_name(self))

class ImageAppend(Filter):
    filter_type    = "append"
    input_ports    = ["in_a","in_b"]
    default_params = {"direction":"horz"}
    output_port    = True
    def execute(self):
        p = self.params
        d = p.direction
        op = "+"
        if d == "vert":
            op = "-"
        cmd = "convert %s %s %sappend " % (self.input("in_a"),self.input("in_b"),op)
        return imagick_exe(cmd,gen_output_file_name(self))


filters = [ImageFill, ImageResize, ImageOver, ImageBlend, ImageCrop, ImageAppend]
contexts = [ImagickContext]