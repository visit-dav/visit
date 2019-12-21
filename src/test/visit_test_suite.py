#!/bin/env python
# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.
"""
VisIt's test suite.
"""

import sys
import os
import math
import glob
import re
import shutil
import subprocess
import multiprocessing
import time
import math
import datetime
import pickle
import json
import traceback



from os.path import join as pjoin
from optparse import OptionParser

from visit_test_common import *
from visit_test_reports import *
from visit_test_ctest import *

# ----------------------------------------------------------------------------
#  Method: visit_root
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
def visit_root():
    return abs_path(test_path(),"../..")


# ----------------------------------------------------------------------------
#  Method: test_path
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
def test_path():
    return os.path.abspath(os.path.split(__file__)[0])


# ----------------------------------------------------------------------------
#  Method: check_skip
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
#
# Modifications:
#  Cyrus Harrison, Tue May 28 12:05:45 PDT 2013
#  Add support for platform based skips.
#
#  Kathleen Biagas, Thu Nov  8 10:30:36 PST 2018
#  Assume entire category is skipped it 'file' key is missing.
#
# ----------------------------------------------------------------------------
def check_skip(skip_list,test_modes,test_cat,test_file):
    if skip_list is None:
        return False
    # look for modes that match
    for v in skip_list['skip_list']:
        if v['mode'] == test_modes:
            for test in v['tests']:
                # check for platform restrictions
                if "platform" in test:
                    tplat = test["platform"].lower()
                    splat = sys.platform.lower()
                    # win,linux,osx
                    # ignore this entry if we are on the wrong platform
                    # else, use std logic
                    if not splat.startswith(tplat):
                        continue
                if test['category'] == test_cat:
                    if "file" in test:
                        if test['file'] == test_file:
                            if "cases" not in test:
                            # skip the entire file if
                            # there are no specific cases
                                return True
                    else:
                        # skip the entire category
                        return True
    return False
# ----------------------------------------------------------------------------
#  Method: parse_test_specific_vargs
#
#  Programmer: Cyrus Harrison
#  Date:       Tues April 29, 2014
# ----------------------------------------------------------------------------
def parse_test_specific_vargs(test_file):
    lines = [l.strip() for l in open(test_file).readlines()]
    # check for pattern "# ... VARGS: {visit command line args}
    vargs = [l[1:] for l in lines if len(l) > 0 and l[0] == "#" and l.count("VARGS:")  == 1]
    vargs = " ".join(vargs)
    vargs = vargs.replace("VARGS:","")
    return vargs

# ----------------------------------------------------------------------------
#  Method: parse_test_specific_limit
#
#  Programmer: Kathleen Biagas 
#  Date:       Thu Nov 8, 2018
# ----------------------------------------------------------------------------
def parse_test_specific_limit(test_file):
    lines = [l.strip() for l in open(test_file).readlines()]
    # check for pattern "# ... LIMIT: {limit}
    limline = [l[1:] for l in lines if len(l) > 0 and l[0] == "#" and l.count("LIMIT:")  == 1]
    if len(limline) > 0:
        limline = " ".join(limline)
        limline = limline.replace("LIMIT:","")
        return int(limline)
    return -1

