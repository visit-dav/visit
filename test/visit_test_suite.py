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

# ----------------------------------------------------------------------------
#  Method: visit_root
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
def visit_root():
    return abs_path(test_path(),"..")


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
# ----------------------------------------------------------------------------
def check_skip(skiplist,test_modes,test_cat,test_file):
    if skiplist is None:
        return False
    # look for modes that match
    for v in skiplist['skip_list']:
        if v['mode'] == test_modes:
            for test in v['tests']:
                if test['category'] == test_cat:
                    if test['file'] == test_file:
                        if not test.has_key("cases"):
                        # skip the entire file if
                        # there are no specific cases
                            return True
    return False


# ----------------------------------------------------------------------------
#  Method: run_visit_test
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
def run_visit_test(args):
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
    rcmd  =  opts.executable + " "
    rcmd +=  opts.vargs + " "
    rcmd +=  "-exec-timeout %d -idle-timeout %d " % (opts.limit,opts.limit)
    if not opts.interactive:
        rcmd +=  "-nowin "
    if not opts.use_pil:
        rcmd +=  "-noPIL "
    rcmd +=  "-timing -cli "
    cfile = pjoin(test_dir,test_base + ".config")
    if os.path.isfile(cfile):
        rcmd += "-config " + cfile + " "
    else:
        rcmd += "-noconfig "
    rcmd += "-geometry %dx%d+32+32 " % (opts.width,opts.height)
    rcmd += " -s %s < %s" % (os.path.abspath(test_script),os.path.abspath(test))
    modes_list = opts.modes.split(",")
    if "dlb" in modes_list:
        rcmd += " -allowdynamic "
    if "icet" in modes_list:
        rcmd += " -icet "
    modes = ""
    if opts.verbose:
        rcmd += " -verbose "
    fuzzy = opts.fuzzy
    if "serial" in modes_list:
        modes = "serial"
    else:
        if "scalable" in modes_list:
            modes = "scalable"
            if not opts.nofuzzy:
                fuzzy = True
        if "parallel" in modes_list:
            if len(modes) > 0:
                modes +=","
            modes +="parallel"
        if "icet" in modes_list:
            if len(modes) > 0:
                modes +=","
            modes +="icet"
    run_dir = pjoin(opts.resultdir,"_run","_%s_%s" % (test_cat, test_base))
    # set opts vars
    run_dir = pjoin(opts.resultdir,"_run","_%s_%s" % (test_cat, test_base))
    tparams = {}
    tparams["category"]       = test_cat
    tparams["name"]           = test_base
    tparams["file"]           = test_file
    tparams["modes"]          = modes
    tparams["run_dir"]        = run_dir
    tparams["result_dir"]     = opts.resultdir
    tparams["fuzzy_match"]    = fuzzy
    tparams["skip_file"]      = None
    tparams["skip_file_win"]  = None
    tparams["interactive"]    = opts.interactive
    tparams["use_pil"]        = opts.use_pil
    tparams["pixdiff"]        = opts.pixdiff
    tparams["avgdiff"]        = opts.avgdiff
    tparams["numdiff"]        = opts.numdiff
    tparams["top_dir"]        = top_dir
    tparams["data_dir"]       = opts.datadir
    tparams["baseline_dir"]   = opts.baselinedir
    tparams["tests_dir"]      = opts.testsdir
    tparams["visit_bin"]      = opts.executable

    if not opts.noskip:
        tparams["skip_file"]  = opts.skipfile
        tparams["skip_file_win"] = opts.skipfilewin
        if not sys.platform.startswith("win"):
            skip = check_skip(opts.skiplist,modes,test_cat,test_file)
        else:
            skip = check_skip(opts.skiplistwin,modes,test_cat,test_file)
            if not skip:
                skip = check_skip(opts.skiplist,modes,test_cat,test_file)

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
            shutil.rmtree(run_dir)
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
        sexe(rcmd,
             suppress_output=(not (opts.verbose or opts.lessverbose)),
             echo=False)
        json_res_file = pjoin(opts.resultdir,"json","%s_%s.json" %(test_cat,test_base))
        if os.path.isfile(json_res_file ):
            results = json_load(json_res_file)
            if results.has_key("result_code"):
                rcode = results["result_code"]
        # get end timestamp
        etime = time.time()
        dtime = math.ceil(etime - stime)
        # wait for any logs to flush
        time.sleep(1)
        # change back to prev working dir
        os.chdir(curr_dir)
        # check for core files
        ncores_files = process_cores(run_dir,opts.resultdir,test_cat,test_base)
        # move logs and timings to the html output dir
        process_runtime_logs(run_dir,opts.resultdir,test_cat,test_base)
        # parse any output files
        result = TestScriptResult(idx,
                                  test_cat,
                                  test_base,
                                  test_file,
                                  rcode,
                                  ncores_files,
                                  dtime)
        if opts.cleanup:
            shutil.rmtree(run_dir)
    log_test_result(opts.resultdir,result)
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
def log_test_result(resultdir,result):
    Log(result.message())
    HTMLIndex(resultdir).add_result(result)
    JSONIndex(pjoin(opts.resultdir,"results.json")).add_result(result)

