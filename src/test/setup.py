#!/usr/bin/env python
#
# file: setup.py
# author: Cyrus Harrison <cyrush@llnl.gov>
#
# distutils gen + setup script for the 'visit_testing' module.
#

import sys
import os
import shutil

from os.path import join  as pjoin
from distutils.core import setup


#
# Support run w/ visit's cli.
#
using_visit = False
try:
    # the command line string passed to cli
    # will confuse distutils, so modify
    # sys.argv to only have args passed after
    # '-s setup.py'
    args = Argv()
    sys.argv = [__file__]
    sys.argv.extend(args)
    using_visit = True
except:
    pass

# Note ideal, but we auto gen a src dir to make 
# a sane package from the script layout in the visit repo to use 
#with distutils
def generate():
    build_base = "py_src"
    # special case for VisIt build: 
    # find if --build-base  was passed by VisIt's CMake, if so
    # use it to generate the source dir
    #for arg in sys.argv:
    #    if arg.startswith("--build-base"):
    #        build_base = arg.split("=")[1]
    #        build_base = os.path.abspath(pjoin(build_base,"..","py_src"))
    if os.path.isdir(build_base):
        shutil.rmtree(build_base)
    os.mkdir(build_base)
    if using_visit:
        src_path = __visit_script_file__
    else:
        src_path = __file__
    src_dir = os.path.split(os.path.abspath(src_path))[0]
    print("[generating module source at: %s]" % build_base)
    module_items = ["HtmlDiff.py",
                    "HtmlPython.py",
                    "visit_test_common.py",
                    "visit_test_main.py",
                    "visit_test_reports.py",
                    "visit_test_suite.py",
                    "visit_test_ctest.py",
                    "nobaseline.pnm",
                    "notext.txt",
                    "report_templates"]
    for itm in module_items:
        itm_path = pjoin(src_dir,itm)
        print("[copying %s]" % itm_path)
        if os.path.isfile(itm_path):
            shutil.copy(itm_path,build_base)
        else:
            des_dir = pjoin(build_base,itm)
            if os.path.isdir(des_dir):
                shutil.rmtree(des_dir)
            shutil.copytree(itm_path,des_dir)
    init_src ="\n".join(["#",
               "# file: __init__.py",
               "# description: module init for `visit_testing'",
               "from visit_test_suite import run_visit_test, run_visit_tests",
               ""])
    open(pjoin(build_base,"__init__.py"),"w").write(init_src)


generate()

setup(name='visit_testing',
      version='0.1',
      author = 'VisIt Team',
      description='VisIt Testing Module',
      package_dir = {'visit_testing':'py_src'},
      packages=['visit_testing'],
      package_data={'visit_testing': ["nobaseline.pnm",
                                      "notext.txt",
                                      pjoin("report_templates","css","*"),
                                      pjoin("report_templates","js","*"),
                                      pjoin("report_templates","index.html")]})


if using_visit:
    sys.exit(0)