# ----------------------------------------------------------------------------
#  Method: launch_visit_test
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
#
#  Modifications:
#    Brad Whitlock, Thu Nov  7 14:00:28 PST 2013
#    Pass width and height to test.
#
#    Kathleen Biagas, Thu Feb  6 14:08:00 PST 2014
#    Pass 'ctest' to test.
#
#    Kathleen Biagas, Fri May 16 15:23:13 PDT 2014
#    Set up sim_dir based off executable location.
#
#    Burlen Loring, Mon May 26 15:36:26 PDT 2014
#    Addedd command line option to use threshold based image diff
#
#    Eric Brugger, Fri Aug 15 10:04:27 PDT 2014
#    I added the ability to specify the parallel launch method.
#
#    Kathleen Biagas, Thu Sep 4 16:45:39 MST 2014
#    Use exe path only for sim_dir on Windows.
#
#    Burlen Loring, Wed Oct 21 15:44:24 PDT 2015
#    Added an option (--display-failed) to display current,
#    baseline, and diff in a popup window as the test runs.
#
#    Kathleen Biagas, Thu Nov  8 10:31:51 PST 2018
#    Added src_dir and cmake_cmd, for plugin-vs-install tests.
#    If test has specified its own 'LIMIT' use it for the kill limit.
#
# ----------------------------------------------------------------------------
def launch_visit_test(args):
    """
    Runs a single VisIt test.
    """
    idx  =  args[0]
    test =  args[1]
    opts =  args[2]
    top_dir     = visit_root()
    test_script = abs_path(test_path(),"visit_test_main.py")
    test_dir, test_file = os.path.split(test)
    test_cat  = os.path.split(test_dir)[1]
    test_base = os.path.splitext(test_file)[0]
    rcmd  =  opts["executable"] + " "
    rcmd +=  opts["vargs"] + " "
    # check for vargs embedded in the test file header
    rcmd +=  parse_test_specific_vargs(test) + " "
    rcmd +=  "-exec-timeout %d -idle-timeout %d " % (opts["limit"],opts["limit"])
    rcmd +=  "-numrestarts 0 "
    if not opts["interactive"]:
        rcmd +=  "-nowin "
    if not opts["use_pil"]:
        rcmd +=  "-noPIL "
    if not opts["no_timings"]:
        rcmd +=  "-timing"
    rcmd += " -cli "
    cfile = pjoin(test_dir,test_base + ".config")
    if os.path.isfile(cfile):
        rcmd += "-config " + cfile + " "
    else:
        rcmd += "-noconfig "
    rcmd += "-geometry %dx%d+32+32 " % (opts["width"],opts["height"])
    rcmd += " -s %s " % os.path.abspath(test_script)
    modes_list = opts["modes"].split(",")
    if "dlb" in modes_list:
        rcmd += " -allowdynamic "
    if "icet" in modes_list:
        rcmd += " -icet "
    modes = ""
    if opts["verbose"]:
        rcmd += " -verbose "
    fuzzy = opts["fuzzy"]
    if "serial" in modes_list:
        modes = "serial"
    else:
        if "scalable" in modes_list:
            modes = "scalable"
            if not opts["no_fuzzy"]:
                fuzzy = True
        if "parallel" in modes_list:
            if len(modes) > 0:
                modes +=","
            modes +="parallel"
        if "icet" in modes_list:
            if len(modes) > 0:
                modes +=","
            modes +="icet"
    run_dir = pjoin(opts["result_dir"],"_run","_%s_%s" % (test_cat, test_base))
    # set opts vars
    tparams = {}
    tparams["script"]         = test
    tparams["category"]       = test_cat
    tparams["name"]           = test_base
    tparams["file"]           = test_file
    tparams["modes"]          = modes
    tparams["run_dir"]        = run_dir
    tparams["result_dir"]     = opts["result_dir"]
    tparams["fuzzy_match"]    = fuzzy
    tparams["skip_file"]      = None
    tparams["interactive"]    = opts["interactive"]
    tparams["use_pil"]        = opts["use_pil"]
    tparams["threshold_diff"] = opts["threshold_diff"]
    tparams["threshold_error"]= opts["threshold_error"]
    tparams["pixdiff"]        = opts["pixdiff"]
    tparams["avgdiff"]        = opts["avgdiff"]
    tparams["numdiff"]        = opts["numdiff"]
    tparams["top_dir"]        = top_dir
    tparams["data_dir"]       = opts["data_dir"]
    tparams["src_dir"]        = opts["src_dir"]
    tparams["data_host"]      = opts["data_host"]
    tparams["baseline_dir"]   = opts["baseline_dir"]
    tparams["tests_dir"]      = opts["tests_dir"]
    tparams["visit_bin"]      = opts["executable"]
    tparams["width"]          = opts["width"]
    tparams["height"]         = opts["height"]
    tparams["ctest"]          = opts["ctest"]
    tparams["display_failed"] = opts["display_failed"]
    tparams["parallel_launch"]= opts["parallel_launch"]
    tparams["host_profile_dir"]   = opts["host_profile_dir"]
    tparams["sessionfiles"]   = opts["sessionfiles"]
    tparams["cmake_cmd"]      = opts["cmake_cmd"]

    exe_dir, exe_file = os.path.split(tparams["visit_bin"])
    if sys.platform.startswith("win"):
        tparams["sim_dir"] = os.path.abspath(exe_dir)
    else:
        tparams["sim_dir"] = os.path.abspath(os.path.join(exe_dir, ".."))

    if not opts["no_skip"]:
        tparams["skip_file"]  = opts["skip_file"]
    skip  =  check_skip(opts["skip_list"],modes,test_cat,test_file)
    if skip:
        Log("[Skipping: %s/%s (found in skip list)]" % (test_cat,test_file))
        result = TestScriptResult(idx,
                                  test_cat,
                                  test_base,
                                  test_file,
                                  116, # full skip return code
                                  0,
                                  0)
    else:
        Log("[Launching: %s/%s]" % (test_cat,test_file))
        # run the test in a unique sub dir
        if os.path.isdir(run_dir):
            try:
                shutil.rmtree(run_dir)
            except OSError as e:
                Log("<Error Cleaning up Script Run Directory before launch> %s" % run_dir)
        os.mkdir(run_dir)
        pfile = open(pjoin(run_dir,"params.json"),"w")
        pfile.write("%s" % json_dumps(tparams))
        pfile.close()
        rcmd += " --params=%s" % os.path.abspath(pjoin(run_dir,"params.json"))
        # get start timestamp
        stime = time.time()
        # change to working dir to our run dir
        curr_dir = os.getcwd()
        os.chdir(run_dir)
        rcode = 0
        test_specific_limit = parse_test_specific_limit(test)
        if test_specific_limit != -1:
            use_limit = test_specific_limit
        else:
            use_limit = opts["limit"]
        sexe_res = sexe(rcmd,
                        suppress_output=(not (opts["verbose"] or opts["less_verbose"])),
                        echo=opts["verbose"],
                        timeout=use_limit * 1.1) # proc kill switch at 110% of the selected timeout
        json_res_file = pjoin(opts["result_dir"],"json","%s_%s.json" %(test_cat,test_base))
        if os.path.isfile(json_res_file):
            results = json_load(json_res_file)
            if "result_code" in results:
                rcode = results["result_code"]
            # os.mkdir(run_dir)
        if sexe_res["killed"]:
            Log("<Limit killed> %s" % os.path.abspath(test))
            # wait for process chain to die
            time.sleep(1)
        # get end timestamp
        etime = time.time()
        dtime = math.ceil(etime - stime)
        # wait for any logs to flush
        time.sleep(1)
        # change back to prev working dir
        os.chdir(curr_dir)
        # check for core files
        ncores_files = process_cores(run_dir,opts["result_dir"],test_cat,test_base)
        # move logs and timings to the html output dir
        process_runtime_logs(run_dir,opts["result_dir"],test_cat,test_base)
        # parse any output files
        result = TestScriptResult(idx,
                                  test_cat,
                                  test_base,
                                  test_file,
                                  rcode,
                                  ncores_files,
                                  dtime)
        if opts["cleanup"]:
            try:
                shutil.rmtree(run_dir)
            except OSError as e:
                Log("<Error Removing Script Run Directory> %s" % run_dir)
    log_test_result(opts["result_dir"],result)
    return result


