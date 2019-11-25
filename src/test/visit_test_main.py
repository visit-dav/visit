# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.
"""
file: visit_test_main.py
description: Provides VisIt environment to executes test suite scripts.
author:  Cyrus Harrison
date:    May 30, 2012

notes:   Ported/refactored from 'Testing.py'
"""
# ----------------------------------------------------------------------------
#  Modifications:
#
# ----------------------------------------------------------------------------

import atexit
import glob
import gzip
import json
import os
import platform
import shutil
import string
import subprocess
import sys
import tempfile
import thread
import time

import HtmlDiff
import HtmlPython

from stat import *

import xml.etree.ElementTree as ElTree
from xml.etree.ElementTree import ParseError

# check for pil
pil_available = True
try:
    from PIL import Image, ImageChops, ImageStat
except ImportError, pilImpErr:
    pil_available=False

# check for VTK
VTK_available = True
try:
    from vtk import vtkPNGReader, vtkPNMReader, vtkJPEGReader, vtkTIFFReader, \
                    vtkImageResize, vtkImageDifference
    import array
except ImportError, vtkImpErr:
    VTK_available=False

# used to acccess visit_test_common
sys.path.append(os.path.abspath(os.path.split(__visit_script_file__)[0]))

from visit_test_common import *
from visit_test_ctest import *

# list of files to clean up at exit
filesToRemoveUponExit = []

# ----------------------------------------------------------------------------
# ----------------------------------------------------------------------------
#
# Path helper Methods
#
# ----------------------------------------------------------------------------
# ----------------------------------------------------------------------------

# ----------------------------------------------------------------------------
#  Method: out_path
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
def out_path(*args):
    """
    Generates proper absolute path relative to the test suite results directory.
    """
    rargs = [TestEnv.params["result_dir"]]
    rargs.extend(args)
    return abs_path(*rargs)

# ----------------------------------------------------------------------------
#  Method: data_path
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
#
#  Modifications:
#    Brad Whitlock, Fri Dec 12 17:56:34 PST 2014
#    Optionally prepend a remote host to the database path.
# ----------------------------------------------------------------------------
def data_path(*args):
    """
    Generates proper absolute path relative to the 'data' directory.
    """
    rargs = [TestEnv.params["data_dir"]]
    rargs.extend(args)
    dp = abs_path(*rargs)
    if TestEnv.params["data_host"] != "localhost":
        dp = TestEnv.params["data_host"] + ":" + dp
    return dp


# ----------------------------------------------------------------------------
#  Method: cmfe_data_path
#
#  Programmer: Kathleen Biagas
#  Date:       Fri Sep 21 2012
# ----------------------------------------------------------------------------
def cmfe_data_path(*args):
    """
    Generates proper absolute path relative to the 'data' directory,
    with extra 'escapes' for cmfe parsing on Windows.
    """

    if not platform.system() == "Windows":
        return data_path(*args)
    else:
        return data_path(*args).replace("\\", "\\\\\\\\").replace(":", "\:")


# ----------------------------------------------------------------------------
#  Method: silo_data_path
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
def silo_data_path(*args):
    """
    Helper that generates proper silo data absolute file paths.
    Incorporates SILO_MODE logic.
    """
    rargs = ["silo_%s_test_data" % TestEnv.params["silo_mode"]]
    rargs.extend(args)
    return data_path(*rargs)


# ----------------------------------------------------------------------------
#  Method: cmfe_silo_data_path
#
#  Programmer: Kathleen Biagas
#  Date:       Fri Sep 21 2012
# ----------------------------------------------------------------------------
def cmfe_silo_data_path(*args):
    """
    Helper that generates proper silo data absolute file paths.
    Incorporates SILO_MODE logic.
    With extra 'escapes' for cmfe parsing on Windows.
    """
    if not platform.system() == "Windows":
        return silo_data_path(*args)
    else:
        return silo_data_path(*args).replace("\\", "\\\\\\\\").replace(":", "\:")


# ----------------------------------------------------------------------------
#  Method: test_root_path
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
def test_root_path(*args):
    """
    Generates proper absolute path relative to the 'test/' directory.
    """
    rargs = [TestEnv.params["top_dir"],"test"]
    rargs.extend(args)
    return abs_path(*rargs)


# ----------------------------------------------------------------------------
#  Method: test_module_path
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 9 2013
# ----------------------------------------------------------------------------
def test_module_path(*args):
    """
    Generates proper absolute path relative to the directory containing
    the test python scripts.
    """
    rargs = [os.path.split(__visit_script_file__)[0]]
    rargs.extend(args)
    return abs_path(*rargs)


# ----------------------------------------------------------------------------
#  Method: tests_path
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
def tests_path(*args):
    """
    Generates proper absolute path relative to the 'test/tests' directory.
    """
    rargs = [TestEnv.params["tests_dir"]]
    rargs.extend(args)
    return abs_path(*rargs)

# ----------------------------------------------------------------------------
#  Method: visit_bin_path
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
def visit_bin_path(*args):
    """
    Generates proper absolute path relative to the 'bin/visit/..' directory.
    """
    rargs = [abs_path(os.path.split(TestEnv.params["visit_bin"])[0])]
    rargs.extend(args)
    return abs_path(*rargs)

# ----------------------------------------------------------------------------
#  Method: visit_src_path
#
#  Programmer: Kathleen Biagas
#  Date:       November 8, 2018 
# ----------------------------------------------------------------------------
def visit_src_path(*args):
    """
    Generates proper absolute path relative to the 'src' directory.
    """
    rargs = [TestEnv.params["src_dir"]]
    rargs.extend(args)
    return abs_path(*rargs)


# ----------------------------------------------------------------------------
#  Method: baseline_path
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
def test_baseline_path(*args):
    """
    Generates proper absolute path relative to the 'test/baseline' directory.
    """
    rargs = [TestEnv.params["baseline_dir"]]
    rargs.extend(args)
    return abs_path(*rargs)



# ----------------------------------------------------------------------------
#  Method: html_output_file_handle
#
#  Programmer: Cyrus Harrison
#  Date:       Wed Dec 12 2012
# ----------------------------------------------------------------------------
def html_output_file_handle(mode='a'):
    """
    Returns a handle to the html output file for the current test.
    """
    res = open(out_path("html","%s_%s.html" % (TestEnv.params["category"],
                                               TestEnv.params["name"])), mode)
    return res


# ----------------------------------------------------------------------------
#  Method: json_output_file_path
#
#  Programmer: Cyrus Harrison
#  Date:       Wed Dec 12 2012
# ----------------------------------------------------------------------------
def json_output_file_path():
    path = out_path("json","%s_%s.json" % (TestEnv.params["category"],
                                           TestEnv.params["name"]))
    return path

# ----------------------------------------------------------------------------
#  Method: load_json_results
#
#  Programmer: Cyrus Harrison
#  Date:       Wed Dec 12 2012
# ----------------------------------------------------------------------------
def json_results_load():
    """
    Loads and returns the current set of json results as a dict.
    """
    return json_load(json_output_file_path())

# ----------------------------------------------------------------------------
#  Method: load_json_results
#
#  Programmer: Cyrus Harrison
#  Date:       Wed Dec 12 2012
# ----------------------------------------------------------------------------
def json_results_save(obj):
    """
    Saves the passed dict to the json results file.
    """
    json_dump(obj,json_output_file_path())


# ----------------------------------------------------------------------------
#  Method: TestScriptPath
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 21 2010
# ----------------------------------------------------------------------------
def TestScriptPath():
    """
    Helper that provides the full path to the current script file
    """
    script_file     = TestEnv.params["file"]
    script_category = TestEnv.params["category"]
    script_dir      = tests_path(script_category,script_file)
    return script_dir

def RemoveFile(f):
    """
    Remove a file, failing silently if it did not exist to begin with
    """
    try:
        os.unlink(f)
    except:
        pass
   

# ----------------------------------------------------------------------------
# Function: GenFileNames
#
# Purpose:
#   Return the file names of the baseline, the current and the difference
#   files.
#
# Returns:
#   cur      The name of the current file.
#   diff     The name of the difference file.
#   base     The name of the baseline file.
#   altbase  The name of the mode specific baseline.
#   modeSpecific A flag indicating if the baseline is mode specific.
#
# Modifications:
#   Eric Brugger, Tue Apr 27 13:23:16 PDT 2010
#   I added the modeSpecific return value, which indicates if the baseline
#   image is mode specific.
#
#   Eric Brugger, Wed May 16 10:15:29 PDT 2018
#   I corrected a bug where mode_specific wasn't being set properly
#   when it was using a mode specific baseline that wasn't listed in
#   "mode_specific.json".
#
# ----------------------------------------------------------------------------
def GenFileNames(test_case, ext):
    pyfilebase = TestEnv.params["name"]
    category   = TestEnv.params["category"]
    modes      = TestEnv.params["modes"]

    dcur_cat   = out_path("current",category)
    dcur_base  = pjoin(dcur_cat,pyfilebase)
    ddiff_cat  = out_path("diff",category)
    ddiff_base = pjoin(ddiff_cat,pyfilebase)

    for rdir in [dcur_cat, dcur_base, ddiff_cat, ddiff_base]:
        if not os.path.isdir(rdir):
            os.mkdir(rdir)
    # create file names
    cur  = pjoin(dcur_base,test_case  + ext)
    diff = pjoin(ddiff_base,test_case + ext)
    base = test_baseline_path(category,pyfilebase,test_case + ext)
    altbase = ""
    mode_specific = 0
    if modes != "":
        mode_dir = modes.replace(",","_")
        altbase  = test_baseline_path(category,pyfilebase,mode_dir,test_case + ext)
        modefile = test_baseline_path(category,pyfilebase,"mode_specific.json")
        if os.path.isfile(altbase):
            # check for alternate mapping from
            base = altbase
            mode_specific = 1
            Log("Using mode specific baseline: %s" % base)
        elif os.path.isfile(modefile):
            modemap = json_load(modefile)
            if modes in modemap["modes"]:
                selected_mode = modemap["modes"][modes]
                case_file     = test_case + ext
                if case_file in selected_mode.keys():
                    ms_dir  = selected_mode[case_file]
                    altbase = test_baseline_path(category,pyfilebase,ms_dir,case_file)
                    base = altbase
                    mode_specific = 1
                    Log("Using mode specific baseline: %s (from %s)" % (base,modefile))
    return (cur, diff, base, altbase, mode_specific)

# ----------------------------------------------------------------------------
#  Method: CalcDiffState
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
def CalcDiffState(p_pixs, d_pixs, davg):
    if p_pixs != 0:
        dpix = d_pixs * 100.0 / p_pixs
        if dpix > TestEnv.params["pixdiff"]:
            if davg > TestEnv.params["avgdiff"]:
                diff_state = 'Unacceptable'
            else:
                diff_state = 'Acceptable'
        else:
            diff_state  = 'Acceptable'
    else:
        if d_pixs != 0:
            dpix = 1000000.0
            diff_state = 'Unacceptable'
        else:
            dpix = 0.0
            diff_state = 'None'
    return diff_state, dpix

# ----------------------------------------------------------------------------
#  Method: LogTestStart
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
def LogTestStart():
    """
    Add test file info to log file, and begin test html output.
    """
    msg  = "\n"
    msg += " - - - - - - - - - - - - - - -\n"
    msg += "  START:  Test script %s\n" % TestEnv.params["file"]
    msg += "\n"
    Log(msg)
    JSONTestStart()
    HTMLTestStart()

