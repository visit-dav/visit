#!/bin/env python
#*****************************************************************************
#
# Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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
file: visit_test_reports.py
description: Beta Test Report Generation.
author: Cyrus Harrison
date: Fri Jan 13 13:45:13 PST 2012
"""
# ----------------------------------------------------------------------------
#  Modifications:
#
# ----------------------------------------------------------------------------

import sys
import string
import os
import shutil
import json

from os.path import join as pjoin
from collections import namedtuple

# ----------------------------------------------------------------------------
#  Method: test_root_dir
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
def test_root_dir():
    return os.path.split(os.path.abspath(__file__))[0]


# ----------------------------------------------------------------------------
#  Class: HtmlTemplateSet
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
class HtmlTemplateSet(object):
    def __init__(self,fname):
        self.sections = {}
        lines = open(fname).readlines()
        self.__parse_sections(lines)
    def use_template(self,key,mapping):
        tplate = string.Template(self.sections[key])
        return tplate.safe_substitute(mapping)
    def __parse_sections(self,lines):
        self.sections = {}
        idx = 0
        while idx < len(lines):
            l = lines[idx]
            if l.startswith("$$"):
                name = l[2:l.find("\n")].strip()
                contents = ""
                idx +=1
                l = lines[idx]
                while l.strip() != "$$":
                    contents = contents + l
                    idx+=1
                    l= lines[idx]
            self.sections[name] = contents
            idx+=1
    def __getitem__(self,key):
        return string.Template(self.sections[key])


# ----------------------------------------------------------------------------
#  Class: HTMLIndex
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
class HTMLIndex(object):
    def __init__(self,obase=None):
        self.obase = obase
        if self.obase is None:
            self.obase = test_root_dir()
        tfile = pjoin(test_root_dir(),"report_templates","index.html")
        self.tset = HtmlTemplateSet(tfile)
    def write_header(self,rmode,rtstamp):
        self.__copy_misc()
        mapping = { "run_mode": rmode, "timestamp": rtstamp}
        res = self.tset.use_template("index_header",mapping)
        f = self.__file(True)
        f.write(res)
        f.close()
    def add_result(self,result):
        lsec = 20
        rcode = result.returncode
        mapping = {"category":result.category,
                   "base":result.base,
                   "name":result.file,
                   "result_code": result.returncode,
                   "details": "%s_%s" % (result.category,result.base),
                   "run_time": result.runtime,
                   "test_idx": str(result.index),
                   "button": "%s_%s_toggle" % (result.category,result.base),
                   }
        if rcode == 111:
            mapping["result_color"] = "good"
            mapping["result_text"]  = "Succeeded"
        elif rcode == 112:
            mapping["result_color"] = "good"
            mapping["result_text"]  = "Acceptable"
        elif rcode  == 113:
            mapping["result_color"] = "bad"
            mapping["result_text"]  = "Unacceptable"
        elif rcode  == 114:
            mapping["result_color"] = "unknown"
            mapping["result_text"]  = "Unknown"
        elif rcode  == 115:
            mapping["result_color"] = "killed"
            mapping["result_text"]  = "Self-Killed after %s" % lsec
        elif rcode  == 116:
            mapping["result_color"] = "skipped"
            mapping["result_text"]  = "Skipped"
            mapping["run_time"]     = "(skipped)"
        elif rcode  == 118:
            mapping["result_color"] = "killed"
            mapping["result_text"]  = "OS-Killed after %s" % lsec
        elif rcode  == 119:
            mapping["result_color"] = "good_with_skips"
            mapping["result_text"]  = "Succeeded With Skips"
        elif rcode  == 120:
            mapping["result_color"] = "good_with_skips"
            mapping["result_text"]  = "Acceptable With Skips"
        elif rcode  == 121:
            mapping["result_color"] = "na"
            mapping["result_text"]  = "Not Applicable"
        else:
            mapping["result_color"] = "failed"
            mapping["result_text"]  = "Failed: exit == %s  unknown" % str(rcode)
        f = self.__file()
        if rcode  != 116:
            json_results = pjoin(self.obase,"json","%s_%s.json" % (result.category,result.base))
            if False and os.path.isfile(json_results):
                # future functionality:
                res = self.tset.use_template("index_entry_start",mapping)
                jr = json.load(open(json_results))
                case_status ={"passed":"good",
                              "failed":"bad",
                              "unknown":"unknown",
                              "skipped":"skipped"}
                for r in jr["results"]:
                    r_map = dict(r)
                    if r["status"] in case_status.keys():
                        r_map["status_color"] = case_status[r["status"]]
                    else:
                        r_map["status_color"] = case_status["unknown"]
                    res += self.tset.use_template("case_entry",r_map)
                res += self.tset.use_template("index_entry_end",mapping)
            else:
                res = self.tset.use_template("index_entry",mapping)
        else:
            res = self.tset.use_template("index_entry_skip",mapping)
        f.write(res)
        f.close()
    def write_footer(self,etstamp,rtime):
        f = self.__file()
        res = res = self.tset.use_template("index_footer",
                                           {"timestamp":etstamp,
                                            "runtime":rtime})
        f.write(res)
        f.close()
    def __file(self,create = False):
        ofname = pjoin(self.obase,"html","index.html")
        if create:
            return open(ofname,"w")
        else:
            return open(ofname,"a")
    def __copy_misc(self):
        des_html = pjoin(self.obase,"html")
        des_css  = pjoin(des_html,"css")
        des_js   = pjoin(des_html,"js")
        tp_base  = pjoin(test_root_dir(),"report_templates")
        for path in [pjoin(self.obase,"html"),des_css,des_js]:
            if not os.path.isdir(path):
                os.mkdir(path)
        shutil.copy(pjoin(tp_base,"css","styles.css"),des_css)
        shutil.copy(pjoin(tp_base,"js","jquery-latest.js"),des_js)
        shutil.copy(pjoin(tp_base,"js","jquery.tablesorter.js"),des_js)

# ----------------------------------------------------------------------------
#  Class: TestScriptResult
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
class TestScriptResult(namedtuple('TestScriptResult',
                                  ['index',
                                   'category',
                                   'base',
                                   'file',
                                   'returncode',
                                   'numcores',
                                   'runtime'])):
    def error(self):
        return not self.returncode in [111,112,116,119,120,121]
    def skip(self):
        return self.returncode == 116
    def message(self):
        codes = { 111:"+ Passed with zero differences in test file:",
                  112:"- Acceptable differences found in test file:",
                  113:"! Unacceptable differences found in test file:",
                  114:"! Unknown if any differences found in test file:",
                  116:"+ Skipped: Test file in skip list:",
                  118:"! Killed after timelimit hit when running test file:",
                  119:"+ Succeeded with partial skips in test file:",
                  120:"- Acceptable with partial skips in test file:",
                  121:"> Not applicable in this mode for test file:",
                   -1:"! ERROR: Missing Exit() when running test file:"}
        rcode = self.returncode
        if not rcode in codes.keys():
            rcode = -1
        return codes[rcode] + " %s/%s" % (self.category,self.file)

# ----------------------------------------------------------------------------
#  Class: JSONIndex
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
class JSONIndex(object):
    def __init__(self,ofile=None,clear=False):
        self.ofile = ofile
        if self.ofile is None:
            self.ofile = pjoin(test_root_dir(),"results.json")
        if clear:
            self.__working_file(create=True)
    def add_result(self,result):
        fo = self.__working_file()
        res = json.dumps(result)
        fo.write(res + "\n")
    def finalize(self):
        fo  = open(self.ofile,"w")
        res = JSONIndex.load_results(self.ofile + ".index")
        fo .write(json.dumps(res))
    @classmethod
    def load_results(self,fname):
        if fname.endswith(".json"):
            return json.loads(open(fname))
        else:
            # load working file
            res = []
            if not os.path.isfile(fname):
                return res
            lines = [l.strip() for l in open(fname).readlines() if l.strip() != ""]
            for l in lines:
                test = TestScriptResult(*json.loads(l))
                res.append(test)
            return res
    def __working_file(self,create = False):
        if create:
            return open(self.ofile + ".index","w")
        else:
            return open(self.ofile + ".index","a")


# ----------------------------------------------------------------------------
#  Method: main
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
def main():
    args = sys.argv
    if args[1] == "index":
        idx = HTMLIndex()
        icmd  = args[2]
        iargs =args[3:]
        if icmd == "open":
            idx.header(iargs)
        elif icmd == "add":
            idx.add_entry(iargs)
        elif icmd == "close":
            idx.footer(iargs)

if __name__ == "__main__":
    main()