# ----------------------------------------------------------------------------
#  Method: process_cores
#
#  Programmer: Cyrus Harrison
#  Date:       Thu Aug  9 2012
# ----------------------------------------------------------------------------
def process_cores(run_dir,res_dir,test_cat,test_name):
    MAX_CORE_SIZE = 262144000 # 250 mbytes
    core_files    = glob.glob(pjoin(run_dir,"*core*"))
    num_cores     = len(core_files)
    if num_cores > 0:
        cores_base_dir = pjoin(res_dir,"cores")
        cores_cat_dir  = pjoin(cores_base_dir,test_cat)
        cores_test_dir = pjoin(cores_cat_dir,test_name)
        # make sure the log dirs exists
        for dname in [cores_base_dir, cores_cat_dir, cores_test_dir]:
            if not os.path.isdir(dname):
                os.mkdir(dname)
        # move any cores into this dir
        for cfile in core_files:
            # make sure we don't copy huge core files.
            csize = os.path.getsize(cfile)
            if csize < MAX_CORE_SIZE:
                shutil.copy2(cfile, cores_test_dir)
    return num_cores


# ----------------------------------------------------------------------------
#  Method: process_runtime_logs
#
#  Programmer: Cyrus Harrison
#  Date:       Thu Aug  9 2012
# ----------------------------------------------------------------------------
def process_runtime_logs(run_dir,res_dir,test_cat,test_name):
    rlog_files = glob.glob(pjoin(run_dir,"*vlog"))
    rlog_files.extend(glob.glob(pjoin(run_dir,"*.timings")))
    if len(rlog_files) > 0:
        logs_base_dir = pjoin(res_dir,"logs")
        logs_cat_dir  = pjoin(logs_base_dir,test_cat)
        logs_test_dir = pjoin(logs_cat_dir ,test_name)
        # make sure the log dirs exists
        for dname in [logs_base_dir, logs_cat_dir, logs_test_dir]:
            if not os.path.isdir(dname):
                os.mkdir(dname)
        for rlog in rlog_files:
            shutil.copy2(rlog, logs_test_dir)


# ----------------------------------------------------------------------------
#  Method: log_test_result
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
def log_test_result(result_dir,result):
    Log(result.message())
    HTMLIndex(result_dir).add_result(result)
    JSONIndex(pjoin(result_dir,"results.json")).add_result(result.to_dict())