# ----------------------------------------------------------------------------
#  Method: LogTestExit
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
def LogTestExit(excode):
    """
    Add test exit info to log file, and complete test html output.
    """
    msg  = "\n"
    msg += "  EXIT:   Test script %s\n" % TestEnv.params["file"]
    msg += "  EXCODE: %d\n" % excode
    msg += " - - - - - - - - - - - - - - -\n"
    msg += "\n"
    Log(msg)
    JSONTestExit(excode)
    HTMLTestExit(excode)



# ----------------------------------------------------------------------------
#  Method: HTMLTestStart
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
def HTMLTestStart():
    """
    Begin test html output.
    """
    # TODO: use template file
    html = html_output_file_handle(mode='wt')
    html.write("<SCRIPT TYPE=\"text/javascript\">\n")
    html.write("<!--\n")
    html.write("function popup(mylink, name)\n")
    html.write("{\n")
    html.write("if (! window.focus)return true;\n")
    html.write("var href;\n")
    html.write("if (typeof(mylink) == 'string')\n")
    html.write("   href=mylink;\n")
    html.write("else\n")
    html.write("   href=mylink.href;\n")
    html.write("window.open(href,name,'width=500,height=500,scrollbars=no');\n")
    html.write("return false;\n")
    html.write("}\n")
    html.write("//-->\n")
    html.write("</SCRIPT>\n")
    html.write("<html><head><title>Results for %s/%s</title></head>\n" % (TestEnv.params["category"],TestEnv.params["file"]))
    html.write("<body bgcolor=\"#a0a0f0\">\n")
    html.write("<H1>Results of VisIt Regression Test - <a href=%s_%s_py.html>%s/%s</a></H1>\n" % (TestEnv.params["category"],TestEnv.params["name"],TestEnv.params["category"],TestEnv.params["name"]))
    html.write("<table border>\n")
    html.write(" <tr>\n")
    html.write("  <td rowspan=2><b><i>Test Case</b></i></td>\n")
    html.write("  <td colspan=2 align=center><b><i>Errors</b></i></td>\n")
    html.write("  <td colspan=3 align=center><b><i>Images</b></i></td>\n")
    html.write(" </tr>\n")
    html.write(" <tr>\n")
    html.write("  <td>%Diffs</td>\n")
    html.write("  <td>Maximum</td>\n")
    html.write("  <td>Baseline</td>\n")
    html.write("  <td>Current</td>\n")
    html.write("  <td>Diff Map</td>\n")
    html.write(" </tr>\n")
    html.write("\n")


# ----------------------------------------------------------------------------
#  Method: JSONTestStart
#
#  Programmer: Cyrus Harrison
#  Date:       Wed Dec  12 2012
# ----------------------------------------------------------------------------
def JSONTestStart():
    res = {}
    res["env"] =  dict(TestEnv.params)
    res["sections"] = [{"name":"<default>","cases":[]}]
    json_results_save(res)



# ----------------------------------------------------------------------------
#  Method: HTMLTestExit
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
def HTMLTestExit(excode):
    """
    Complete test html output.
    """
    # TODO use template file
    html = html_output_file_handle()
    html.write("</table>\n")
    html.write("<p>Final Return Code: %s</p>\n" % str(excode))
    html.write("</body>\n")
    html.write("</html>\n")

# ----------------------------------------------------------------------------
#  Method: JSONTestExit
#
#  Programmer: Cyrus Harrison
#  Date:       Wed Dec  12 2012
# ----------------------------------------------------------------------------
def JSONTestExit(excode):
    res = json_results_load()
    res["result_code"] = excode
    json_results_save(res)



# ----------------------------------------------------------------------------
#  Method: LogTextTestResult
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
def LogTextTestResult(case_name,nchanges,nlines,failed,skip):
    """
    Log the result of a text based test.
    """
    if failed:
        if skip:
            status = "skipped"
        else:
            status = "failed"
    else:
        if nchanges < 0:
            status = "unknown"
        else:
            status = "passed"
    # write html result
    Log("    Test case '%s' %s" % (case_name,status.upper()))
    JSONTextTestResult(case_name,status,nchanges,nlines,failed,skip)
    HTMLTextTestResult(case_name,status,nchanges,nlines,failed,skip)

# ----------------------------------------------------------------------------
#  Method: JSONTextTestResult
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
def JSONTextTestResult(case_name,status,nchanges,nlines,failed,skip):
    res = json_results_load()
    t_res = {'name':     case_name,
             'status':   status,
             'nchanges': nchanges,
             'nlines':   nlines}
    res["sections"][-1]["cases"].append(t_res)
    json_results_save(res)


# ----------------------------------------------------------------------------
#  Method: HTMLTextTestResult
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
def HTMLTextTestResult(case_name,status,nchanges,nlines,failed,skip):
    """
    Creates html entry for the result of a text based test.
    """
    # TODO use template file
    html = html_output_file_handle()
    # write to the html file
    color = "#00ff00"
    if failed:
        if skip:
            color = "#0000ff"
        else:
            color = "#ff0000"
    else:
        if (nchanges < 0):
            color = "#00ffff"
    html.write(" <tr>\n")
    html.write("  <td bgcolor=\"%s\"><a href=\"%s.html\">%s</a></td>\n" % (color, case_name, case_name))
    html.write("  <td colspan=5 align=center>%d modifications totalling %d lines</td>\n" % (nchanges,nlines))
    html.write(" </tr>\n")

# ----------------------------------------------------------------------------
#  Method: LogAssertTestResult
#
#  Programmer: Cyrus Harrison
#  Date:       Fri Nov 22 2013
# ----------------------------------------------------------------------------
def LogAssertTestResult(case_name,assert_check,result,details,skip):
    """
    Log the result of an assert based test.
    """
    details = str(details)
    if not result:
        if skip:
            status = "skipped"
        else:
            status = "failed"
    else:
        status = "passed"
    # write result
    Log("    Test case '%s' (Assert: %s) %s" % (case_name,
                                                assert_check,
                                                status.upper()))
    JSONAssertTestResult(case_name,status,assert_check,result,details,skip)
    HTMLAssertTestResult(case_name,status,assert_check,result,details,skip)

# ----------------------------------------------------------------------------
#  Method: JSONAssertTestResult
#
#  Programmer: Cyrus Harrison
#  Date:       Fri Nov 22 2013
# ----------------------------------------------------------------------------
def JSONAssertTestResult(case_name,status,assert_check,result,details,skip):
    res = json_results_load()
    
    if not result:
        if skip:
            status = "skipped"
        else:
            status = "failed"
    else:
        status = "passed"
    
    t_res = {'name':         case_name,
             'status':       status,
             'assert_check': assert_check,
             'details':      details}
    res["sections"][-1]["cases"].append(t_res)
    json_results_save(res)


# ----------------------------------------------------------------------------
#  Method: HTMLTextTestResult
#
#  Programmer: Cyrus Harrison
#  Date:       Fri Nov 22 2013
# ----------------------------------------------------------------------------
def HTMLAssertTestResult(case_name,status,assert_check,result,details,skip):
    """
    Creates html entry for the result of an assert based test.
    """
    # TODO use template file
    html = html_output_file_handle()
    # write to the html file
    color = "#00ff00"
    if not result:
        if skip:
            color = "#0000ff"
        else:
            color = "#ff0000"
    html.write(" <tr>\n")
    html.write("  <td bgcolor=\"%s\">%s</td>\n" % (color, case_name))
    html.write("  <td colspan=5 align=center> %s : %s (Assert_%s)</td>\n" % (details,str(result),assert_check))
    html.write(" </tr>\n")

# ----------------------------------------------------------------------------
#  Method: LogImageTestResult
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
#
#  Modifications:
#   Kathleen Biagas, Thu Feb  6 14:08:00 PST 2014
#   Only do ctest logging if ctest is enabled.
#
#   Burlen Loring, Tue May 27 11:44:04 PDT 2014
#   Report threshold error
#
#   Burlen Loring, Fri Oct  2 09:13:04 PDT 2015
#   report image error, cpu and walltime. for every test
#
#   Burlen Loring, Wed Oct 21 15:41:57 PDT 2015
#   * fix double reporting of image error
#   * added an option to display the current, baseline and diff in
#     a popup window during the test
#
# ----------------------------------------------------------------------------
def LogImageTestResult(case_name,
                       diffState,modeSpecific,
                       tPixs, pPixs, dPixs, dpix, davg,
                       cur, diff, base, thrErr):
    """
    Log the result of an image based test.
    """
    # write data to the log file if there is one
    if TestEnv.params["ctest"]:
        Log(ctestReportDiff(thrErr))
    details = ""
    if diffState == 'None':
        status = "passed"
    elif diffState == 'Acceptable':
        status  = "passed"
        details = "#pix=%06d, #nonbg=%06d, #diff=%06d, ~%%diffs=%.3f, avgdiff=%3.3f, threrr=%3.3f" \
                    % (tPixs, pPixs, dPixs, dpix, davg, thrErr)
    elif diffState == 'Unacceptable':
        status  = "failed"
        details = "#pix=%06d, #nonbg=%06d, #diff=%06d, ~%%diffs=%.3f, avgdiff=%3.3f, threrr=%3.3f" \
                    % (tPixs, pPixs, dPixs, dpix, davg, thrErr)
        if TestEnv.params["ctest"]:
            Log(ctestReportDiffImages(cur,diff,base))
        if TestEnv.params["display_failed"]:
            os.system("montage -background '#555555' -geometry +16+16 -label 'current\\n%%f' %s -label 'baseline\\n%%f' %s -label 'diff\\n%%f' %s del.png && display del.png"%(cur, base, diff))
    elif diffState == 'Skipped':
        status = "skipped"
    else:
        status = "unknown"
        details = "#pix=UNK , #nonbg=UNK , #diff=UNK , ~%%diffs=UNK,  avgdiff=UNK threrr=UNK"
    msg = "    Test case '%s' %s" % (case_name,status.upper())
    if details !="":
        msg += ": " + details
    if TestEnv.params["ctest"]:
        testTime = ctestGetElapsedTime()
        Log(ctestReportWallTime(testTime))
        Log(ctestReportCPUTime(testTime))
    Log(msg)
    JSONImageTestResult(case_name, status,
                        diffState,modeSpecific,
                        tPixs, pPixs,dPixs, dpix, davg)
    # write html result
    HTMLImageTestResult(case_name, status,
                        diffState,modeSpecific,
                        tPixs, pPixs,dPixs, dpix, davg)

# ----------------------------------------------------------------------------
#  Method: LogImageTestResult
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
def JSONImageTestResult(case_name, status,
                        diffState, modeSpecific,
                        dpix, tPixs, pPixs, dPixs, davg):
    res = json_results_load()
    t_res = {'name':          case_name,
             'status':        status,
             'diff_state':    diffState,
             'mode_specific': modeSpecific,
             'total_pixels':  tPixs,
             'non_bg_pixels': pPixs,
             'diff_pixels':   dPixs,
             'diff_percent':  dpix,
             'avg_pixels':    davg}
    res["sections"][-1]["cases"].append(t_res)
    json_results_save(res)

