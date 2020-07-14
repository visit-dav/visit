#!/bin/env python
# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.
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
import smtplib
import multiprocessing
import glob

from os.path import join as pjoin
from collections import namedtuple

from visit_test_common import *

# ----------------------------------------------------------------------------
#  Method: test_root_dir
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
def test_root_dir():
    return os.path.split(os.path.abspath(__file__))[0]

# ----------------------------------------------------------------------------
#  Class: OutputLock
#
#  Programmer: Cyrus Harrison
#  Date:       Wed Jan 16 15:46:34 PST 2013
# ----------------------------------------------------------------------------
class OutputLock(object):
    lock = multiprocessing.Lock()
    @classmethod
    def acquire(cls):
        cls.lock.acquire()
    @classmethod
    def release(cls):
        cls.lock.release()


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
    def __init__(self,obase):
        self.obase = obase
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
        # in the mp case, this is called by multiple test
        # processes, make sure to lock access to avoid
        # corruption
        lsec = 20
        rcode = result.return_code
        mapping = {"category":result.category,
                   "base":result.base,
                   "name":result.file,
                   "result_code": result.return_code,
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
        elif rcode  == 122:
            mapping["result_color"] = "plot_plugin"
            mapping["result_text"]  = "Plot plugin requirements not met"
        elif rcode  == 123:
            mapping["result_color"] = "operator_plugin"
            mapping["result_text"]  = "Operator plugin requirements not met"
        elif rcode  == 124:
            mapping["result_color"] = "database_plugin"
            mapping["result_text"]  = "Database plugin requirements not met"
        else:
            mapping["result_color"] = "failed"
            mapping["result_text"]  = "Failed: exit == %s  unknown" % str(rcode)
        if rcode  != 116:
            res = self.tset.use_template("index_entry",mapping)
        else:
            res = self.tset.use_template("index_entry_skip",mapping)
        OutputLock.acquire()
        f = self.__file()
        f.write(res)
        f.close()
        OutputLock.release()
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
        css_files = glob.glob(pjoin(tp_base,"css","*.css"))
        js_files  = glob.glob(pjoin(tp_base,"js","*.js"))
        for css_file in css_files:
            shutil.copy(css_file,des_css)
        for js_file  in js_files:
            shutil.copy(js_file,des_js)

# ----------------------------------------------------------------------------
#  Class: TestScriptResult
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
class TestScriptResult(object):
    def __init__(self,index,
                      category,
                      base,
                      file,
                      return_code,
                      num_cores,
                      runtime):
        self.index        = index
        self.category     = category
        self.base         = base
        self.file         = file
        self.return_code  = return_code
        self.num_cores    = num_cores
        self.runtime      = runtime
    def to_dict(self):
        return dict(self.__dict__)
    @classmethod
    def from_dict(cls,vals):
        v = dict(vals)
        if "details" in v:
            del v["details"]
        return TestScriptResult(**v)
    def error(self):
        return not self.return_code in [111,112,116,119,120,121]
    def skip(self):
        return self.return_code == 116
    def status(self):
        codes = { 111:"succeeded",
                  112:"acceptable",
                  113:"unacceptable",
                  114:"unknown",
                  116:"skipped",
                  118:"killed",
                  119:"succeeded with skips",
                  120:"acceptable with skips",
                  121:"na",
                  122:"plot plugin requirement not met",
                  123:"operator plugin requirement not met",
                  124:"database plugin requirement not met",
                   -1:"failed"}
        rcode = self.return_code
        if not rcode in list(codes.keys()):
            rcode = -1
        return codes[rcode]
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
                  122:"! Plot plugin requirement not met",
                  123:"! Operator plugin requirement not met",
                  124:"! Database plugin requirement not met",
                   -1:"! ERROR: Missing Exit() when running test file:"}
        rcode =self.return_code
        if not rcode in list(codes.keys()):
            rcode = -1
        return codes[rcode] + " %s/%s" % (self.category,self.file)

# ----------------------------------------------------------------------------
#  Class: TestCaseResult
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
class TestCaseResult:
    @classmethod
    def from_dict(cls,vals):
        if "diff_pixels" in vals:
            return TestCaseImageResult.from_dict(vals)
        else:
            return TestCaseTextResult.from_dict(vals)

# ----------------------------------------------------------------------------
#  Class: TestCaseImageResult
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
class TestCaseImageResult(object):
    def __init__(self,name,
                      status,
                      diff_state,
                      mode_specific,
                      total_pixels,
                      non_bg_pixels,
                      diff_pixels,
                      avg_pixels):
        self.name         = name
        self.status       = status
        self.diff_state   = mode_specific
        self.total_pixels = total_pixels
        self.no_bg_pixels = non_bg_pixels
        self.diff_pixels  = diff_pixels
        self.avg_pixels   = avg_pixels
    def to_dict(self):
        return dict(self.__dict__)
    @classmethod
    def from_dict(cls,vals):
        return TestCaseImageResult(**vals)
    def error(self):
        return not self.status in ["passed","skipped"]
    def skip(self):
        return self.status == "skipped"
    def message(self):
        res = ""
        for key in ['name',
                    'status',
                    'diff_state',
                    'mode_specific',
                    'total_pixels',
                    'non_bg_pixels',
                    'diff_pixels',
                    'diff_percent',
                    'avg_pixels']:
            res += "%s: %s " % (key,self.__dict__[key])
        return res