# ----------------------------------------------------------------------------
#  Method: default_suite_options
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 8 2013
#
#  Modifications:
#    Eric Brugger, Fri Aug 15 10:04:27 PDT 2014
#    I added the ability to specify the parallel launch method.
#
#    Brad Whitlock, Mon Dec 15 15:42:32 PST 2014
#    Added --data-host, --host-profile-dir.
#
#    Burlen Loring, Wed Oct 21 15:44:24 PDT 2015
#    Added an option (--display-failed) to display current,
#    baseline, and diff in a popup window as the test runs.
#
#    Mark C. Miller, Tue Sep  6 18:54:31 PDT 2016
#    Added sessionfiles option to rigoursly test session files
#
#    Kathleen Biagas, Thu Nov  8 10:33:45 PST 2018
#    Added src_dir and cmake_cmd.  
#
#    Eric Brugger, Wed Dec  5 13:05:18 PST 2018
#    Changed the definition of tests_dir_def to the new location of the
#    test directory.
#
#    Kathleen Biagas, Thu Dec 13 10:51:54 PST 2018
#    Changed definition of data_dir to new location.  Assumes in-src build,
#    which is used for nightly regression tests.
#
# ----------------------------------------------------------------------------
def default_suite_options():
    data_dir_def    = abs_path(visit_root(),"src","testdata")
    base_dir_def    = abs_path(visit_root(),"test","baseline")
    tests_dir_def   = abs_path(visit_root(),"src","test","tests")
    visit_exe_def   = abs_path(visit_root(),"src","bin","visit")
    src_dir_def     = abs_path(visit_root(),"src")
    skip_def        = pjoin(test_path(),"skip.json")
    # Set nprocs_def to 1, since multi-proc test mode seems to result in
    # crossed streams. In the past we have used: multiprocessing.cpu_count()
    nprocs_def      = 1 
    opts_full_defs = {
                      "use_pil":True,
                      "threshold_diff":False,
                      "threshold_error":{},
                      "src_dir":      src_dir_def,
                      "data_dir":     data_dir_def,
                      "baseline_dir": base_dir_def,
                      "tests_dir":    tests_dir_def,
                      "result_dir":   test_path(),
                      "post":False,
                      "verbose":False,
                      "less_verbose":False,
                      "quiet":False,
                      "width": 300,
                      "height":300,
                      "modes":"serial",
                      "classes":"nightly",
                      "limit":600,
                      "skip_file":skip_def,
                      "no_skip":False,
                      "check_data":True,
                      "fuzzy":False,
                      "no_fuzzy":False,
                      "cleanup":True,
                      "cleanup_delay":10,
                      "executable":visit_exe_def,
                      "data_host":"localhost",
                      "interactive":False,
                      "pixdiff":0.0,
                      "avgdiff":0.0,
                      "numdiff":0.0,
                      "vargs": "",
                      "host_profile_dir": "",
                      "retry":False,
                      "index":None,
                      "timeout":3600,
                      "nprocs":nprocs_def,
                      "ctest":False,
                      "display_failed":False,
                      "parallel_launch":"mpirun",
                      "sessionfiles":False,
                      "cmake_cmd":"cmake",
                      "no_timings":False,
                      "rsync_post":None}
    return opts_full_defs

def finalize_options(opts):
    opts["executable"]   = abs_path(opts["executable"])
    opts["result_dir"]   = abs_path(opts["result_dir"])
    opts["src_dir"]      = abs_path(opts["src_dir"])
    opts["data_dir"]     = abs_path(opts["data_dir"])
    opts["tests_dir"]    = abs_path(opts["tests_dir"])
    opts["baseline_dir"] = abs_path(opts["baseline_dir"])
    if isinstance(opts["classes"],str):
        opts["classes"]  = opts["classes"].split(",")
    opts["skip_list"]    = None
    if not opts["skip_file"] is None and os.path.isfile(opts["skip_file"]):
        try:
            if opts["no_skip"] == False:
                opts["skip_list"] = json_load(opts["skip_file"])
        except:
            opts["skip_list"] = None