# ----------------------------------------------------------------------------
# Function: Save_Validate_Perturb_Restore_Session
#
# Purpose: Rigorously test session files by checking xml compliance 
# (which requires xmllint) and also saving the session and then perturbing
# thestate and restoring the saved session
#
# Programmer: Mark C. Miller, Tue Sep  6 18:53:39 PDT 2016
#
# Modifications
#
# Mark C. Miller, Wed Sep  7 17:38:39 PDT 2016
# Added logic to have xmllint save its output and then have VisIt attempt
# to read the file xmllint produces. This tests VisIt's ability to read
# foreign-generated XML content which may vary a bit from how VisIt writes it.
#
# Mark C. Miller, Wed Sep 14 13:25:12 PDT 2016
# Added logic to fallback to python's xml tools if xmllint is not available.
# ----------------------------------------------------------------------------
def Save_Validate_Perturb_Restore_Session(cur):
    retval = 0
    trans = string.maketrans("():; #","______")
    sfile = "%s.session"%string.translate(string.rstrip(cur,".png"),trans)
    ofile = "%s.xmlized.session"%string.translate(string.rstrip(cur,".png"),trans)
    RemoveFile(sfile)
    SaveSession(sfile)

    # Coarse check for xml validity using xmllint tool and for VisIt's ability to
    # re-read xml from some producer other than VisIt
    try:
        xmlvalid = subprocess.call(["xmllint", "--output", ofile, "--postvalid", "--dtdvalid", \
            abs_path(test_root_path(),"visit.dtd"), sfile])
    except:
        try:
            # Fallback to python's xml 
            xmltree = ElTree.parse(sfile)
            if xmltree:
                xmltree.write(ofile, "US-ASCII")
                xmlvalid = 0
        except:
            xmlvalid = 1

    if xmlvalid != 0:
        retval = 1
    else:
        try:
            xmlvalid = RestoreSession(ofile, 0)
            if xmlvalid != 1L:
                retval = 1
        except:
            retval = 1

    # Delete all plots and all windows but 1
    for i in range(16,1,-1):
        try:
            SetActiveWindow(i)
            DeleteAllPlots()
            DeleteWindow()
        except:
            pass
    SetActiveWindow(1)
    DeleteAllPlots()

    # Close all databases
    gatts = GetGlobalAttributes()
    for db in gatts.sources:
        CloseDatabase(db)

    # Restore a crummy session file that changes a lot of stuff wildly
    # so that when we restore the real session, we have a higher likelihood
    # of encountering overlooked session issus
    RestoreSession(abs_path(test_root_path(),"crummy.session"),0)

    # Now restore the original session
    RestoreSession(sfile,0)

    return retval

# ----------------------------------------------------------------------------
# Function: Test
#
# Purpose:
#   Write out the file, compare it to the baseline, thumbnail it,
#   and add it's data to the html
#
# Modifications:
#   Mark C. Miller, Mon Mar 29 19:37:26 PST 2004
#   Added alternate SaveWindowAttributes
#
#   Mark C. Miller, Tue Mar 30 15:48:25 PST 2004
#   Added pause for interacitve mode
#
#   Brad Whitlock, Tue Mar 30 16:38:52 PST 2004
#   Added code to sample memory.
#
#   Mark C. Miller, Mon Apr 12 16:34:50 PDT 2004
#   Added code to test against an alternate baseline if one exists
#
#   Jeremy Meredith, Tue May  4 13:26:41 PDT 2004
#   Catch exceptions from failing to open a baseline.  This can happen if
#   you make a clearcase element before putting an image into it.
#
#   Mark C. Miller, Tue Nov 28 23:13:08 PST 2006
#   Replaced maxerr, maxrms, maxpix with diffState. Added diff measure
#   indicating amount of diffs in pixels and not just count of diff pixels
#
#   Mark C. Miller, Tue Nov 28 23:50:15 PST 2006
#   Changed maxdiff to meddiff
#
#   Mark C. Miller, Wed Nov 29 08:19:52 PST 2006
#   Changed meddiff to avgdiff
#
#   Sean Ahern, Thu Dec 20 14:48:14 EST 2007
#   Made diffState be a string so its easier to understand.
#
#   Eric Brugger, Thu Apr 22 12:56:41 PDT 2010
#   I made several changes to the return code behavior of the script.  It
#   returns error code 119 if the test succeeded and the test had some skips.
#   It returns error code 120 if the test had acceptable differences and
#   had some skips.  It returns error code 113 if the differences were
#   unacceptable regardless of whether some tests were skipped.
#
#   Eric Brugger, Tue Apr 27 13:23:16 PDT 2010
#   I enhanced the routine so that the text next to the large baseline image
#   indicates if it is a mode specific image or not.
#
#   Eric Brugger, Tue Jul 13 15:03:54 PDT 2010
#   I added the optional argument alreadySaved that indicates if the image
#   has already been saved.
#
#   Brad Whitlock, Thu Nov  7 14:01:26 PST 2013
#   Force width and height for the window.
#
#   Kathleen Biagas, Wed Jan 15 09:39:13 MST 2014
#   Saved alternate SaveWindowAttributes for use in GetBackgroundImage
#
#   Mark C. Miller, Tue Sep  6 18:51:23 PDT 2016
#   Added Save_Validate_... to rigorously test sessionfiles
# ----------------------------------------------------------------------------
def Test(case_name, altSWA=0, alreadySaved=0):
    CheckInteractive(case_name)
    # for read only globals, we don't need to use "global"
    # we may need to use global for these guys
    #global maxds
    #global numskip
    global savedAltSWA

    (cur, diff, base, altbase, modeSpecific) = GenFileNames(case_name, ".png")

    if TestEnv.params["sessionfiles"]:
        sessState = Save_Validate_Perturb_Restore_Session(cur)

    # save the window in visit
    if alreadySaved == 0:
        if altSWA != 0:
            sa = altSWA
            savedAltSWA = altSWA
        else:
            savedAltSWA = 0
            sa = SaveWindowAttributes()
            sa.screenCapture=1
            # Force the active window to be the right size.
            width = TestEnv.params["width"]
            height = TestEnv.params["height"]
            g = GetGlobalAttributes()
            win = g.windows[g.activeWindow]
            ResizeWindow(win, width, height)
        sa.family   = 0
        sa.fileName = cur
        sa.format   = sa.PNG
        SetSaveWindowAttributes(sa)
        SaveWindow()

    diffState = 'Unknown'
    skip      = TestEnv.check_skip(case_name)
    tPixs     = 0
    pPixs     = 0
    dPixs     = 0
    dpix      = 0.0
    davg      = 0.0
    thrErr    = -1.0

    if TestEnv.params["use_pil"]:
        if TestEnv.params["threshold_diff"]:
            # threshold difference
            diffState, thrErr, (tPixs, pPixs, dPixs, davg) \
                = DiffUsingThreshold(case_name, cur, diff, base, altbase,
                               TestEnv.params["threshold_error"])
        else:
            # raw difference
            (tPixs, pPixs, dPixs, davg) \
                = DiffUsingPIL(case_name, cur, diff, base, altbase)
            diffState, dpix = CalcDiffState(pPixs, dPixs, davg)

    if skip:
        diffState = 'Skipped'
        TestEnv.results["numskip"]+= 1

    LogImageTestResult(case_name,diffState, modeSpecific,
                       dpix, tPixs, pPixs, dPixs, davg,
                       cur, diff, base, thrErr)

    # update maxmimum diff state
    diffVals = {
        'None' :         0,
        'Acceptable' :   1,
        'Unacceptable' : 2,
        'Unknown' :      3,
        'Skipped' :      0
    }

    # Capture session file xml validity failure as Unacceptable if all else was ok
    if TestEnv.params["sessionfiles"] and sessState != 0 and diffVals[diffState] < 2:
            diffState = 'Unacceptable'

    TestEnv.results["maxds"] = max(TestEnv.results["maxds"], diffVals[diffState])

# ----------------------------------------------------------------------------
# Function: HTMLImageTestResult
#
#
# Modifications:
#   Mark C. Miller, Mon Jul  6 22:07:07 PDT 2009
#   Generate 'mouse-over' hrefs ONLY for case where there are diffs.
#   When there are no diffs, reference baseline instead of current. The
#   rationale for this later change is so that we can then create symlinks
#   for the html content instead of making copies and taking more disk space.
#
#   Eric Brugger, Tue Apr 27 13:23:16 PDT 2010
#   I added the modeSpecific argument, which causes the text next to the
#   baseline image to indicate if it is a mode specific image or not.
#
# ----------------------------------------------------------------------------

def HTMLImageTestResult(case_name,status,
                        diffState, modeSpecific,
                        dpix, tPixs, pPixs, dPixs, davg):
    """
    Writes HTML entry for a single test image.
    """
    # TODO use template file
    html = html_output_file_handle()
    # write to the html file
    color = "#ffffff"
    if   diffState == 'None':           color = "#00ff00"
    elif diffState == 'Acceptable':     color = "#ffff00"
    elif diffState == 'Unacceptable':   color = "#ff0000"
    elif diffState == 'Skipped':        color = "#0000ff"
    else:                               color = "#ff00ff"
    html.write(" <tr>\n")
    html.write("  <td bgcolor=\"%s\"><a href=\"%s.html\">%s</a></td>\n" % (color, case_name, case_name))
    html.write("  <td align=center>%.2f</td>\n" % (dpix))
    html.write("  <td align=center>%.2f</td>\n" % (davg))
    if (diffState == 'Unknown'):
        html.write("  <td align=center>Not Available</td>\n")
        html.write("  <td align=center><a href=\"b_%s.png\" onclick='return popup(\"b_%s.png\",\"image\");'><img src=\"b_%s_thumb.png\"></a></td>\n" % (case_name,case_name,case_name))
        html.write("  <td align=center>Not Available</td>\n")
    elif (diffState != 'None'):
        html.write("  <td align=center><a href=\"b_%s.png\" onclick='return popup(\"b_%s.png\",\"image\");'><img src=\"b_%s_thumb.png\"></a></td>\n" % (case_name,case_name,case_name))
        html.write("  <td align=center><a href=\"c_%s.png\" onclick='return popup(\"c_%s.png\",\"image\");'><img src=\"c_%s_thumb.png\"></a></td>\n" % (case_name,case_name,case_name))
        html.write("  <td align=center><a href=\"d_%s.png\" onclick='return popup(\"d_%s.png\",\"image\");'><img src=\"d_%s_thumb.png\"></a></td>\n" % (case_name,case_name,case_name))
    else:
        html.write("  <td colspan=3 align=center><a href=\"b_%s.png\" onclick='return popup(\"b_%s.png\",\"image\");'><img src=\"b_%s_thumb.png\"></a></td>\n" % (case_name,case_name,case_name))
    html.write(" </tr>\n")

    # write the individual testcase
    testcase = open(out_path("html","%s.html" % case_name), 'wt')
    testcase.write("<html><head><title>Results for test case %s</title></head>\n" % case_name)
    testcase.write("<h1>Results for test case <i>%s</i></h1>\n" % case_name)
    testcase.write("<body bgcolor=\"#a080f0\">\n")
    testcase.write("<table border=5><tr><td></td></tr>\n")
    testcase.write("  <tr>\n")
    testcase.write("    <td align=center rowspan=9 bgcolor=%s>\n"%color)
    if (diffState == 'None' or diffState == 'Acceptable'):
        testcase.write("        <b><h1><i>Passed</i></h1></b>\n");
    elif (diffState == 'Unacceptable'):
        testcase.write("        <b><h1><i>Failed</i></h1></b>\n");
    elif (diffState == 'Skipped'):
        testcase.write("        <b><h1><i>Skipped</i></h1></b>\n");
    else:
        testcase.write("        <b><h1><i>Unknown</i></h1></b>\n");
    testcase.write("    </td>\n")
    if modeSpecific:
        testcase.write("    <td align=center>Mode<br>Specific<br>Baseline:</td>\n")
    else:
        testcase.write("    <td align=center>Baseline:</td>\n")
    if (diffState == 'None'):
        testcase.write("""    <td><img name="b" border=0 src="b_%s.png"></img></td>\n"""%case_name)
    elif (diffState == 'Unknown'):
        testcase.write("    <td>Not Available</td>\n")
    elif (diffState == 'Skipped'):
        testcase.write("""    <td><img name="b" border=0 src="b_%s.png"></img></td>\n"""%case_name)
    else:
        testcase.write("""    <td><a href="" onMouseOver="document.b.src='c_%s.png'" onMouseOut="document.b.src='b_%s.png'"><img name="b" border=0 src="b_%s.png"></img></a></td>\n"""%(case_name,case_name,case_name))
    testcase.write("  </tr>\n")
    testcase.write("  <tr>\n")
    testcase.write("    <td align=center>Current:</td>\n")
    if (diffState == 'None'):
        testcase.write("    <td>Same As Baseline</td>\n")
    elif (diffState == 'Unknown'):
        testcase.write("    <td>Not Available</td>\n")
    elif (diffState == 'Skipped'):
        testcase.write("    <td>Skipped</td>\n")
    else:
        testcase.write("""    <td><a href="c_%s.png"><img name="c" border=0 src="c_%s.png"></img></a></td>\n"""%(case_name,case_name))
    testcase.write("  </tr>\n")
    testcase.write("  <tr>\n")
    testcase.write("    <td align=center rowspan=7>Diff Map:</td>\n")
    if (diffState == 'None'):
        testcase.write("    <td rowspan=7>No Differences</td>\n")
    elif (diffState == 'Unknown'):
        testcase.write("    <td rowspan=7>Not Available</td>\n")
    elif (diffState == 'Skipped'):
        testcase.write("    <td rowspan=7>Skipped</td>\n")
    else:
        testcase.write("""    <td><a href="" onMouseOver="document.d.src='b_%s.png'" onMouseOut="document.d.src='d_%s.png'"><img name="d" border=0 src="d_%s.png"></img></a></td>\n"""%(case_name,case_name,case_name))
    testcase.write("    <td align=center><i>Error Metric</i></td>\n")
    testcase.write("    <td align=center><i>Value</i></td>\n")
    testcase.write("  </tr>\n")
    testcase.write("      <tr><td>Total Pixels</td>  <td align=right>%06d</td></tr>\n"%tPixs)
    testcase.write("      <tr><td>Non-Background</td><td align=right>%06d</td></tr>\n"%pPixs)
    testcase.write("      <tr><td>Different</td>     <td align=right>%06d</td></tr>\n"%dPixs)
    testcase.write("      <tr><td>%% Diff. Pixels</td><td align=right>%f</td></tr>\n"%dpix)
    testcase.write("      <tr><td>Avg. Diff</td><td align=right>%f</td></tr>\n"%davg)
    testcase.write("      <tr></tr>\n")
    testcase.write("  </tr>\n")
    testcase.write("</table>\n")
    testcase.write("</html>\n")
    testcase.close()

