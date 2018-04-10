#!/usr/bin/env python

#
#
# Python script to facilitate rebase-lining a large number of results
# from the posted HTML results
#
# Examples: This script must be run from the top of the test dir tree
#
# To rebaseline *all* files from oldsilo test from date tag 2018-04-07-09:12
#
#     ./rebase.py -c databases -p oldsilo -m serial -d '2018-04-07-09:12'
#
# To rebaseline silo_00.png & silo_01.png files from oldsilo test from same tag 
#
#     ./rebase.py -c databases -p oldsilo -m serial -d '2018-04-07-09:12' 'oldsilo_0[0-1].png'
#
# To interactively rebaseline files from oldsilo test
#
#     ./rebase.py -c databases -p oldsilo -m serial -d '2018-04-07-09:12' --prompt
#
# -c arg is the category
# -p arg is the pyfilename without .py extension
# -d arg is the date tag of the test results from which to draw images
# -m is the mode of the test results from which to draw images
#
# Note: This will NOT HANDLE rebaselining of files in mode-specific baseline dirs
#

import argparse
import glob
from optparse import OptionParser
import os
import urllib
import sys

def confirm_startup_dir_is_visit_test_root():
    retval = True
    if not os.getcwd().split('/')[-1] == 'test':
        print '"test" does not appear to be current working directory name.'
        retval = False
    if not os.access(".svn", os.F_OK):
        print '".svn" does not appear to be present in the current working directory.'
        retval = False
    if not os.access("baseline", os.F_OK):
        print '"baseline" does not appear to be present in the current working directory.'
        retval = False
    if not os.access("visit_test_suite.py", os.F_OK):
        print '"visit_test_suite.py" does not appear to be present in the current working directory.'
        retval = False
    if not retval:
        print "Run this script only from the root of the VisIt test dir tree"
        sys.exit(1)
    return True

def parse_args():
    """
    Parses arguments to runtest.
    """
    parser = OptionParser()
    parser.add_option("-c",
                      "--category",
                      dest="category",
                      help="Specify test category")
    parser.add_option("-p",
                      "--pyfile",
                      dest="pyfile",
                      help="Specify test py filename without the .py extension")
    parser.add_option("-m",
                      "--mode",
                      dest="mode",
                      default="serial",
                      help="Specify test mode")
    parser.add_option("-d",
                      "--datetag",
                      dest="datetag",
                      help="Specify the VisIt test result date tag (e.g. '2018-04-07-09:12') from which to draw new baselines")
    parser.add_option("--prompt",
                      default=False,
                      dest="prompt",
                      action="store_true",
                      help="Prompt before copying each file")
    opts, cases = parser.parse_args()
    return opts, cases

# Get list of baseline image names for this category and py file
def get_baseline_image_filenames(cat, pyfile, cases):
    retval = []
    if cases:
        for patt in cases:
            retval += glob.glob("baseline/%s/%s/%s"%(cat,pyfile,patt))
    else:
        retval = glob.glob("baseline/%s/%s/*.png"%(cat,pyfile))

    newretval = []
    for f in retval:
        newretval.append(f.split('/')[-1])

    return newretval

# Iterate, getting current PNGs from HTML server and putting
def copy_currents_from_html_pages(filelist, cat, pyfile, mode, datetag, prompt):
    for f in filelist:
        if prompt:
            docopy = raw_input("Copy file \"%s\" (enter y/Y for yes)? "%f)
            if docopy != 'y' and docopy != 'Y':
                continue
        print "Copying file \"%s\""%f
        g = urllib.urlopen("http://portal.nersc.gov/project/visit/tests/%s/surface_trunk_%s/c_%s"%(datetag,mode,f))
        if 'Not Found' in g.read():
            print "*** Current \"%s\" not found. Using base."%f
            urllib.urlretrieve("http://portal.nersc.gov/project/visit/tests/%s/surface_trunk_%s/b_%s"%(datetag,mode,f),
                filename="baseline/%s/%s/%s"%(cat,pyfile,f))
        else:
            urllib.urlretrieve("http://portal.nersc.gov/project/visit/tests/%s/surface_trunk_%s/c_%s"%(datetag,mode,f),
                filename="baseline/%s/%s/%s"%(cat,pyfile,f))

#
# Confirm in correct dir
#
confirm_startup_dir_is_visit_test_root()

#
# Read the command line
#
opts, cases = parse_args()
vopts = vars(opts)

#
# Get list of images to re-base
#
imagelist = get_baseline_image_filenames(vopts['category'], vopts['pyfile'], cases)

#
# Iterate, copying currents from HTML pages to baseline dir
#
copy_currents_from_html_pages(imagelist,
    vopts['category'],
    vopts['pyfile'],
    vopts['mode'],
    vopts['datetag'],
    vopts['prompt'])