# ----------------------------------------------------------------------------
#  Method: parse_args
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
#
#  Modifications:
#    Eric Brugger, Fri Aug 15 10:04:27 PDT 2014
#    I added the ability to specify the parallel launch method.
#
#    Kathleen Biagas, Wed Nov 5 14:32:21 PST 2014
#    On windows, glob any '*.py' tests names.
#
#    Matthew Wheeler, Mon Dec 15 12:56:00 GMT 2014
#    Changed pixdiff % to be a float rather than an int
#
#    Burlen Loring, Wed Oct 21 15:44:24 PDT 2015
#    Added an option (--display-failed) to display current,
#    baseline, and diff in a popup window as the test runs.
#
#    Mark C. Miller, Tue Sep  6 18:54:31 PDT 2016
#    Added sessionfiles option to rigoursly test session files
#
#    Kathleen Biagas, Thu Nov  8 10:34:27 PST 2018
#    Added '--src' for specifying src_dir, and --cmake for specifying
#    cmake_cmd, used for plugin-vs-install tests.
#
#    Kathleen Biagas, Wed Dec 18 17:22:59 MST 2019
#    For windows, move the glob of '*.py' tests name to after full-path
#    expansion in main.
#
# ----------------------------------------------------------------------------
def parse_args():
    """
    Parses arguments to runtest.
    """
    parser = OptionParser()
    defs = default_suite_options()
    parser.add_option("-r",
                      "--run-only",
                      dest="use_pil",
                      default=True,
                      action="store_false",
                      help="no image differencing (no PIL)")
    parser.add_option( "--src",
                      dest="src_dir",
                      default=defs["src_dir"],
                      help="path to src directory [default=%s]" % defs["src_dir"])
    parser.add_option("-d",
                      "--data-dir",
                      dest="data_dir",
                      default=defs["data_dir"],
                      help="path to data directory [default=%s]" % defs["data_dir"])
    parser.add_option("-b",
                      "--baseline-dir",
                      dest="baseline_dir",
                      default=defs["baseline_dir"],
                      help="path to baseline directory [default=%s]" % defs["baseline_dir"])
    parser.add_option("-t",
                      "--tests-dir",
                      dest="tests_dir",
                      default=defs["tests_dir"],
                      help="path to tests directory [default=%s]" % defs["tests_dir"])
    parser.add_option("-o",
                      "--output-dir",
                      dest="result_dir",
                      default=defs["result_dir"],
                      help="path to output directory [default=%s]" % defs["result_dir"])
    parser.add_option("-p",
                      "--post",
                      default=defs["post"],
                      action="store_true",
                      help="post results to web")
    parser.add_option("-q",
                      "--quiet",
                      default=defs["quiet"],
                      action="store_true",
                      help="suppress all text output")
    parser.add_option("-v",
                      "--verbose",
                      default=defs["verbose"],
                      action="store_true",
                      help="extra test output")
    parser.add_option("--lessverbose",
                      default=defs["less_verbose"],
                      dest="less_verbose",
                      action="store_true",
                      help="extra test output without progress messages")
    parser.add_option("--width",
                      type="int",
                      default=defs["width"],
                      help="set image width")
    parser.add_option("--height",
                      type="int",
                      default=defs["height"],
                      help="set image height")
    parser.add_option("-m",
                      "--modes",
                      default=defs["modes"],
                      help="specify mode in which to run tests"
                           " [choose from 'parallel','serial','scalable', "
                           " 'dlb','hdf5', 'icet', and combinations such as"
                           " 'scalable,parallel']")
    parser.add_option("-c",
                      "--classes",
                      default=defs["classes"],
                      help="list of classes [choose from 'nightly']")
    parser.add_option("-l",
                      "--limit",
                      type="int",
                      default=defs["limit"],
                      help="set maximum elapsed run-time for each test")
    parser.add_option("-s",
                      "--skiplist",
                      dest="skip_file",
                      default=defs["skip_file"],
                      help="specify a skip list file [default=%s]" % defs["skip_file"])
    parser.add_option("--no-skiplist",
                      dest="no_skip",
                      default=defs["no_skip"],
                      action = "store_true",
                      help="Do not use a skip list file")
    parser.add_option("--no-data-check",
                      dest="check_data",
                      default=defs["check_data"],
                      action = "store_false",
                      help="Skip build sanity check on input data files")
    parser.add_option("--fuzzy",
                      dest="fuzzy",
                      default=defs["fuzzy"],
                      action = "store_true",
                      help="Use fuzzy image matching [default=False]")
    parser.add_option("--no-fuzzy",
                      dest="no_fuzzy",
                      default=defs["no_fuzzy"],
                      action = "store_true",
                      help="Do not use fuzzy image matching for scalable mode"),
    parser.add_option("--no-cleanup",
                      dest="cleanup",
                      default=defs["cleanup"],
                      action = "store_false",
                      help="Do not remove the _run directory"),
    parser.add_option("--cleanup-delay",
                      dest="cleanup_delay",
                      default=defs["cleanup_delay"],
                      type="int",
                      help="# of seconds to wait for processes to finish "
                           "before cleaning up. [default=%d]" % defs["cleanup_delay"]),
    parser.add_option("-e",
                      "--executable",
                      default=defs["executable"],
                      help="specify executable version of visit to run. "
                           "For example, use \"-e '/usr/gapps/visit/bin/visit -v "
                           " [default = %s]" % defs["executable"])
    parser.add_option("--data-host",
                      dest="data_host",
                      default=defs["data_host"],
                      help="Specify remote host to use for data and compute engine. "
                           "For example, use \"--data-host vulcan.llnl.gov "
                           " [default = %s]" % defs["data_host"])
    parser.add_option("-i",
                      "--interactive",
                      action = "store_true",
                      default=defs["interactive"],
                      help="don't redirect test .py file into visit."
                           "Just bring up CLI and let user Source() "
                           "the .py file explicitly.")
    parser.add_option("--pixdiff",
                      type="float",
                      default=defs["pixdiff"],
                      help="allowed % of pixels different [default = 0.0%]")
    parser.add_option("--avgdiff",
                      type="float",
                      default=defs["avgdiff"],
                      help="if pixdiff exceeded, allowed mean grayscale diff "
                           "[default = 0]")
    parser.add_option("--numdiff",
                      type="float",
                      default=defs["numdiff"],
                      help="allowed relative numerical difference in text "
                           "results [default = 0.0]")
    parser.add_option("--vargs",
                      default=defs["vargs"],
                      help="arguments to pass directly to VisIt "
                           "(surround them \" or ')")
    parser.add_option("--host-profile-dir",
                      default=defs["host_profile_dir"],
                      help="Specify a directory from which host profiles will be read.")
    parser.add_option("--retry",
                      default=defs["retry"],
                      action="store_true",
                      help="retry failed test cases from last index file")
    parser.add_option("--index",
                      dest="index",
                      default=defs["index"],
                      help="load test cases from a specific index file")
    parser.add_option("--timeout",
                      type="int",
                      default=defs["timeout"], # total timeout of one hour
                      help="total test suite timeout in seconds [default = 3600]")
    parser.add_option("-n","--num-processes",
                      dest="nprocs",
                      type=int,
                      default=defs["nprocs"],
                      help="number of tests to launch simultaneously [default =%d]" % defs["nprocs"])
    parser.add_option("--ctest",
                      default=defs["ctest"],
                      action="store_true",
                      help="generate ctest compatible output")
    parser.add_option("--threshold-diff",
                      default=defs["threshold_diff"],
                      dest="threshold_diff",
                      action="store_true",
                      help="use threshold based image diff")
    parser.add_option("--threshold-error",
                      default=defs["threshold_error"],
                      dest="threshold_error",
                      type=str,
                      action='callback',
                      callback=ParseThresholdOverride,
                      help="Per case overide of the max allowable error for threshold based image diff")
    parser.add_option("--display-failed",
                      default=defs["display_failed"],
                      action="store_true",
                      help="use image magick to display the current baseline and diff for each failure")
    parser.add_option("--parallel-launch",
                      default=defs["parallel_launch"],
                      help="specify the parallel launch method. "
                           "Options are mpirun and srun, or a space-separated "
                           "list of parallel launch options.")
    parser.add_option("--sessionfiles",
                      dest="sessionfiles",
                      action = "store_true",
                      default=defs["sessionfiles"],
                      help="Rigoursly test session files by saving a session, "
                           "validating the XML and then perturbing and "
                           "restoring the session prior to each image save.")
    parser.add_option("--no-timings",
                      dest="no_timings",
                      default=defs["no_timings"],
                      action = "store_true",
                      help="Do not generate timing files.")
    parser.add_option("--rsync-post",
                      default=defs["rsync_post"],
                      help="Post results via rsync")
    parser.add_option( "--cmake",
                      dest="cmake_cmd",
                      default=defs["cmake_cmd"],
                      help="path to cmake executable [default=%s]" % defs["cmake_cmd"])

    # parse args
    opts, tests = parser.parse_args()
    # note: we want a dict b/c the values could be passed without using optparse
    opts = vars(opts)
    return opts, tests