# ----------------------------------------------------------------------------
# Function: GetBackgroundImage
#
# Modifications:
#    Kathleen Biagas, Wed Jan 15 09:40:09 MST 2014
#    Use alternate SaveWindowAttributes if available.
# ----------------------------------------------------------------------------
def GetBackgroundImage(file):
    """
    Returns the image of just VisIt's background without any plots
    """
    notHiddenList = []
    activeList = []

    plots = ListPlots(1)
    plotInfos = string.split(plots,"#")
    for entry in plotInfos:

        if entry == "":
            continue;

        plotParts = string.split(entry,"|")
        plotHeader = plotParts[0]
        plotInfo = plotParts[1]

        # get CLI's plot id for this plot
        plotId = string.split(plotHeader,"[")
        plotId = plotId[1]
        plotId = string.split(plotId,"]")
        plotId = int(plotId[0])

        # get this plot's active & hidden status
        words = string.split(plotInfo,";")
        hidden = -1
        active = -1
        for word in words:
            if word == "hidden=0":
                hidden = 0
            elif word == "hidden=1":
                hidden = 1
            elif word == "active=0":
                active = 0
            elif word == "active=1":
                active = 1

        if active == 1:
            activeList.append(plotId)

        # if this plot is NOT hidden, hide it
        if hidden == 0:
            SetActivePlots((plotId))
            HideActivePlots()
            notHiddenList.append(plotId)

    # ok, all non-hidden plots have been hidden. So,
    # now save the background image
    oldSWA = SaveWindowAttributes()
    tmpSWA = SaveWindowAttributes()
    if savedAltSWA == 0:
        tmpSWA.screenCapture = 1
    else:
        tmpSWA = savedAltSWA
    tmpSWA.family   = 0
    tmpSWA.fileName = out_path("current",file + "_bg.png")
    tmpSWA.format   = tmpSWA.PNG
    SetSaveWindowAttributes(tmpSWA)
    SaveWindow()
    bkimage = Image.open(tmpSWA.fileName)

    # ok, now restore everything to the way it was
    # before we got here
    SetSaveWindowAttributes(oldSWA)
    SetActivePlots(tuple(notHiddenList))
    HideActivePlots()
    SetActivePlots(tuple(activeList))

    return bkimage

# ----------------------------------------------------------------------------
# Function: GetReaderForFile
#
# Burlen Loring, Sat May 24 15:44:33 PDT 2014
# Create a reader to read an image file
#
# Modifications:
#
# ----------------------------------------------------------------------------

def GetReaderForFile(filename):
    """
    Given a filename return a VTK reader that can read it
    """
    r = vtkPNGReader()
    if not r.CanReadFile(filename):
        r = vtkPNMReader()
        if not r.CanReadFile(filename):
            r = vtkJPEGReader()
            if not r.CanReadFile(filename):
                r = vtkTIFFReader()
                if not r.CanReadFile(filename):
                    return None
    r.SetFileName(filename)
    return r

# ----------------------------------------------------------------------------
# Function: VTKImageDataToPILImage
#
# Burlen Loring, Sat May 24 15:44:33 PDT 2014
# In memory conversion from vtkImageData to PIL Image
#
# Modifications:
#
# ----------------------------------------------------------------------------

def VTKImageDataToPILImage(dataset):
    """
    Convert the image from VTK to PIL
    """
    def zeros(n):
      while n>0:
        yield 0
        n-=1

    def fmt(nc):
      if nc==1: return 'L'
      elif nc==3: return 'RGB'
      elif nc==4: return 'RGBA'
      else: return None

    dims = dataset.GetDimensions()[0:2]
    vim = dataset.GetPointData().GetArray(0)
    nc = vim.GetNumberOfComponents()
    nt = vim.GetNumberOfTuples()
    buf = array.array('B',zeros(nc*nt)) # TOOD -- Use numpy
    vim.ExportToVoidPointer(buf)
    pim = Image.frombuffer(fmt(nc),dims,buf,'raw',fmt(nc),0,1)
    pim = pim.transpose(Image.FLIP_TOP_BOTTOM)
    return pim

# ----------------------------------------------------------------------------
# Function: DiffUsingThreshold
# Burlen Loring, Sat May 24 15:44:33 PDT 2014
#
# Image difference using VTK's threshold algorithm. Stats are
# computed and diffs are enhanced using the same processing as
# is done in DiffUsingPIL for which this is a drop in replacement.
#
# Modifications:
#
# ----------------------------------------------------------------------------

def DiffUsingThreshold(case_name, cur, diff, base, altbase, overrides):
    """
    Computes difference and stats for the regression test using
    threshold based image difference algorithm.
    """
    acceptableError = 10.0
    if case_name in overrides:
        acceptableError = overrides[case_name]

    baseline = None
    baseReader = None
    baselines = [altbase, base]
    # check for existence of the baseline
    for f in baselines:
        if os.path.isfile(f):
            baseline = f
            break
    if baseline is None:
        Log('Warning: No baseline image: %s'%(base))
        if TestEnv.params["ctest"]:
            Log(ctestReportMissingBaseline(base))
        baselines = [test_module_path('nobaseline.pnm')]
    # check for the reader
    for f in baselines:
        baseReader = GetReaderForFile(f)
        if baseReader is not None:
            break
    if baseReader is None:
        Log('Warning: No reader for baseline image: %s'%(base))
    # read baseline
    baseReader.Update()
    baseDims = baseReader.GetOutput().GetDimensions()

    # read test image
    testReader = GetReaderForFile(cur)
    testReader.Update()
    testDims = testReader.GetOutput().GetDimensions()

    # resize baseline
    baseSource = baseReader
    if testDims != baseDims:
        Log('Error: Baseline%s and current%s images are different sizes.' \
            %(str(baseDims[0:2]),str(testDims[0:2])))
        baseSource = vtkImageResize()
        baseSource.SetInputConnection(baseReader.GetOutputPort())
        baseSource.SetOutputDimensions(testDims)

    # compute the diff
    differ = vtkImageDifference()
    differ.SetInputConnection(testReader.GetOutputPort())
    differ.SetImageConnection(baseSource.GetOutputPort())
    differ.Update()

    # test status
    error = differ.GetThresholdedError();
    testFailed = error > acceptableError
    diffState = 'Unacceptable' if testFailed else 'Acceptable'

    # create images/data for test suite's reporting
    mdiffimg = None
    dmin, dmax, dmean, dmedian, drms, dstddev = 0,0,0,0,0,0
    plotpixels, diffpixels, totpixels = 0,0,0

    oldimg = VTKImageDataToPILImage(baseReader.GetOutput())
    newimg = VTKImageDataToPILImage(testReader.GetOutput())
    diffimg = VTKImageDataToPILImage(differ.GetOutput())

    mdiffimg, dmin, dmax, dmean, dmedian, drms, dstddev, \
    plotpixels, diffpixels, totpixels \
        = ProcessDiffImage(case_name, oldimg, newimg, diffimg)

    mdiffimg.save(diff)
    #diffimg.save(diff)

    CreateImagesForWeb(case_name, testFailed, oldimg, newimg, mdiffimg)

    return diffState, error, (totpixels, plotpixels, diffpixels, dmean)

# ----------------------------------------------------------------------------
# Function: DiffUsingPIL
#
# Modifications:
#   Jeremy Meredith, Tue Jun  7 12:14:11 PDT 2005
#   Fixed error reporting for missing baseline images.
#
#   Mark C. Miller, Mon Jul  6 22:07:07 PDT 2009
#   Modified to instead of always generating thumb of current (new) image
#   and only of baseline (old) and diffs when there are diffs to do the
#   opposite. That is always generate thumb of baseline (old) image and
#   current and diffs when there are diffs.
#
#   Mark C. Miller, Tue Jul 20 19:27:09 PDT 2010
#   Left in (commented out) line for color image stats. Will use in a later
#   update to compute max channel difference.
#
#   Kathleen Biagas, Thu Feb  6 14:08:00 PST 2014
#   Only do ctest logging if ctest is enabled.
#
#   Burlen Loring, Mon May 26 13:39:37 PDT 2014
#   refactor generally useful code into two new functions: CreateImagesForWeb
#   and ProcessDiffImage
# ----------------------------------------------------------------------------

