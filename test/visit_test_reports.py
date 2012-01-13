#!/bin/env python

"""
file: visit_test_reports.py
description: Beta Test Report Generation.
author: Cyrus Harrison
date: Fri Jan 13 13:45:13 PST 2012
"""

import sys
import string
import os
import shutil

from os.path import join as pjoin

def test_root_dir():
    return os.path.split(os.path.abspath(__file__))[0]

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

class IndexFile(object):
    def __init__(self):
        tfile = pjoin(test_root_dir(),"report_templates","index.html")
        self.tset = HtmlTemplateSet(tfile)
    def header(self,args):
        self.__copy_misc()
        rmode   = args[0]
        rtstamp = args[1]
        mapping = { "run_mode": rmode, "timestamp": rtstamp}
        res = self.tset.use_template("index_header",mapping)
        f = self.__file(True)
        f.write(res)
        f.close()
    def add_entry(self,args):
        cat   = args[0]
        base  = args[1]
        name  = args[2]
        rcode = int(args[3])
        rtime = args[4]
        lsec  = args[5]
        mapping = {"category":cat,
                   "base":base,
                   "name":name,
                   "result_code": rcode,
                   "details": "%s_%s" % (cat,base),
                   "run_time": rtime}
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
        elif rcode  == 0:
            mapping["result_color"] = "failed"
            mapping["result_text"]  = "Failed: exit == 0  unknown"
        f = self.__file()
        res = self.tset.use_template("index_entry",mapping)
        f.write(res)
        f.close()
    def footer(self,args):
        f = self.__file()
        res = res = self.tset.use_template("index_footer",{})
        f.write(res)
        f.close()
    def __file(self,create = False):
        ofname = pjoin("html","index.beta.html")
        if create:
            return open(ofname,"w")
        else:
            return open(ofname,"a")
    def __copy_misc(self):
        des_css = pjoin("html","css")
        des_js  = pjoin("html","js")
        tp_base = pjoin(test_root_dir(),"report_templates")
        if not os.path.isdir(des_css):
            os.mkdir(des_css)
        if not os.path.isdir(des_js):
            os.mkdir(des_js)
        shutil.copy(pjoin(tp_base,"css","styles.css"),des_css)
        shutil.copy(pjoin(tp_base,"js","jquery-latest.js"),des_js)
        shutil.copy(pjoin(tp_base,"js","jquery.tablesorter.js"),des_js)

def main():
    args = sys.argv
    if args[1] == "index":
        idx = IndexFile()
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