# ----------------------------------------------------------------------------
#  Method: ParseThresholdOverride
#
#  Programmer: Burlen Loring
#  Date:       Tue May 27 11:11:22 PDT 2014
# ----------------------------------------------------------------------------
def ParseThresholdOverride(option, opt, value, parser):
    """
    Convert threshold overrides encodeed in a string like
    --threshold-error=a:b,c:d to a dictionary. Parse
    errors are intentionally fatal.
    """
    d={}
    for pair in value.split(','):
        k,v = pair.split(':')
        d[k] = float(v)
    setattr(parser.values, option.dest, d)

# ----------------------------------------------------------------------------
#  Method: find_test_cases
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
def find_test_cases(tests_dir,test_classes):
    """
    Finds test suite scripts.
    """
    Log("[Finding test scripts]")
    if len(test_classes)> 0:
        Log("[Test Classes: %s]" % str(test_classes))
    res    = []
    tfiles = glob.glob(pjoin(tests_dir,"*","*.py"))
    tfiles = [ os.path.abspath(tf) for tf in tfiles]
    if len(test_classes) == 0 or test_classes == "ANY":
        res = tfiles
    else:
        for tf in tfiles:
            test_lines = open(tf).readlines()
            match = False
            for l in test_lines:
                if l.count("#  CLASSES:"):
                    for c in test_classes:
                        if l.count(c):
                            match = True
            if match:
                res.append(tf)
            else:
                Log("Excluding: %s" %tf)
    return res

# ----------------------------------------------------------------------------
#  Method: load_test_cases_from_index
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
def load_test_cases_from_index(tests_dir,result_idx,only_failures=False):
    """
    Finds test suite scripts.
    """
    Log("[Loading test cases from index file = %s)]" % result_idx)
    res = []
    tests = JSONIndex.load_results(result_idx)
    for t in tests:
        tsr = TestSuiteResult(**t)
        if only_failures and tsr.error():
            res.append(pjoin(tests_dir,tsr.category,tsr.file))
        else:
            res.append(pjoin(tests_dir,tsr.category,tsr.file))
    return res

# ----------------------------------------------------------------------------
#  Method: prepare_result_dirs
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
def prepare_result_dirs(res_dir=None):
    """
    Creates output directories for test suite results
    """
    if res_dir is None:
        res_dir = test_path()
    else:
        if not os.path.isdir(res_dir):
            os.mkdir(res_dir)
    run_dir = pjoin(res_dir,"_run")
    if not os.path.isdir(run_dir):
        os.mkdir(run_dir)
    for d in ["html","json","current","diff"]:
        full_dir = pjoin(res_dir,d)
        if os.path.isdir(full_dir):
            shutil.rmtree(full_dir)
        os.mkdir(full_dir)