def DiffUsingPIL(case_name, cur, diff, baseline, altbase):
    """
    Diffs test results using PIL.
    """
    # open it using PIL Image
    newimg = Image.open(cur)
    size = newimg.size;

    # open the baseline image
    try:
        if (os.path.isfile(altbase)):
            oldimg = Image.open(altbase)
            if (size != oldimg.size):
                Log("Error: Baseline and current images are different sizes... resizing baseline to compensate")
                oldimg = oldimg.resize(size, Image.BICUBIC)
        elif (os.path.isfile(baseline)):
            oldimg = Image.open(baseline)
            if (size != oldimg.size):
                Log("Error: Baseline and current images are different sizes... resizing baseline to compensate")
                oldimg = oldimg.resize(size, Image.BICUBIC)
        else:
            Log("Warning: No baseline image: %s" % baseline)
            if TestEnv.params["ctest"]:
                Log(ctestReportMissingBaseline(baseline))
            oldimg = Image.open(test_module_path('nobaseline.pnm'))
            oldimg = oldimg.resize(size, Image.BICUBIC)
    except:
        oldimg = Image.open(test_module_path('nobaseline.pnm'))
        Log("Warning: Defective baseline image: %s" % baseline)
        if TestEnv.params["ctest"]:
            Log(ctestReportMissingBaseline(baseline))
        oldimg = oldimg.resize(size, Image.BICUBIC)


    # create the difference image
    try:
        diffimg = ImageChops.difference(oldimg, newimg)
    except:
        diffimg = oldimg
    #dstatc = ImageStat.Stat(diffimg) # stats of color image

    mdiffimg, dmin, dmax, dmean, dmedian, drms, dstddev, \
    plotpixels, diffpixels, totpixels \
        = ProcessDiffImage(case_name, oldimg, newimg, diffimg)

    mdiffimg.save(diff)

    CreateImagesForWeb(case_name, bool(dmax!=0), oldimg, newimg, mdiffimg)

    return (totpixels, plotpixels, diffpixels, dmean)


# ----------------------------------------------------------------------------
# Function: CreateImagesForWeb
#
# Burlen Loring, Mon May 26 09:45:21 PDT 2014
# Split this out of DiffUsingPIL so it can be used elsewhere.
#
# Modifications:
#
# ----------------------------------------------------------------------------

def CreateImagesForWeb(case_name, testFailed, baseimg, testimg, diffimg):
    """
    Given test image set create coresponding thumbnails for web
    consumption
    """
    thumbsize = (100,100)

    # full size baseline
    baseimg.save(out_path("html","b_%s.png"%case_name))
    # thumb size baseline
    oldthumb = baseimg.resize(   thumbsize, Image.BICUBIC)
    oldthumb.save(out_path("html","b_%s_thumb.png"%case_name))

    if (testFailed):
        # fullsize test image and diff
        testimg.save(out_path("html","c_%s.png"%case_name))
        diffimg.save(out_path("html","d_%s.png"%case_name))
        # thumbsize test and diff
        newthumb = testimg.resize(thumbsize, Image.BICUBIC)
        newthumb.save(out_path("html","c_%s_thumb.png"%case_name))

        diffthumb = diffimg.resize(thumbsize, Image.BICUBIC)
        diffthumb.save(out_path("html","d_%s_thumb.png"%case_name))

# ----------------------------------------------------------------------------
# Function: ProcessDiffImage
#
# Burlen Loring, Mon May 26 09:45:21 PDT 2014
# Split this out of DiffUsingPIL so it can be used elsewhere.
#
# Modifications:
#
# ----------------------------------------------------------------------------

def ProcessDiffImage(case_name, baseimg, testimg, diffimg):
    """
    Given a set of test images process (ie enhance) the difference image
    and compute various stats used in the report. Return the processed image
    and stats.
    """
    diffimg = diffimg.convert("L", (0.3333333, 0.3333333, 0.3333333, 0))

    dstat   = ImageStat.Stat(diffimg)
    dmin    = dstat.extrema[0][0]
    dmax    = dstat.extrema[0][1]
    dmean   = dstat.mean[0]
    dmedian = dstat.median[0]
    drms    = dstat.rms[0]
    dstddev = dstat.stddev[0]

    plotpixels = 0
    diffpixels = 0
    size = testimg.size
    totpixels = size[0] * size[1]

    mdiffimg = diffimg.copy()

    if (dmax > 0 and dmax != dmin):
        # brighten the difference image before we save it
        pmap = []
        pmap.append(0)
        for i in range(1,256): pmap.append(255)
        mdiffimg = mdiffimg.point(pmap)

        annotAtts = GetAnnotationAttributes()

        if (annotAtts.backgroundMode != 0 or
            annotAtts.backgroundColor != (255, 255, 255, 255)):

            # we have to be really smart if we don't have a constant color
            # background
            backimg = GetBackgroundImage(case_name)

            # now, scan over pixels in baseimg counting how many non-background
            # pixels there are and how many diffs there are
            for col in range(0,size[0]):
                for row in range(0, size[1]):
                    newpixel = testimg.getpixel((col,row))
                    oldpixel = baseimg.getpixel((col,row))
                    backpixel = backimg.getpixel((col,row))
                    diffpixel = mdiffimg.getpixel((col,row))
                    if oldpixel != backpixel:
                        plotpixels = plotpixels + 1
                    if diffpixel == 255:
                        diffpixels = diffpixels + 1
        else:
            # constant color background
            mdstat   = ImageStat.Stat(mdiffimg)
            baseimgm = baseimg.convert("L", (0.3333333, 0.3333333, 0.3333333, 0))
            map1 = []
            for i in range(0,254): map1.append(255)
            map1.append(0)
            map1.append(0)
            baseimgm = baseimgm.point(map1)
            mbstat   = ImageStat.Stat(baseimgm)
            diffpixels = int(mdstat.sum[0]/255)
            plotpixels = int(mbstat.sum[0]/255)

    return mdiffimg, dmin, dmax, dmean, dmedian, drms, dstddev, plotpixels, diffpixels, totpixels


# ----------------------------------------------------------------------------
# Function: FilterTestText
#
#
# Modifications:
#   Mark C. Miller, Tue Jan 29 18:57:45 PST 2008
#   Moved code to filter VISIT_TOP_DIR to top of routine to ensure it is
#   executed always, not just in the case where numdifftol is zero. I also
#   fixed cases where a floating point number occuring at the end of a
#   sentence ending in a period ('.') was not getting correctly interpreted
#   and filtered.
#
#   Mark C. Miller, Tue Jan 29 19:37:54 PST 2008
#   Adjusted case with the absolute value of the base values are very near
#   zero (e.g. less than square of numdifftol), to switch to absolute
#   diffs.
#
#   Mark C. Miller, Thu Jan 31 17:51:21 PST 2008
#   Modified the algorithm in a subtle way. Since the string.replace() calls
#   are applied repeatedly over the entire input string, it was possible for
#   an earlier replace to be corrupted by a later replace. The new algorithm
#   uses a two-step replace process. As numbers are found in the input string,
#   they are compared for magnitude of numerial difference to their counter
#   parts in the baseline string. If the difference is BELOW threshold, we aim
#   to replace the 'current' numerical value in inText with the baseline value
#   in baseText. This has the effect of preventing the numerical difference
#   from causing a REAL difference when the two strings are diff'd later on.
#   If the difference is NOT below threshold, we skip this replacement. That
#   has the effect of causing a REAL difference when the two strings are
#   diff'd later. When the replacement is performed (e.g. the numerical
#   difference is below threshold), we perform the replacement in two steps.
#   In the first pass over the string, we replace each current value with
#   a unique replacement 'tag.' The string we use must be unique over all
#   words in inText. In the second pass, we replace each of these replacement
#   tags with the actual baseline string thereby making that word in the
#   string identical to the baseline result and effectively eliminating it
#   from effecting the text difference.
#
#   Mark C. Miller, Tue Mar  4 18:35:45 PST 2008
#   Fixed some issues with the replace algorithm. Changed how near-zero
#   diffs are handled back to 'ordinary' relative diff. Made it more graceful
#   if it is unable to import PIL. Made text diff'ing proceed without PIL.
#
#   Mark C. Miller, Tue Mar  4 19:53:19 PST 2008
#   Discovered that string managment was taking a non-trivial portion of
#   total test time for text-oriented tests. So, found a better way to handle
#   the replacements using string slicing. Now, replacements are handled as
#   we march word-for-word through the strings.
#
#   Mark C. Miller, Thu Mar  6 09:39:43 PST 2008
#   Made logic for relative diff clearer. Switched to use min operation in
#   denominator and switch order of min/abs there.
#
#   Mark C. Miller, Tue Jun  9 09:23:30 PDT 2009
#   Removed refs to old Clearcase VOB paths. Fixed setting of 'inStart' for
#   string replacement to use inWords[w] for search rather than inWordT
#   which is potentially altered due to translate call and may not be found
#   in tmpText.
#
#   Cyrus Harrison, Tue Nov  6 13:08:56 PST 2012
#   Make sure to filter TestEnv.params["run_dir"] as well.
#
# ----------------------------------------------------------------------------

def FilterTestText(inText, baseText):
    """
    Filters words from the test text before it gets saved.
    """
    #
    # We have to filter out the absolute path information we might see in
    # this string. runtest passes the value for visitTopDir here.
    #
    inText = inText.replace(TestEnv.params["run_dir"], "VISIT_TOP_DIR/test")
    inText = inText.replace(out_path(), "VISIT_TOP_DIR/test")
    inText = inText.replace(test_root_path(), "VISIT_TOP_DIR/test")
    inText = inText.replace(data_path(), "VISIT_TOP_DIR/data")
    numdifftol = TestEnv.params["numdiff"]
    #
    # Only consider doing any string substitution if numerical diff threshold
    # is non-zero
    #
    if numdifftol != 0.0:

        tmpText = inText

        #
        # Break the strings into words. Pass over words looking for words that
        # form numbers. Whenever we have numbers, compute their difference
        # and compare it to threshold. If its above threshold, do nothing.
        # The strings will wind up triggering a text difference. If its below
        # threshold, eliminate the word from effecting text difference by
        # setting it identical to corresponding baseline word.
        #
        baseWords = string.split(baseText)
        inWords = string.split(tmpText)
        outText=""
        transTab = string.maketrans(string.digits, string.digits)
        inStart = 0
        for w in range(len(baseWords)):
            try:
                inWordT = string.translate(inWords[w], transTab, '><,()')
                baseWordT = string.translate(baseWords[w], transTab, '><,()')
                if inWordT.count(".") == 2 and inWordT.endswith(".") or \
                   baseWordT.count(".") == 2 and baseWordT.endswith("."):
                    inWordT = inWordT.rstrip(".")
                    baseWordT = baseWordT.rstrip(".")
                inStart = string.find(tmpText, inWords[w], inStart)

                #
                # Attempt to convert this word to a number. Exception indicates
                # it wasn't a number and we can move on to next word
                #
                inVal = string.atof(inWordT)
                baseVal = string.atof(baseWordT)

                #
                # Compute a relative difference measure for these two numbers
                # This logic was taken from www.math.utah.edu/~beebe/software/ndiff
                #
                if inVal == baseVal:
                    valDiff = 0
                elif inVal == 0 and baseVal != 0:
                    valDiff = numdifftol # treat as above threshold
                elif inVal != 0 and baseVal == 0:
                    valDiff = numdifftol # treat as above threshold
                else:
                    valDiff = abs(inVal - baseVal) / min(abs(inVal), abs(baseVal))

                #
                # We want to ignore diffs that are deemed below threshold given
                # the relative diff. measure above. To affect this, we need to
                # replace the numbers in the input text that differ with their
                # cooresponding numbers in the baseline text. This will have the
                # effect of making the HTML difference ignore this value.
                # So, we do this replace only if the diff is non-zero and less
                # than threshold.
                #
                if valDiff > 0 and valDiff < numdifftol:
                    tmpText = tmpText[:inStart] + baseWordT + tmpText[inStart+len(inWordT):]

                inStart = inStart + len(inWordT)

            #
            # This word wasn't a number, move on
            #
            except ValueError:
                # ignore exceptions
                pass

        return tmpText

    else:

        return inText

