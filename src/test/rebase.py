#!/usr/bin/env python

# run ./rebase.py --help for documentation
def usage():
    return \
"""
%prog args [test-file1 test-file2 ...]

where args specify the category, test .py filename, mode
and date tag (of the posted html results) and test-file1,
etc. are either the names or file globs (protected by
quotes as necessary) of tests to rebaseline. If no files
or file globs are specified then all results from the
specified test .py file are rebased.

Note that if you choose to re-baseline a whole series of
files which may include skips or actual passes, then it
will discover there are no *current* results posted for
those cases, issue a warning message and skip those cases.

Sometimes, its easiest to use rebase.py on a whole series
and then selectively svn revert the ones you didn't want to
rebase prior to committing them.

Warning messages are generated if the data copied from the
posted HTML results doesn't pass some basic sanity checks,
PNG formatted image, file size is within 25% of original.
These are only warnings and up to the user to decide whether
any action is required.

You can use file globbing for -c, -p args and test-file
options. But, be sure to single-quote them on the command line.
Also, you need to single-quote any string options that might
otherwise be interpreted by the shell such as the date tag
option.

If you run this script and discover it did a bunch of changes
you don't want, you can always run svn revert -R . to undu
everything this script changed or you can run svn revert
selectively to undo just some changes.

For mode-specific cases, sometimes a new mode-specific baseline
needs to be added. In that case, rebase.py may fail to do anything
or produce any output. This is becasue your working copy does not
have an instance of the previous mode-specific image and you must
create it manually. For example, if test operators/foo.py had
no mode-specific (parallel) sub-dir, but foo_02.png is now generating
a parallel-specific result, you would
    * mkdir operators/foo/parallel
    * touch operators/foo/parallel/foo_02.png
    * svn add operators/foo/parallel
and then do the rebase operation as normal. It will warn about file
size change because the touched file is empty and the rebased file
will be much larger. That is ok.


Examples...

To rebaseline *all* files from oldsilo test from date tag 2018-04-07-09:12
 
    ./rebase.py -c databases -p oldsilo -m serial -d '2018-04-07-09:12'

To rebaseline silo_00.png & silo_01.png files from oldsilo test from same tag 

    ./rebase.py -c databases -p oldsilo -m serial -d '2018-04-07-09:12' 'oldsilo_0[0-1].png'

To be interactively prompted upon each file to rebaseline from oldsilo test

    ./rebase.py -c databases -p oldsilo -m serial -d '2018-04-07-09:12' --prompt

To rebaseline all results for all test files from hybrid

    ./rebase.py -c hybrid -p '*.py' -m serial -d '2018-04-07-09:12'

To rebaseline literally everything except tecplot results

    ./rebase.py -c '*' -p '*.py' -m serial -d '2018-04-07-09:12'
    svn revert -R baseline/databases/tecplot
"""

import argparse
import glob
import imghdr
from optparse import OptionParser
import os
import sys

if (sys.version_info > (3, 0)):
    import urllib.request, urllib.parse, urllib.error
else:
    import urllib


def confirm_startup_dir_is_visit_test_root():
    retval = True
    if not os.getcwd().split('/')[-1] == 'test':
        print('"test" does not appear to be current working directory name.')
        retval = False
    if not os.access("baseline", os.F_OK):
        print('"baseline" does not appear to be present in the current working directory.')
        retval = False
    if not retval:
        print("Run this script only from the root of the VisIt test dir tree")
        sys.exit(1)
    return True