# ----------------------------------------------------------------------------
#  Method: prepare_data_dir
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
#
#  Modifications:
#    Kathleen Biagas, Mon Jan 27 13:02:45 MST 2014
#    Change 'make test' to 'make testdata'.
# ----------------------------------------------------------------------------
def prepare_data_dir(data_dir):
    """
    Simple check of test data.
    """
    dfile = pjoin(data_dir,"silo_hdf5_test_data","globe.silo")
    Log("[Checking test data]")
    if not os.path.isfile(dfile):
        Log("[Rebuilding test data based "
            "on the fact that '%s' doesn't exist]" % dfile)
        if sys.platform.startswith("win"):
            # TODO: What should we do on windows if the data doesn't exist?
            pass
        else:
            cwd = os.getcwd()
            os.chdir(data_dir)
            cmd = "make testdata"
            sexe(cmd)
            os.chdir(cwd)
    else:
        Log("[Test data file '%s' exists.]" % dfile)

# ----------------------------------------------------------------------------
#  Method: cleanup
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
def cleanup(res_dir,delay):
    Log("[Cleanup: Waiting for delayed writes]")
    time.sleep(delay)
    Log("[Cleanup: Removing _run directory]")
    run_dir = pjoin(res_dir,"_run")
    try:
        shutil.rmtree(run_dir)
    except OSError as e:
        Log("<Error Removing _run Directory> %s" % run_dir)

# ----------------------------------------------------------------------------
#  Method: launch_tests
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
def launch_tests(opts,tests):
    error = False
    results = []
    ntests = len(tests)
    if ntests == 1:
        Log("[Running %d test case]" % ntests)
    else:
        Log("[Running %d test cases]" % ntests)
    if opts["no_skip"] == False and os.path.isfile(opts["skip_file"]):
        Log("[Using skip list file: '%s']" % opts["skip_file"])
    test_args = [(idx,tests[idx],opts) for idx in range(ntests)]
    # save the input list
    test_list = json_dumps([(idx,tests[idx]) for idx in range(ntests)])
    open(pjoin(opts["result_dir"],"tests.json"),"w").write(test_list)
    if len(test_args) < opts["nprocs"]:
        opts["nprocs"] = len(test_args)
    if opts["nprocs"] < 2:
        Log("[Using %s test process]" % opts["nprocs"])
        for args in test_args:
            results.append(launch_visit_test(args))
    else:
        Log("[Using %s test processes]" % opts["nprocs"])
        pool = multiprocessing.Pool(processes=opts["nprocs"])
        p = pool.map_async(launch_visit_test,test_args)
        try:
            #results = p.get(0xFFFF) # used to make sure ctl-c doesn't hang
            results = p.get(opts["timeout"]) # used to make sure ctl-c doesn't hang
        except KeyboardInterrupt:
            Log("<Test Suite Run aborted by user keyboard interrupt.>")
            pool.terminate()
            error = True
        except Exception as e:
            traceback.print_exc()
            Log("<Unknown Exception>:%s" % str(e))
            pool.terminate()
            error = True
    if len(results) != len(test_list):
        for t in test_args:
            if not t[0] in [ r.index for r in results]:
                test_dir, test_file = os.path.split(t[1])
                test_cat  = os.path.split(test_dir)[1]
                test_base = os.path.splitext(test_file)[0]
                Log("<Missing result from test %s/%s, assuming Killed>" % (test_cat,test_base))
                res  = TestScriptResult(t[0],
                                        test_cat,
                                        test_base,
                                        test_file,
                                        115, # killed return code
                                        0,
                                        0)
                results.append(res)
    error = error or True in [r.error() for r in results]
    return error, results

# ----------------------------------------------------------------------------
#  Method: rsync_post
#
#  Programmer: Cyrus Harrison
#  Date:       ue Nov 25 11:23:12 PST 2014
#
#  Modifications:
# ----------------------------------------------------------------------------
def rsync_post(src_dir,rsync_dest):
    rsync_dest = pjoin(rsync_dest,timestamp(sep="_"))
    rsync_cmd =  "rsync -vur %s/ %s" % (src_dir,rsync_dest)
    Log("[rsyncing results to %s]" % rsync_dest)
    exe_res = sexe(rsync_cmd,
                   suppress_output=(not (opts["verbose"] or opts["less_verbose"])),
                    echo=opts["verbose"])