# ----------------------------------------------------------------------------
#  Method: CheckInteractive
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
def CheckInteractive(case_name):
    """
    Helper which pauses if we are in interactive mode.
    """
    # if interactive, pause for user
    if TestEnv.params["interactive"]:
        print "***********************"
        print "***********************"
        print "***********************"
        print "Saving %s"% case_name
        print "Hit Any Key To Continue"
        print "***********************"
        print "***********************"
        print "***********************"
        next = sys.stdin.read(1)

# ----------------------------------------------------------------------------
# Function: TestText
#
#
# Modifications:
#   Brad Whitlock, Tue Mar 30 16:39:43 PST 2004
#   Added code to sample memory.
#
#   Mark C. Miller, Tue May 25 14:29:40 PDT 2004
#   Added code to support interactive mode
#
#   Hank Childs, Sun Mar 27 14:34:30 PST 2005
#   Fix typo with testing mode specific baselines.
#
#   Jeremy Meredith, Tue Jun  7 12:09:12 PDT 2005
#   Added support for missing baseline text files again.
#
#   Brad Whitlock, Mon Nov 21 13:41:19 PST 2005
#   I made sure that it uses the mode-specific baseline if one exists.
#
#   Eric Brugger, Thu Apr 22 12:56:41 PDT 2010
#   I made several changes to the return code behavior of the script.  It
#   returns error code 119 if the test succeeded and the test had some skips.
#   It returns error code 120 if the test had acceptable differences and
#   had some skips.  It returns error code 113 if the differences were
#   unacceptable regardless of whether some tests were skipped.
#
#   Eric Brugger, Tue Apr 27 13:23:16 PDT 2010
#   I enhanced the routine so that the text next to the large baseline image
#   indicates if it is a mode specific image or not.
#
#   Cyrus Harrison, Tue Nov  6 13:08:56 PST 2012
#   Fix error code propagation logic.
#
#   Mark C. Miller, Wed Jul  9 18:43:41 PDT 2014
#   Added setFailed and setSkipped args for caller to control these
#
#   Mark C. Miller, Thu Jun 28 18:37:31 PDT 2018
#   Added logic to copy current file to "html" dir so it gets posted with
#   results so it can be more easily re-based later if needed.
# ----------------------------------------------------------------------------
def TestText(case_name, inText):
    """
    Write out text to file, diff it with the baseline, and log the result.
    """
    CheckInteractive(case_name)

    # create file names
    (cur, diff, base, altbase, modeSpecific) = GenFileNames(case_name, ".txt")

    if os.path.isfile(base):
        baseText = open(base).read()
    else:
        Log("Warning: No baseline text file: %s" % base)
        base = test_module_path("notext.txt")
        baseText = "notext"

    # Filter out unwanted text
    inText = FilterTestText(inText, baseText)

    # save the current text output
    fout = open(cur, 'w')
    fout.write(inText)
    fout.close()

    nchanges = 0
    nlines   = 0

    # diff the baseline and current text files
    d = HtmlDiff.Differencer(base, cur)
    # change to use difflib
    (nchanges, nlines) = d.Difference(out_path("html","%s.html"%case_name), case_name)

    # Copy the text file itself to "html" dir if there are diffs
    # to facilitate rebase, later if desired.
    if nchanges > 0:
        shutil.copy(cur, out_path("html", "c_%s.txt"%case_name))

    # save the diff output
    # TODO_WINDOWS THIS WONT WORK ON WINDOWS
    # we can use difflib
    diff_cmd = "diff " + base + " " + cur
    res = sexe(diff_cmd,ret_output = True)
    fout = open(diff, 'w')
    fout.write(res["output"])
    fout.close()

    # did the test fail?
    failed = (nchanges > 0)
    skip   =  TestEnv.check_skip(case_name)

    LogTextTestResult(case_name,nchanges,nlines,failed,skip)

    # Increment the number of skips if appropriate
    if skip:
        TestEnv.results["numskip"] += 1

    # set error codes
    if failed and not skip:
        TestEnv.results["maxds"] = max(TestEnv.results["maxds"], 2)


# ----------------------------------------------------------------------------
# Function: AssertTrue
#
#
# Modifications:
#
# ----------------------------------------------------------------------------
def AssertTrue(case_name,val):
    CheckInteractive(case_name)
    result = val == True
    skip = TestEnv.check_skip(case_name)
    if skip:
        TestEnv.results["numskip"] += 1
    if result == False and not skip:
        TestEnv.results["maxds"] = max(TestEnv.results["maxds"], 2)
    LogAssertTestResult(case_name,"True",result,val,skip)

# ----------------------------------------------------------------------------
# Function: AssertTrue
#
#
# Modifications:
#
# ----------------------------------------------------------------------------
def AssertFalse(case_name,val):
    CheckInteractive(case_name)
    result = val == False
    skip = TestEnv.check_skip(case_name)
    if skip:
        TestEnv.results["numskip"] += 1
    if result == False and not skip:
        TestEnv.results["maxds"] = max(TestEnv.results["maxds"], 2)
    LogAssertTestResult(case_name,"False",result,val,skip)

# ----------------------------------------------------------------------------
# Function: AssertEqual
#
#
# Modifications:
#
# ----------------------------------------------------------------------------
def AssertEqual(case_name,val_a,val_b):
    CheckInteractive(case_name)
    result = val_a == val_b
    skip = TestEnv.check_skip(case_name)
    if skip:
        TestEnv.results["numskip"] += 1
    if result == False and not skip:
        TestEnv.results["maxds"] = max(TestEnv.results["maxds"], 2)
    LogAssertTestResult(case_name,"Equal",result,
                        "%s == %s" % (str(val_a),str(val_b)),skip)

# ----------------------------------------------------------------------------
# Function: AssertGT
#
#
# Modifications:
#
# ----------------------------------------------------------------------------
def AssertGT(case_name,val_a,val_b):
    CheckInteractive(case_name)
    result = val_a > val_b
    skip = TestEnv.check_skip(case_name)
    if skip:
        TestEnv.results["numskip"] += 1
    if result == False and not skip:
        TestEnv.results["maxds"] = max(TestEnv.results["maxds"], 2)
    LogAssertTestResult(case_name,"Greater than",
                        result,"%s > %s" % (str(val_a),str(val_b)),skip)

# ----------------------------------------------------------------------------
# Function: AssertGTE
#
#
# Modifications:
#
# ----------------------------------------------------------------------------
def AssertGTE(case_name,val_a,val_b):
    CheckInteractive(case_name)
    result = val_a >= val_b
    skip = TestEnv.check_skip(case_name)
    if skip:
        TestEnv.results["numskip"] += 1
    if result == False and not skip:
        TestEnv.results["maxds"] = max(TestEnv.results["maxds"], 2)
    LogAssertTestResult(case_name,"Greater than or Equal",
                        result,"%s >= %s" % (str(val_a),str(val_b)),skip)

# ----------------------------------------------------------------------------
# Function: AssertLT
#
#
# Modifications:
#
# ----------------------------------------------------------------------------
def AssertLT(case_name,val_a,val_b):
    CheckInteractive(case_name)
    result = val_a < val_b
    skip = TestEnv.check_skip(case_name)
    if skip:
        TestEnv.results["numskip"] += 1
    if result == False and not skip:
        TestEnv.results["maxds"] = max(TestEnv.results["maxds"], 2)
    LogAssertTestResult(case_name,"Less than",
                        result,"%s < %s" % (str(val_a),str(val_b)),skip)

# ----------------------------------------------------------------------------
# Function: AssertLTE
#
#
# Modifications:
#
# ----------------------------------------------------------------------------
def AssertLTE(case_name,val_a,val_b):
    CheckInteractive(case_name)
    result = val_a <= val_b
    skip = TestEnv.check_skip(case_name)
    if skip:
        TestEnv.results["numskip"] += 1
    if result == False and not skip:
        TestEnv.results["maxds"] = max(TestEnv.results["maxds"], 2)
    LogAssertTestResult(case_name,"Less than or Equal",
                        result,"%s <= %s" % (str(val_a),str(val_b)),skip)

# ----------------------------------------------------------------------------
# Function: TestSection
#
# ----------------------------------------------------------------------------
def TestSection(sectionName):
    """
    Write a section header into the results table so it is easier to understand
    the results for a large test.
    """
    LogSectionStart(sectionName)

# ----------------------------------------------------------------------------
# Function: LogSection
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
def LogSectionStart(sectionName):
    """
    Log start of a test section, and create html entry.
    """
    Log("    BEGIN SECTION: %s" % sectionName)
    HTMLSectionStart(sectionName)
    JSONSectionStart(sectionName)

# ----------------------------------------------------------------------------
# Function: HTMLSectionStart
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
def HTMLSectionStart(sectionName):
    """
    Create html entry for the start of a test section.
    """
    html = html_output_file_handle()
    html.write(" <tr>\n")
    html.write("  <td colspan=6 align=center bgcolor=\"#0000ff\"><font color=\"#ffffff\"><b>%s</b></font></td>\n" % sectionName)
    html.write(" </tr>\n")

# ----------------------------------------------------------------------------
# Function: JSONSectionStart
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
def JSONSectionStart(section_name):
    """
    Create a new section in the json results.
    """
    res = json_results_load()
    s_res = {'name':   section_name,
             'cases':  []}
    res["sections"].append(s_res)
    json_results_save(res)


# ----------------------------------------------------------------------------
# Function: Exit
#
# Modifications:
#   Eric Brugger, Thu Apr 22 12:56:41 PDT 2010
#   I made several changes to the return code behavior of the script.  It
#   returns error code 119 if the test succeeded and the test had some skips.
#   It returns error code 120 if the test had acceptable differences and
#   had some skips.  It returns error code 113 if the differences were
#   unacceptable regardless of whether some tests were skipped.
# ----------------------------------------------------------------------------

def Exit(excode=0):
    """
    Exit with the appropriate error code.  Must be called at end of test cases.
    """
    # future mem tracking logic will need to clear engine cache:
    #ClearCacheForAllEngines()
    rcode = None
    if TestEnv.params["interactive"] == 0:
        if (excode):
            rcode = excode
    if rcode is None and TestEnv.results["maxds"] == 0:
        if TestEnv.results["numskip"] == 0:
            rcode = 111
        else:
            rcode = 119
    if rcode is None and TestEnv.results["maxds"] == 1:
        if TestEnv.results["numskip"] == 0:
            rcode = 112
        else:
            rcode = 120
    if rcode is None and TestEnv.results["maxds"] == 2:
        rcode = 113
    if rcode is None:
        rcode  = 114
    LogTestExit(rcode)
    # finalize results.
    open("returncode.txt","w").write("%d\n" % rcode)
    sys.exit(rcode)

# ----------------------------------------------------------------------------
# Function: RequiredPlotPlugin
#
# ----------------------------------------------------------------------------
def RequiredPlotPlugin(name):
    """
    Checks that the specified plot plugins are available and exits if not.
    """
    P = PlotPlugins()
    if type(name) is tuple or type(name) is list:
        for n in name:
            if n not in P:
                Exit(122)
    else:
        if name not in P:
            Exit(122)

# ----------------------------------------------------------------------------
# Function: RequiredOperatorPlugin
#
# ----------------------------------------------------------------------------
def RequiredOperatorPlugin(name):
    """
    Checks that the specified operator plugins are available and exits if not.
    """
    P = OperatorPlugins()
    if type(name) is tuple or type(name) is list:
        for n in name:
            if n not in P:
                Exit(123)
    else:
        if name not in P:
            Exit(123)

# ----------------------------------------------------------------------------
# Function: RequiredDatabasePlugin
#
# ----------------------------------------------------------------------------
def RequiredDatabasePlugin(name):
    """
    Checks that the specified database plugins are available and exits if not.
    """
    P = DatabasePlugins(TestEnv.params["data_host"])["plugins"]
    if type(name) is tuple or type(name) is list:
        for n in name:
            if n not in P:
                Exit(124)
    else:
        if name not in P:
            Exit(124)

