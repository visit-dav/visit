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
 file: test_filters_cmd.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 3/24/2012
 description:
    unittest test cases for Filters in the cmd module.

"""

import unittest
import os
from os.path import join as pjoin

from flow import *
from flow.filters import cmd, file_ops

# uncomment for detailed exe info
#import logging
#logging.basicConfig(level=logging.INFO)

class TestCmd(unittest.TestCase):
    def setUp(self):
        odir = pjoin("tests","_test_output")
        if not os.path.isdir(odir):
            os.mkdir(odir)
        print ""
    def test_01_workspace_setup(self):
        odir = pjoin("tests","_test_output")
        w = Workspace()
        w.register_filters(file_ops)
        w.register_filters(cmd)
        for i in range(10):
            f = open(pjoin(odir,"cmd.test.input.%04d.txt" % i),"w")
            f.write("%0d\n" % i)
            f.close()
        fi = w.add_filter("file_name","finput")
        fi["pattern"] = pjoin(odir,"cmd.test.input.%04d.txt")
        mv = w.add_filter("cmd","mv")
        mv["cmd"]   = "mv "
        mv["obase"] = pjoin(odir,"cmd_mv")
        w.connect("finput","mv:in")
        print w.graph
        sspace = StateSpace({"idx":10})
        sgen = StateVectorGenerator(sspace)
        w.execute(sgen)

if __name__ == '__main__':
    unittest.main()