# ----------------------------------------------------------------------------
#  Class: TestCaseTextResult
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
class TestCaseTextResult(object):
    def __init__(self,name,status,nchanges,nlines):
        self.name     = name
        self.status   = status
        self.nchanges = nchanges
        self.nllines  = nlines
    def to_dict(self):
        return dict(self.__dict__)
    @classmethod
    def from_dict(cls,vals):
        return TestCaseTextResult(**vals)
    def error(self):
        return not self.status in ["passed","skipped"]
    def skip(self):
        return self.status == "skipped"
    def message(self):
        res = ""
        for key in ['name',
                    'status',
                    'nchanges',
                    'nlines']:
            res += "%s: %s " % (key,self.__dict__[key])
        return res



# ----------------------------------------------------------------------------
#  Class: JSONIndex
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
class JSONIndex(object):
    def __init__(self,ofile=None):
        self.ofile = ofile
    def write_header(self,opts,tests,ststamp):
        res = {}
        res["info"]    = {"start_timestamp":ststamp,
                          "host": hostname(False)}
        res["options"] = opts
        res["tests"]   = tests
        res["results"] = []
        json_dump(res,self.ofile)
    def add_result(self,result):
        # in the mp case, this is called by multiple test
        # processes, make sure to lock access to avoid
        # corruption
        OutputLock.acquire()
        res = JSONIndex.load_results(self.ofile)
        res["results"].append(result)
        json_dump(res,self.ofile)
        OutputLock.release()
    def finalize(self,etstamp,rtime):
        res = JSONIndex.load_results(self.ofile)
        res["info"]["end_timestamp"] =  etstamp
        res["info"]["runtime"]       =  rtime
        json_dump(res,self.ofile)
        full = JSONIndex.load_results(self.ofile,True)
        json_dump(full,self.ofile)
    @classmethod
    def load_results(cls,fname,load_cases=False):
        res = {}
        if not os.path.isfile(fname):
            return res
        res = json_load(fname)
        if load_cases:
            cls.load_cases(fname,res)
        return res
    @classmethod
    def load_cases(cls,fname,res):
        index_base     = os.path.split(os.path.abspath(fname))[0]
        for script_res in res["results"]:
            if not "details" in list(script_res.keys()):
                tsr = TestScriptResult(**script_res)
                cases_results =  pjoin(index_base,
                                        "json","%s_%s.json" % (tsr.category,tsr.base))
                if os.path.isfile(cases_results):
                    case_vals = json_load(cases_results)
                    script_res["details"] = case_vals
                    script_res["status"]  = tsr.status()
                else:
                    script_res["details"] = {}
def text_summary(json_res,errors_only=False):
    rtxt  = ""
    for r in json_res["results"]:
        tscript_res = TestScriptResult.from_dict(r)
        if not errors_only or tscript_res.error():
            rtxt += "[%s/%s]\n" %( tscript_res.category, tscript_res.base)
            rtxt += " %s\n" % tscript_res.message()
            if "sections" in list(r["details"].keys()):
                for sect in r["details"]["sections"]:
                    if not errors_only and sect['name'] != "<default>":
                        rtxt += "[[%s]]\n" % sect['name']
                    for c_res in sect["cases"]:
                        cr =TestCaseResult.from_dict(c_res)
                        if not errors_only or cr.error():
                            rtxt += "  %s\n" % cr.message()
    return rtxt

def email_summary(json_res,email_from,email_to,smtp_server,errors_only=False):
    errors  = [TestScriptResult.from_dict(v).error() for v in json_res["results"]]
    nerrors = len([ v for v in errors if v == True])
    error   = True in errors
    ntests  = len(errors)
    subject  = "[visit_test_suite] Test Suite Run "
    if error:
        subject+= "*Failed* "
    else:
        subject+= "Succeeded "
    subject += "(%d of %d scripts passed)" % (ntests - nerrors,ntests)
    msg = "\n"
    if error:
        msg += "=====================================\n"
        msg += "Failures:\n"
        msg += "=====================================\n"
        msg +=  text_summary(json_res,True)
    if not errors_only:
        msg += "\n"
        msg += "=====================================\n"
        msg += "All Cases:\n"
        msg += "=====================================\n"
        msg +=  text_summary(json_res)
    body = "\r\n".join([
                        "From: %s" % email_from,
                        "To: %s" % email_to,
                        "Subject: %s" % subject,
                         msg])
    server = smtplib.SMTP(smtp_server)
    server.sendmail(email_from, [email_to], body)
    server.quit()


def main():
    """
    Main entry point for commandline text + email summary from json results.
    """
    if len(sys.argv) < 3:
        print("usage:")
        print(" Text Report:  visit_test_reports.py [results.json] --text  <errors_only=False>")
        print(" Email Report: visit_test_reports.py [results.json] --email [user1@email.com;user2@email.com;...] [smtp server] <errors_only=False>")
        sys.exit(-1)
    errors_only = False
    res = JSONIndex.load_results(sys.argv[1],True)
    if sys.argv[2] == "--text":
        if len(sys.argv)  == 4:
            errors_only = bool(sys.argv[3])
        print(text_summary(res,errors_only))
    else:
        recp_list = sys.argv[3]
        if recp_list.count(";"):
            recp_list = recp_list.split(';')
        smtp_svr  = sys.argv[4]
        if len(sys.argv)  == 6:
            errors_only = bool(sys.argv[5])
        email_summary(res,"visit-developers@ornl.gov",
                      recp_list,
                      smtp_svr,
                      errors_only)

if __name__ == "__main__":
    main()