# ----------------------------------------------------------------------------
# Function: TurnOnAllAnnotations
#
# ----------------------------------------------------------------------------
def TurnOnAllAnnotations(givenAtts=0):
    """
    Turns on all annotations.

    Either from the default instance of AnnotationAttributes,
    or using 'givenAtts'.
    """
    if (givenAtts == 0):
        a = AnnotationAttributes()
    else:
        a = givenAtts
    a.axes2D.visible = 1
    a.axes3D.visible = 1
    a.axes3D.triadFlag = 1
    a.axes3D.bboxFlag = 1
    a.userInfoFlag = 0
    a.databaseInfoFlag = 1
    a.legendInfoFlag = 1
    SetAnnotationAttributes(a)

# ----------------------------------------------------------------------------
# Function: TurnOffAllAnnotations
#
# ----------------------------------------------------------------------------
def TurnOffAllAnnotations(givenAtts=0):
    """
    Turns off all annotations.

    Either from the default instance of AnnotationAttributes,
    or using 'givenAtts'.
    """
    if (givenAtts == 0):
        a = AnnotationAttributes()
    else:
        a = givenAtts
    a.axes2D.visible = 0
    a.axes3D.visible = 0
    a.axes3D.triadFlag = 0
    a.axes3D.bboxFlag = 0
    a.userInfoFlag = 0
    a.databaseInfoFlag = 0
    a.legendInfoFlag = 0
    SetAnnotationAttributes(a)

# ----------------------------------------------------------------------------
# Function: CleanUpFilesToRemoveUponExit 
# Remove any files we created temporarily
# ----------------------------------------------------------------------------
def CleanUpFilesToRemoveUponExit():
    for f in filesToRemoveUponExit:
        RemoveFile(f)

# ----------------------------------------------------------------------------
# Function: DecompressDatabase
# Do this using python itself instead of relying upon having a gzip OS tool
# available and be judicious about memory usage.
# ----------------------------------------------------------------------------
def DecompressDatabase(abs_dbname, zip_ext):

    if os.path.isdir(os.environ['TMPDIR']):
         tmpdir = os.environ['TMPDIR']
    elif os.path.isdir(os.environ['TMP']):
         tmpdir = os.environ['TMP']
    elif os.path.isdir("/tmp"):
        tmpdir = "/tmp"
    else:
        tmpdir = os.path.dirname(abs_dbname)

    try:
        f = gzip.GzipFile(abs_dbname+zip_ext,"r")
        (gd, gname) = tempfile.mkstemp(suffix=os.path.basename(abs_dbname),
                          dir=tmpdir,text="wb")
        os.close(gd)
        g = open(gname,"wb")
        buf = f.read(100000)
        while len(buf):
            g.write(buf)
            buf = f.read(100000)
        g.close()
        filesToRemoveUponExit.append(gname)
    except:
        RemoveFile(gname)
        gname = ""
    return gname

# ----------------------------------------------------------------------------
# Function: FindAndOpenDatabase
#
# Modifications:
#   Mark C. Miller, Thu Mar 24 14:29:45 PDT 2016
#   Added support for gzip'd compressed variants of the file.
# ----------------------------------------------------------------------------
def FindAndOpenDatabase(dbname, extraPaths=()):
    """
    Searches in places where we're likely to find data files that we do NOT
    maintain in VisIt's repo. One needs to use the FindAndOpenDatabase()
    method in place of the OpenDatabase() method to invoke the behavior
    of searching in these dirs for the database file to open.
    """
    externalDbPaths=(data_path(),
                     "/project/projectdirs/visit/data",
                      "/usr/gapps/visit/data",
                      "/home/visit/data")
    for p in externalDbPaths + extraPaths:
        abs_dbname = "%s/%s"%(p,dbname)
        if os.path.isfile(abs_dbname):
            return OpenDatabase(abs_dbname), abs_dbname
        elif os.path.isfile(abs_dbname+".gz"):
            abs_dbname = DecompressDatabase(abs_dbname, ".gz")
            return OpenDatabase(abs_dbname), abs_dbname
    Log("Unable to OpenDatabase \"%s\" at any of the specified paths.\n" % dbname)
    return 0, ""

#############################################################################
#   Simulation Support
#############################################################################

def SimVisItDir():
    return TestEnv.params["sim_dir"]

def SimProgram(sim):
    if not sys.platform.startswith("win"):
        return os.path.join(SimVisItDir(), "tools","DataManualExamples","Simulations",sim)
    else:
        return os.path.join(SimVisItDir(), "tools","DataManualExamples","Simulations","%s.exe"%sim)

def SimFile(sim2):
    workingdir = os.curdir
    return os.path.abspath(os.path.join(workingdir, sim2))

def TestSimulation(sim, sim2):
    return Simulation(SimVisItDir(), SimProgram(sim), SimFile(sim2))

def TestParallelSimulation(sim, sim2, np):
    return Simulation(SimVisItDir(), SimProgram(sim), SimFile(sim2), np)

def TestBatchSimulation(sim):
    return Simulation(SimVisItDir(), SimProgram(sim), SimFile("batch"), batch=True)

# ----------------------------------------------------------------------------
#  Class: Simulation
#
#  Programmer: Brad Whitlock
#  Date:       Wed Dec 18, 2013
#
#  Modifications:
#    Brad Whitlock, Fri Jun 27 11:14:56 PDT 2014
#    Added some parallel launch code.    
#
#    Brad Whitlock, Thu Sep  4 15:58:13 PDT 2014
#    Added valgrind support for serial.
#
#    Kathleen Biagas, Thu Sep 4 16:43:27 MST 2014
#    Set close_fds to False for windows.
#
#    Eric Brugger, Wed Oct 29 12:18:07 PDT 2014
#    Changed the parallel job launching logic to use srun on surface instead
#    of edge.
#
#    Burlen Loring, Fri Oct  2 12:31:12 PDT 2015
#    Added gdb support for serial
#
#    Eric Brugger, Tue May 23 08:34:06 PDT 2017
#    I changed the parallel job launching logic to use srun on both surface
#    and cab instead of just surface.
#
#    Eric Brugger, Tue Sep 12 09:19:14 PDT 2017
#    I changed the parallel job launching logic to use srun on syrah in
#    addition to surface and cab.
#
#    Mark C. Miller, Fri Jun 29 08:51:36 PDT 2018
#    Replaced surface with pascal
#
# ----------------------------------------------------------------------------
class Simulation(object):
    def __init__(self, vdir, s, sim2, np=1, batch=False):
        self.simulation = s
        self.host = "localhost"
        self.sim2 = sim2
        self.visitdir = vdir
        self.p = None
        self.connected = False
        self.extraargs = []
        self.np = np
        self.valgrind = False
        self.gdb = False
        self.batch = batch

    def enablevalgrind(self):
        self.valgrind = True

    def startsim(self):
        """
        Start up the simulation.
        """
        # Close any compute engine that the test script launched already.
        CloseComputeEngine()
        # Start up the simulation.
        tfile = self.sim2 + ".trace"
        if self.batch:
            args = [self.simulation, "-dir", self.visitdir, "-trace", tfile]
        else:
            args = [self.simulation, "-dir", self.visitdir, "-trace", tfile, "-sim2", self.sim2]
        for a in self.extraargs:
            args = args + [a]
        if self.np > 1:
            # TODO: subclass MainLauncher, use internallauncher function to start
            # up the parallel engine using the host profile XML's for the current
            # host so we get the proper way of submitting parallel jobs.

            # For now...
            import socket
            if "pascal" in socket.gethostname() or \
               "cab" in socket.gethostname() or \
               "syrah" in socket.gethostname():
                do_submit = 0
                if do_submit:
                    msubscript = os.path.join(os.path.abspath(os.curdir), string.replace(self.sim2, "sim2", "msub"))
                    f = open(msubscript, "wt")
                    f.write("#!/bin/sh\n")
                    f.write("cd %s\n" % os.path.abspath(os.curdir))
                    f.write("ulimit -c 0\n")
                    f.write("srun -n %d" % self.np)
                    for a in args:
                        f.write(" %s" % a)
                    f.write("\n")
                    f.close()
                    args = ["msub", "-l", "nodes=1:ppn=12", msubscript]
                else:
                    args = ["srun", "-n", str(self.np)] + args
            else:
                args = ["mpiexec", "-n", str(self.np)] + args
        else:
            # Serial
            if self.valgrind:
                logfile = GenFileNames("valgrind", ".txt")[0]
                args = ["env", "GLIBCXX_FORCE_NEW=1", "valgrind", "--tool=memcheck", "--leak-check=full", "--log-file="+logfile] + args

            if self.gdb:
                args = ["xterm", "-geometry", "150x50", "-e", "gdb", "--args"] + args

        s="Running: "
        for a in args:
            s = s + a + " "
        print s
        if not sys.platform.startswith("win"):
            self.p = subprocess.Popen(args,
                                      stdin=subprocess.PIPE,
                                      stdout=subprocess.PIPE,
                                      stderr=subprocess.PIPE,
                                      close_fds=True)
        else:
            self.p = subprocess.Popen(args,
                                      stdin=subprocess.PIPE,
                                      stdout=subprocess.PIPE,
                                      stderr=subprocess.PIPE,
                                      close_fds=False)

        return self.p != None

    def addargument(self, arg):
        """
        Add an extra command line argument for the simulation."
        """
        self.extraargs = self.extraargs + [arg]

    def wait(self):
        if self.batch:
            return True
        for i in xrange(120): # Wait up to 2 minutes.
            try:
                s = os.stat(self.sim2)
                return True
            except:
                time.sleep(1)
        return False

    def connect(self):
        """
        Connect to the simulation."
        """
        print "Connecting to simulation ", self.simulation

        # The sim might not have started by the time we get here.
        # Wait for the sim2 file to exist.
        ret = False
        if self.wait():
            ret = OpenDatabase(self.sim2)
            if ret:
                self.connected = True
        return ret

    def disconnect(self):
        """
        Disconnect from the simulation.
        """
        self.connected = False
        DeleteAllPlots()
        CloseDatabase(self.sim2)
        CloseComputeEngine(self.host, self.sim2)
        return 1

    def endsim(self):
        """
        Tell the simulation to terminate."
        """
        havesim = False
        if self.batch:
            havesim = True
        elif self.connected:
            havesim = True
        # 'being nice' hangs on windows
        if not sys.platform.startswith("win") and havesim:
            # Be nice about it.
            self.p.stdin.write("quit\n")
            (o,e) = self.p.communicate()
            sys.stderr.write('sim stdout = %s\n'%(o))
            sys.stderr.write('sim stderr = %s\n'%(e))
            self.p.wait()
        else:
            # Force the sim to terminate.
            self.p.terminate()
        return 1

    # Sim commands
    def consolecommand(self, cmd):
        """
        Send a console command to the simulation."
        """
        if self.connected:
            self.p.stdin.write(cmd + "\n")
            self.p.stdin.flush()

    def controlcommand(self, cmd):
        """
        Send a control command to the simulation."
        """
        ret = 0
        if self.connected:
            ret = SendSimulationCommand(self.host, self.sim2, cmd)

    def metadata(self):
        md = None
        if self.connected:
            md = GetMetaData(self.sim2)
        return md

# ----------------------------------------------------------------------------
# Function: TestSimStartAndConnect
#
# ----------------------------------------------------------------------------

