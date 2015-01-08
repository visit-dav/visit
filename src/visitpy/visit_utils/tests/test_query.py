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
 file: test_query.py
 author: Cyrus Harrison (cyrush@llnl.gov)
 created: 4/09/2010
 description:
    Unit tests for query helpers.

"""

import unittest
import os
import sys

from os.path import join as pjoin

from visit_test import *
from visit_utils import query, python_query

try:
    import visit
except:
    pass


db_dir = pjoin(os.path.split(__file__)[0],"_data")
db = pjoin(db_dir,"multi_rect2d.silo")
vpq_file = pjoin(db_dir,"visit_min.vpq")


class TestQuery(unittest.TestCase):
    @visit_test
    def setUp(self):
        visit.OpenDatabase(db)
        visit.AddPlot("Pseudocolor","d")
        visit.DrawPlots()
    @visit_test
    def test_query_01_std(self):
        res_v = query("Min")
        res_s = query("Min",rmode="string").strip()
        self.assertTrue( (res_v - 0.02357020415365696) < 1e-5 )
        self.assertEqual(res_s,"d -- Min = 0.0235702 (zone 44 in domain 5 at coord <0.483333, 0.483333>)")
    @visit_test
    def test_query_02_kwargs(self):
        #add a threshold 
        visit.AddOperator("Isovolume")
        iatts = visit.IsovolumeAttributes()
        iatts.lbound = 0.4
        iatts.ubound = 1e+37
        visit.SetOperatorOptions(iatts)
        visit.DrawPlots()
        res_v = query("Min",use_actual_data=True)
        self.assertTrue( (res_v - 0.3778594434261322) < 1e-5 )
        res_v = query("Min",use_actual_data=False)
        self.assertTrue( (res_v - 0.02357020415365696) < 1e-5 )
    @visit_test
    def test_query_03_py_query(self):
        res_a = python_query(file=vpq_file,msg_lvl=4)
        res_b = python_query(source=open(vpq_file).read(),msg_lvl=4)
        res_c = python_query(file=vpq_file,rmode="string",msg_lvl=4).strip()
        print res_a, res_b, res_c
        self.assertTrue( (res_a - 0.02357020415365696) < 1e-5 )
        self.assertTrue( (res_b - 0.02357020415365696) < 1e-5 )
        self.assertEqual(res_c,"Min = %s" % str(res_a))
    @visit_test
    def tearDown(self):
        visit.DeleteAllPlots()
        visit.CloseDatabase(db)
        visit.CloseComputeEngine()


if __name__ == "__main__":
    unittest.main()