def parse_args():
    """
    Parses arguments to runtest.
    """
    parser = OptionParser(usage())
    parser.add_option("-c",
                      "--category",
                      dest="category",
                      help="[Required] Specify test category")
    parser.add_option("-p",
                      "--pyfile",
                      dest="pyfile",
                      help="[Required] Specify test py filename without the .py extension")
    parser.add_option("-m",
                      "--mode",
                      dest="mode",
                      default="serial",
                      help="[Required] Specify test mode")
    parser.add_option("-d",
                      "--datetag",
                      dest="datetag",
                      help="[Required] Specify the VisIt test result date tag (e.g. '2018-04-07-09:12') from which to draw new baselines")
    parser.add_option("-t",
                      "--type",
                      dest="type",
                      default="png",
                      help="[Optional] Specify test result type (e.g. 'png' or 'txt') [png]")
    parser.add_option("--prompt",
                      default=False,
                      dest="prompt",
                      action="store_true",
                      help="[Optional] Prompt before copying each file")
    opts, cases = parser.parse_args()
    return opts, cases

# Get list of baseline image names for this category and py file
def get_baseline_filenames(mode, cat, pyfile, test_type, cases):
    retval = []
    if cases:
        for patt in cases:
            if mode == "serial":
                retval += glob.glob("baseline/%s/%s/%s"%(cat,pyfile,patt))
            else:
                retval += glob.glob("baseline/%s/%s/%s/%s"%(cat,pyfile,mode,patt))
    else:
        if mode == "serial":
            retval = glob.glob("baseline/%s/%s/*.%s"%(cat,pyfile,test_type))
        else:
            retval = glob.glob("baseline/%s/%s/%s/*.%s"%(cat,pyfile,mode,test_type))

    newretval = []
    for f in retval:
        newretval.append(f.split('/')[1:])

    return newretval

# Iterate, getting current results from HTML server and putting
def copy_currents_from_html_pages(filelist, datetag, prompt, test_type):
    for ffields in filelist:
        cat = ffields[0]
        pyfile = ffields[1]
        if len(ffields) > 3:
            mode = ffields[2]
            f = ffields[3]
        else:
            mode = "serial"
            f = ffields[2]
        if prompt:
            docopy = input("Copy file \"%s\" (enter y/Y for yes)? "%f)
            if docopy != 'y' and docopy != 'Y':
                continue
        if mode == "serial":
            target_file = "baseline/%s/%s/%s"%(cat,pyfile,f)
        else:
            target_file = "baseline/%s/%s/%s/%s"%(cat,pyfile,mode,f)
        # As a sanity check, get current baseline image file size
        cursize = os.stat(target_file).st_size
        if (sys.version_info > (3, 0)):
            g = urllib.request.urlopen("http://portal.nersc.gov/project/visit/tests/%s/pascal_trunk_%s/c_%s"%(datetag,mode,f))
        else:
            g = urllib.urlopen("http://portal.nersc.gov/project/visit/tests/%s/pascal_trunk_%s/c_%s"%(datetag,mode,f))
        if 'Not Found' in g.read():
            print("*** Current \"%s\" not found. Skipping."%f)
        else:
            print("Copying file \"%s\""%f)
            if (sys.version_info > (3, 0)):
                urllib.request.urlretrieve("http://portal.nersc.gov/project/visit/tests/%s/pascal_trunk_%s/c_%s"%(datetag,mode,f),
                                           filename=target_file)
            else:
                urllib.urlretrieve("http://portal.nersc.gov/project/visit/tests/%s/pascal_trunk_%s/c_%s"%(datetag,mode,f),
                                   filename=target_file)
        # Do some simple sanity checks on the resulting file
        if test_type == 'png' and imghdr.what(target_file) != 'png':
            print("Warning: file \"%s\" is not PNG format!"%target_file)
        newsize = os.stat(target_file).st_size
        if newsize < (1-0.25)*cursize or newsize > (1+0.25)*cursize:
            print("Warning: dramatic change in size of file (old=%d/new=%d)\"%s\"!"%(cursize,newsize,target_file))

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
filelist = get_baseline_filenames(
    vopts['mode'],
    vopts['category'],
    vopts['pyfile'],
    vopts['type'],
    cases)

#
# Iterate, copying currents from HTML pages to baseline dir
#
copy_currents_from_html_pages(filelist,
    vopts['datetag'],
    vopts['prompt'],
    vopts['type'])