def TestSimStartAndConnect(testname, sim):
    # Test that the simulation executable exists.
    exe = os.path.split(sim.simulation)[1]
    started = 0
    connected = 0
    if os.path.exists(sim.simulation):
        txt = "Simulation executable \"%s\" exists.\n" % exe
        # Test that the simulation starts and that we can connect to it.
        started = sim.startsim()
        if started:
            txt = txt + "Simulation \"%s\" started.\n" % exe
            connected = sim.connect()
            if connected:
                txt = txt + "VisIt connected to simulation \"%s\"." % exe
            else:
                txt = txt + "VisIt did not connect to simulation \"%s\"." % exe
        else:
            txt = txt + "Simulation \"%s\" did not start." % exe
    else:
        txt = "Simulation executable \"%s\" does not exist.\n" % exe
    TestText(testname, txt)
    return started,connected

# ----------------------------------------------------------------------------
# Function: TestSimMetaData
#
# ----------------------------------------------------------------------------

def TestSimMetaData(testname, md):
    lines = string.split(str(md), "\n")
    txt = ""
    for line in lines:
        if "exprList" in line:
            continue
        outline = line
        if "simInfo.port" in line:
            outline = "simInfo.port = PORT"
        if "simInfo.securityKey" in line:
            outline = "simInfo.securityKey = KEY"
        if "simInfo.host" in line:
            outline = "simInfo.host = HOST"
        txt = txt + outline + "\n"
    TestText(testname, txt)

# ----------------------------------------------------------------------------
#  Class: SimulationMemoryRecorder
#
#  Programmer: Brad Whitlock
#  Date:       Thu Sep  4 15:57:33 PDT 2014
#
#  Modifications:
#
# ----------------------------------------------------------------------------
class SimulationMemoryRecorder(object):
    def __init__(self, filename):
        self.filename = filename
        self.samples = {}
        self.times = []
        self.iteration = 0

    def AddSample(self):
        #Query("Time")
        #self.times = self.times + [GetQueryOutputValue()]
        self.times = self.times + [self.iteration]
        self.iteration = self.iteration + 1

        engine = None
        engines = GetEngineList(1)
        for e in engines:
            if ".sim2" in e[1]:
                engine = e
                break;

        pa = GetProcessAttributes("engine", engine[0], engine[1])
        for i in range(len(pa.pids)):
            if self.samples.has_key(pa.pids[i]):
                self.samples[pa.pids[i]] = self.samples[pa.pids[i]] + [pa.memory[i]]
            else:
                self.samples[pa.pids[i]] = [pa.memory[i]]
        self.WriteFile()

    def WriteFile(self):
        f = open(self.filename, "wt")
        for k in self.samples.keys():
            f.write("#pid_%s\n" % str(k))
            for i in range(len(self.samples[k])):
                f.write("%g %g\n" % (self.times[i], self.samples[k][i]))
        f.close()

def TestExpressions(name, meshQuality=True, operatorCreated=False, prefix=""):
    exprList = Expressions()
    estr = prefix + "Expressions:\n"
    index = 1
    for expr in exprList:
        add = True
        if not meshQuality and "mesh_quality/" in expr[0]:
           add = False
        if not operatorCreated and "operators/" in expr[0]:
           add = False
        if add:
            estr = estr + "expression %d: %s = %s\n" % (index, expr[0], expr[1])
            index = index + 1
    TestText(name, estr)

#############################################################################
#   Argument/Environment Processing
#############################################################################

# ----------------------------------------------------------------------------
#  Class: TestEnv
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
#
#  Modifications:
#    Kathleen Biagas, Tue Jul 23 10:51:37 PDT 2013
#    Added class method "add_skip_case".
#
#    Kathleen Biagas, Thu Nov  8 10:28:37 PST 2018
#    Assume entire category is skipped if 'file' key is missing.
#
# ----------------------------------------------------------------------------
class TestEnv(object):
    """
    Class that holds high level environment options.

    Replaces old use of old global vars.
    """
    params  = {"interactive": 0,
               "use_pil":     True,
               "threshold_diff": False,
               "threshold_error": {},
               "avgdiff":     0.0,
               "pixdiff":     0.0,
               "numdiff":     0.0,
               "serial" :     True,
               "scalable":    False,
               "parallel":    False,
               "silo_mode":   "hdf5"}
    results = {"maxds":   0,
               "numskip": 0}
    @classmethod
    def setup(cls,params_file):
        tparams = json_load(params_file)
        cls.params.update(tparams)
        if not cls.params["skip_file"] is None and os.path.isfile(cls.params["skip_file"]):
            cls.skiplist = json_load(cls.params["skip_file"])
        else:
            cls.skiplist = None
        # parse modes for various possible modes
        for mode in string.split(cls.params["modes"],","):
            if mode == "scalable":
                cls.params["scalable"] = True
            if mode == "parallel":
                cls.params["parallel"] = True
                cls.params["serial"]   = False
            if mode == "pdb":
                cls.params["silo_mode"] = "pdb"
            if (cls.params["use_pil"] or cls.params["threshold_diff"]) and not pil_available:
                Log("WARNING: unable to import modules from PIL: %s" % str(pilImpErr))
                cls.params["use_pil"] = False
                cls.params["threshold_diff"] = False
            if (cls.params["threshold_diff"]) and not VTK_available:
                Log("WARNING: unable to import modules from VTK: %s" % str(vtkImpErr))
                cls.params["threshold_diff"] = False
        if cls.params["fuzzy_match"]:
            # default tols for scalable mode
            if cls.params["pixdiff"] < 2:
                cls.params["use_pil"] = 2
            if cls.params["avgdiff"] < 1:
                cls.params["avgdiff"] = 1
        cls.SILO_MODE = cls.params["silo_mode"]
    @classmethod
    def check_skip(cls,case_name):
        if cls.skiplist is None:
            return False
        # look for modes that match
        for v in cls.skiplist['skip_list']:
            if v['mode'] ==cls.params["modes"]:
                for test in v['tests']:
                    # check for platform restrictions
                    if test.has_key("platform"):
                        tplat = test["platform"].lower()
                        splat = sys.platform.lower()
                        # win,linux,osx
                        # ignore this entry if we are on the wrong platform
                        # else, use std logic
                        if not splat.startswith(tplat):
                            continue
                    if test['category'] == cls.params["category"]:
                        # see if the file matches
                        if test.has_key("file"):
                            if test['file'] == cls.params["file"]:
                                if not test.has_key("cases"):
                                    return True
                                else:
                                    if case_name in test['cases']:
                                        return True
                        else:
                            # whole category skipped
                            return True
        return False
    @classmethod
    def add_skip_case(cls,case_name):
        if cls.skiplist is None:
            return
        # look for modes that match
        for v in cls.skiplist['skip_list']:
            if v['mode'] ==cls.params["modes"]:
                for test in v['tests']:
                    # check for platform restrictions
                    if test.has_key("platform"):
                        tplat = test["platform"].lower()
                        splat = sys.platform.lower()
                        # win,linux,osx
                        # ignore this entry if we are on the wrong platform
                        # else, use std logic
                        if not splat.startswith(tplat):
                            continue
                    if test['category'] == cls.params["category"]:
                        # see if the file matches
                        if test['file'] == cls.params["file"]:
                            if not test.has_key("cases"):
                                test['cases'] = case_name
                                return
                            else:
                                if case_name in test['cases']:
                                    return
                                else: 
                                    test['cases'].append(case_name)
                                    return

def AddSkipCase(case_name):
    TestEnv.add_skip_case(case_name)

# ----------------------------------------------------------------------------
#  Class: InitTestEnv
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
#
#  Modifications:
#    Eric Brugger, Fri Aug 15 10:16:22 PDT 2014
#    I added the ability to specify the parallel launch method.
#
#    Brad Whitlock, Fri Dec 12 18:03:31 PST 2014
#    Use data_host for instead of "localhost" so we can do client/server to data.
# ----------------------------------------------------------------------------
def InitTestEnv():
    """
    Sets up VisIt to execute a test script.
    """
    atexit.register(CleanUpFilesToRemoveUponExit)

    # default file
    params_file="params.json"
    for arg in sys.argv:
        subargs = string.split(arg,"=")
        if (subargs[0] == "--params"):
            params_file = subargs[1]
    # main setup
    TestEnv.setup(params_file)
    # remove the user name
    annot = AnnotationAttributes()
    annot.userInfoFlag = 0
    SetDefaultAnnotationAttributes(annot)
    SetAnnotationAttributes(annot)
    # set scalable rendering mode if desired
    if TestEnv.params["scalable"]:
        ra = GetRenderingAttributes()
        ra.scalableActivationMode = ra.Always
        SetRenderingAttributes(ra)
    else:
        ra = GetRenderingAttributes()
        ra.scalableActivationMode = ra.Never
        SetRenderingAttributes(ra)

    # If we passed a directory to use for reading host profiles then let's 
    # use the host profiles to launch the engine (since it has settings we
    # probably want for the machine profile).
    if TestEnv.params["host_profile_dir"] != "":
        ReadHostProfilesFromDirectory(TestEnv.params["host_profile_dir"], 1)
        if TestEnv.params["data_host"] in GetMachineProfileNames():
            mp = GetMachineProfile(TestEnv.params["data_host"])
            # Make some modifications to the machine profile.
            if TestEnv.params["parallel_launch"] not in ("mpirun", "srun"):
                plaunch = string.split(TestEnv.params["parallel_launch"], " ")
                idx = 0
                try:
                    while idx < len(plaunch):
                        if plaunch[idx] == "-dir":
                            mp.directory = plaunch[idx+1]
                            idx = idx + 2
                        elif plaunch[idx] == "-b":
                            bank = plaunch[idx+1]
                            for i in range(mp.GetNumLaunchProfiles()):
                                mp.GetLaunchProfiles(i).bank = bank
                                mp.GetLaunchProfiles(i).bankSet = 1
                            idx = idx + 2
                        else:
                            idx = idx + 1
                except:
                    pass
            # Make sure that we select a parallel profile if we want to run
            # in parallel.
            if TestEnv.params["parallel"]:
                ap = -1
                for i in range(mp.GetNumLaunchProfiles()):
                    if mp.GetLaunchProfiles(i).parallel:
                        ap = i
                        break
                if ap != -1:
                    mp.activeProfile = ap   
            #print mp

            # Use the machine profile to start the engine.
            OpenComputeEngine(mp)
        else:
            Exit()
    else:
        # start parallel engine if parallel
        haveParallelEngine = True
        if TestEnv.params["parallel"]:
            if TestEnv.params["parallel_launch"] == "mpirun":
                haveParallelEngine = (OpenComputeEngine(TestEnv.params["data_host"], ("-np", "2")) == 1)
            elif TestEnv.params["parallel_launch"] == "srun":
                haveParallelEngine = (OpenComputeEngine(TestEnv.params["data_host"], ("-l", "srun", "-np", "2")) == 1)
        if haveParallelEngine == False:
            Exit()
        else:
            OpenComputeEngine(TestEnv.params["data_host"])

    # Automatically turn off all annotations
    # This is to prevent new tests getting committed that
    # are unnecessarily dependent on annotations.
    TurnOffAllAnnotations()
    # make sure the html dir exists (for compat w/ old runtest)
    if not os.path.isdir(out_path("html")):
        os.mkdir(out_path("html"))
    # colorize the source file, and write to an html file
    HtmlPython.ColorizePython(TestEnv.params["script"],
                              out_path(),
                              TestEnv.params["category"],
                              TestEnv.params["file"],
                              TestEnv.params["name"])
    LogTestStart()

InitTestEnv()

#keep as global var for now
SILO_MODE = TestEnv.SILO_MODE

#
# Run our test script using "Source"
#
visit.Source(TestEnv.params["script"])