# ----------------------------------------------------------------------------
#  Method: parse_args
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
def parse_args():
    """
    Parses arguments to runtest.
    """
    parser = OptionParser()
    data_dir_def    = abs_path(visit_root(),"data")
    base_dir_def    = abs_path(visit_root(),"test","baseline")
    tests_dir_def   = abs_path(visit_root(),"test","tests")
    visit_exe_def   = abs_path(visit_root(),"src","bin","visit")
    skip_def        = pjoin(test_path(),"skip.json")
    skip_def_win    = pjoin(test_path(),"skipwin.json")
    nprocs_def      = multiprocessing.cpu_count()
    parser.add_option("-r",
                      "--run-only",
                      dest="use_pil",
                      default=True,
                      action="store_false",
                      help="no image differencing (no PIL)")
    parser.add_option("-d",
                      "--data-dir",
                      dest="datadir",
                      default=data_dir_def,
                      help="path to data directory [default=%s]" % data_dir_def)
    parser.add_option("-b",
                      "--baseline-dir",
                      dest="baselinedir",
                      default=base_dir_def,
                      help="path to baseline directory [default=%s]" % base_dir_def)
    parser.add_option("-t",
                      "--tests-dir",
                      dest="testsdir",
                      default=tests_dir_def,
                      help="path to tests directory [default=%s]" % tests_dir_def)
    parser.add_option("-o",
                      "--output-dir",
                      dest="resultdir",
                      default=test_path(),
                      help="path to output directory [default=%s]" % test_path())
    parser.add_option("-p",
                      "--post",
                      default=False,
                      action="store_true",
                      help="post results to web")
    parser.add_option("-q",
                      "--quiet",
                      default=False,
                      action="store_true",
                      help="suppress all text output")
    parser.add_option("-v",
                      "--verbose",
                      default=False,
                      action="store_true",
                      help="extra test output")
    parser.add_option("--lessverbose",
                      default=False,
                      action="store_true",
                      help="extra test output without progress messages")
    parser.add_option("--width",
                      type="int",
                      default=300,
                      help="set image width")
    parser.add_option("--height",
                      type="int",
                      default=300,
                      help="set image height")
    parser.add_option("-m",
                      "--modes",
                      default="serial",
                      help="specify mode in which to run tests"
                           " [choose from 'parallel','serial','scalable', "
                           " 'dlb','hdf5', 'icet', and combinations such as"
                           " 'scalable,parallel']")
    parser.add_option("-c",
                      "--classes",
                      default="nightly",
                      help="list of classes [choose from 'nightly']")
    parser.add_option("-l",
                      "--limit",
                      type="int",
                      default=600,
                      help="set maximum elapsed run-time for each test")
    parser.add_option("-s",
                      "--skiplist",
                      dest="skipfile",
                      default=skip_def,
                      help="specify a skip list file [default=%s]" % skip_def)
    parser.add_option( "--skiplistwin",
                      dest="skipfilewin",
                      default=skip_def_win,
                      help="specify a skip list file for windows [default=%s]" % skip_def_win)
    parser.add_option("--no-skiplist",
                      dest="noskip",
                      default=False,
                      action = "store_true",
                      help="Do not use a skip list file")
    parser.add_option("--no-data-check",
                      dest="checkdata",
                      default=True,
                      action = "store_false",
                      help="Skip build sanity check on input data files")
    parser.add_option("--fuzzy",
                      dest="fuzzy",
                      default=False,
                      action = "store_true",
                      help="Use fuzzy image matching [default=False]")
    parser.add_option("--no-fuzzy",
                      dest="nofuzzy",
                      default=False,
                      action = "store_true",
                      help="Do not use fuzzy image matching for scalable mode"),
    parser.add_option("--no-cleanup",
                      dest="cleanup",
                      default=True,
                      action = "store_false",
                      help="Do not remove the _run directory"),
    parser.add_option("-e",
                      "--executable",
                      default=visit_exe_def,
                      help="specify executable version of visit to run. "
                           "For example, use \"-e '/usr/gapps/visit/bin/visit -v "
                           " [default = %s]" % visit_exe_def)
    parser.add_option("-i",
                      "--interactive",
                      action = "store_true",
                      default=False,
                      help="don't redirect test .py file into visit."
                           "Just bring up CLI and let user Source() "
                           "the .py file explicitly.")
    parser.add_option("--pixdiff",
                      type="int",
                      default=0,
                      help="allowed % of pixels different [default = 0%]")
    parser.add_option("--avgdiff",
                      type="int",
                      default=0,
                      help="if pixdiff exceeded, allowed mean grayscale diff "
                           "[default = 0]")
    parser.add_option("--numdiff",
                      type="int",
                      default=0,
                      help="allowed relative numerical difference in text "
                           "results [default = 0]")
    parser.add_option("--vargs",
                      default="",
                      help="arguments to pass directly to VisIt "
                           "(surround them \" or ')")
    parser.add_option("--retry",
                      default=False,
                      action="store_true",
                      help="retry failed test cases from last index file")
    parser.add_option("--index",
                      dest="index",
                      help="load test cases from a specific index file")
    parser.add_option("--timeout",
                      type="int",
                      default=3600, # total timeout of one hour
                      help="total test suite timeout in seconds [default = 3600]")
    parser.add_option("-n","--num-processes",
                      dest="nprocs",
                      type=int,
                      default=nprocs_def,
                      help="number of tests to launch simultaneously [default =%d]" % nprocs_def)
    # parse args
    opts, args       = parser.parse_args()
    opts.executable  = abs_path(opts.executable)
    opts.resultdir   = abs_path(opts.resultdir)
    opts.datadir     = abs_path(opts.datadir)
    opts.testsdir    = abs_path(opts.testsdir)
    opts.baselinedir = abs_path(opts.baselinedir)
    opts.classes     = opts.classes.split(",")
    opts.skiplist    = None
    opts.skiplistwin = None
    if os.path.isfile(opts.skipfile):
        try:
            if opts.noskip == False:
                opts.skiplist = json_load(opts.skipfile)
        except:
            opts.skiplist = None

    if sys.platform.startswith("win"):
        if os.path.isfile(opts.skipfilewin):
            try:
                if opts.noskip == False:
                    opts.skiplistwin = json_load(opts.skipfilewin)
            except:
                opts.skiplistwin = None

    tests           = [ abs_path(t) for t in args]
    return opts, tests