# ----------------------------------------------------------------------------
#  Method: main
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
#
#  Modifications:
#   Kathleen Biagas, Thu Feb  6 14:08:00 PST 2014
#   Only do ctest logging if ctest is enabled.
#
#   Burlen Loring, Fri Oct  2 09:56:26 PDT 2015
#   Make ctest output for each subtest. Moved the timer code
#   into the ctest module so that I could track time spent
#   in the sub test.
#
#   Kathleen Biagas, Wed Dec 18 17:22:59 MST 2019
#   On windows, glob any '*.py' tests names.
#
# ----------------------------------------------------------------------------
def main(opts,tests):
    """
    Main entry point for the test suite.
    """
    finalize_options(opts)
    Log("[[VisIt Test Suite]]")
    if opts["check_data"]:
        prepare_data_dir(opts["data_dir"])
    if opts["index"]:
        ridx  = opts["index"]
        tests = load_test_cases_from_index(opts["tests_dir"],ridx)
    elif opts["retry"]:
        Log("[Retrying failures from previous run]")
        ridx  = pjoin(opts["result_dir"],"results.json")
        tests = load_test_cases_from_index(opts["tests_dir"],ridx,True)
    elif len(tests) == 0:
        tests = find_test_cases(opts["tests_dir"],opts["classes"]) 
    tests = [ abs_path(pjoin(opts["tests_dir"], "..",t)) for t in tests]
    if sys.platform.startswith("win"):
        # use glob to match any *.py
        expandedtests = []
        for t in tests:
           if not '*' in t:
              expandedtests.append(t)
           else:
              for match in glob.iglob(t):
                 expandedtests.append(match)
        if len(expandedtests) > 0:
            tests = expandedtests
    prepare_result_dirs(opts["result_dir"])
    ststamp = timestamp(sep=":")
    stime   = time.time()
    if opts["ctest"]:
        ctestInitTestTimer()
    Log("[Starting test suite run @ %s]" % ststamp)
    html_index = HTMLIndex(opts["result_dir"])
    html_index.write_header(opts["modes"],ststamp)
    json_index = JSONIndex(pjoin(opts["result_dir"],"results.json"))
    json_index.write_header(opts,tests,ststamp)
    error, results = launch_tests(opts,tests)
    etstamp = timestamp(sep=":")
    etime   = time.time()
    rtime   = str(math.ceil(etime - stime))
    html_index.write_footer(etstamp,rtime)
    json_index.finalize(etstamp,rtime)
    nskip   = len([ r.skip()  for r in results if r.skip() == True])
    Log("[Test suite run complete @ %s (wall time = %s)]" % (etstamp,rtime))
    if nskip > 0:
        Log("-- %d files due to skip list." % nskip)
    if not error:
        Log("++ Test suite run finished with NO errors.")
    else:
        nerrors = len([ r.error() for r in results if r.error() == True])
        if nerrors == 1:
            Log("!! Test suite run finished with %d error." % nerrors)
        else:
            Log("!! Test suite run finished with %d errors." % nerrors)
    if opts["cleanup"]:
        cleanup(opts["result_dir"],opts["cleanup_delay"])
    if opts["rsync_post"] is not None:
        rsync_post(opts["result_dir"],opts["rsync_post"])
    if opts['ctest']:
        if not error:
            sys.exit(0)
        else:
            sys.exit(1)
    return pjoin(opts["result_dir"],"results.json")


# ----------------------------------------------------------------------------
# ----------------------------------------------------------------------------
#  Method: run_visit_test and run_visit_tests
#
#  Programmer: Cyrus Harrison
#  Date:       Tue May 28 13:47:08 PDT 2013
#
#  Note: These are the two functions that comprise the module interface
# ----------------------------------------------------------------------------
# ----------------------------------------------------------------------------

def run_visit_test(script_file,
             data_dir=None,
             baseline_dir=None,
             output_dir=None,
             visit_bin=None,
             nprocs=None,
             verbose=False):
    tests = [script_file]
    return run_visit_tests(tests,
                           data_dir,
                           baseline_dir,
                           output_dir,
                           visit_bin,
                           nprocs,
                           verbose)

def run_visit_tests(tests,
                    data_dir=None,
                    baseline_dir=None,
                    output_dir=None,
                    visit_bin=None,
                    nprocs=None,
                    verbose=False):
    opts = default_suite_options()
    if not data_dir is None:
        opts["data_dir"]     = data_dir
    if not baseline_dir is None:
        opts["baseline_dir"] = baseline_dir
    if not output_dir is None:
        opts["result_dir"] = output_dir
    # Future: try to find "visit" in path?
    if not visit_bin is None:
        opts["executable"] = visit_bin
    # override other default options
    opts["check_data"]    = False
    opts["cleanup_delay"] = 1
    if verbose:
        opts["verbose"] = True
    if not nprocs is None:
        opts["nprocs"] = nprocs
    else:
        opts["nprocs"] = 1 # default to 1 for now
    opts["test_dir"] = os.path.split(os.path.abspath(__file__))[0]
    print(opts["test_dir"])
    res_file  = main(opts,tests)
    return JSONIndex.load_results(res_file,True)


if __name__ == "__main__":
    opts, tests = parse_args()
    main(opts,tests)