# ----------------------------------------------------------------------------
#  Method: find_test_cases
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
def find_test_cases(tests_dir):
    """
    Finds test suite scripts.
    """
    Log("[Finding test scripts]")
    res    = []
    tfiles = glob.glob(pjoin(tests_dir,"*","*.py"))
    tfiles = [ os.path.abspath(tf) for tf in tfiles]
    if len(opts.classes) == 0:
        res = tfiles
    else:
        for tf in tfiles:
            test_lines = open(tf).readlines()
            match = False
            for l in test_lines:
                if l.count("#  CLASSES:"):
                    for c in opts.classes:
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
            cmd = "cd  %s && make test" % data_dir
            sexe(cmd)
    else:
        Log("[Test data file '%s' exists.]" % dfile)

# ----------------------------------------------------------------------------
#  Method: cleanup
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
def cleanup(res_dir):
    Log("[Cleanup: Waiting for delayed writes]")
    time.sleep(10)
    Log("[Cleanup: Removing _run directory]")
    run_dir = pjoin(res_dir,"_run")
    shutil.rmtree(run_dir)

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
    if opts.noskip == False and os.path.isfile(opts.skipfile):
        Log("[Using skip list file: '%s']" % opts.skipfile)
    if sys.platform.startswith("win"):
        if opts.noskip == False and os.path.isfile(opts.skipfilewin):
            Log("[Using skip list file: '%s']" % opts.skipfilewin)
    test_args = [(idx,tests[idx],opts) for idx in range(ntests)]
    # save the input list
    test_list = json_dumps([(idx,tests[idx]) for idx in range(ntests)])
    open(pjoin(opts.resultdir,"tests.json"),"w").write(test_list)
    if len(test_args) < opts.nprocs:
        opts.nprocs = len(test_args)
    if opts.nprocs < 2:
        Log("[Using %s test process]" % opts.nprocs)
        for args in test_args:
            results.append(run_visit_test(args))
    else:
        Log("[Using %s test processes]" % opts.nprocs)
        pool = multiprocessing.Pool(processes=opts.nprocs)
        p = pool.map_async(run_visit_test,test_args)
        try:
            #results = p.get(0xFFFF) # used to make sure ctl-c doesn't hang
            results = p.get(opts.timeout) # used to make sure ctl-c doesn't hang
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
#  Method: main
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
def main(opts,tests):
    """
    Main entry point for the test suite.
    """
    Log("[[VisIt Test Suite]]")
    if opts.checkdata:
        prepare_data_dir(opts.datadir)
    if opts.index:
        ridx  = opts.index
        tests = load_test_cases_from_index(opts.testsdir,ridx)
    elif opts.retry:
        Log("[Retrying failures from previous run]")
        ridx  = pjoin(opts.resultdir,"results.json")
        tests = load_test_cases_from_index(opts.testsdir,ridx,True)
    elif len(tests) == 0:
        tests = find_test_cases(opts.testsdir)
    prepare_result_dirs(opts.resultdir)
    ststamp = timestamp(sep=":")
    stime   = time.time()
    Log("[Starting test suite run @ %s]" % ststamp)
    html_index = HTMLIndex(opts.resultdir)
    html_index.write_header(opts.modes,ststamp)
    json_index = JSONIndex(pjoin(opts.resultdir,"results.json"))
    json_index.write_header(opts,ststamp)
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
    if opts.cleanup:
        cleanup(opts.resultdir)

if __name__ == "__main__":
    opts, tests =parse_args()
    main(opts,tests)

