#!/usr/bin/env python

"""
File:    doxygenate.py
Author:  Cyrus Harrison
Created: February 24, 2008
Purpose: Creates a doxygented copy of input source tree and an input Doxyfile.

Usage:   doxygenate [src_dir] [des_dir]
"""

import sys
import os
import glob
import shutil
import subprocess
import re

from DocBlock import *

def check_if_excluded(path):
    """
    Check if path is one we know we dont care about.
    """
    exclusions= [ "src/ac",
                  "src/bin",
                  "src/archives",
                  "src/config-site",
                  "src/cqscore",
                  "src/exe",
                  "src/help",
                  "src/include",
                  "src/lib",
                  "src/plugins",
                  "src/sim",
                  "src/tools",
                  "src/third_party_builtin",
                  "src/java",
                  "src/svn_bin",
                  "src/visitpy",
                  ".svn"]
    for e in exclusions:
        if path.find("/"+e+"/") >=0:
            return True
    return False

def extract_group_tag(path):
    """
    Try to extract a sensible group name from the path 
    """
    tag = None
    tags = [ "avt",
             "common",
             "engine",
             "gui",
             "mdserver",
             "launcher",
             "viewer",
             "databases",
             "operators",
             "plots"]
    for t in tags:
        if path.find("/"+t+"/") >= 0:
            tag = t
            break
    return tag

def extract_doc_blocks(fname):
    """
    Opens a source file and extracts all doc blocks.
    """
    blocks = []
    lopen  = -1
    lc = 0;
    gtag = extract_group_tag(fname)
    f = open(fname)
    lines = f.readlines()
    f.close()
    for l in lines:
        if l.find("// ************") == 0:
            if lopen > -1:
                block_text = ""
                for l in lines[lopen:lc+1]:
                    block_text += l
                db = DocBlock(block_text,lopen,lc)
                db.set_group_tag(gtag)
                blocks.append(db)
                lopen = -1
            else:
                lopen = lc
        lc +=1
    return blocks

def doxygenate_file(fin,fout):
    """
    Extracts all doc blocks from input and creates a doxygenated output.
    """
    input  = open(fin)
    input_lines = input.readlines()
    input.close()
    # get all doc blocks for the file
    blocks = extract_doc_blocks(fin)
    bidx = 0
    more_blocks = bidx < len(blocks)
    lidx = 0
    # create "doxygenated" output
    output = open(fout,"w")
    for l in input_lines:
        if more_blocks and lidx == blocks[bidx].lclose + 1:
            output.write(blocks[bidx].doxygenate())
            bidx +=1
            more_blocks = bidx < len(blocks)
        output.write(l)
        lidx +=1       

def doxygenate_source_tree(src,des):
    """
    Creates a doxygenated copy of src in des.
    """
    if src[-1] != "/": src +="/"
    # create dir in des with same base name as src in des
    src_base = os.path.split(src[:-1])[1] + "/"
    des_sub  = des + src_base
    # if the des sub dir exists we want to remove it
    if os.path.exists(des_sub):
        shutil.rmtree(des_sub)
    # create the des sub dir
    print "[Creating Destination Sub Directory: %s]" % des_sub
    os.mkdir(des_sub)
    # extract *{h,C} files from src, and doxygenate into des
    files = glob.glob(src + "*.h")
    files.extend(glob.glob(src + "*.C"))
    for fsrc in files:
        dn,fn = os.path.split(fsrc)
        fdes = des_sub + fn; 
        print "[Doxifing: %s to %s]" % (fsrc,fdes)
        doxygenate_file(fsrc,fdes)
    # Decend to subdirs
    dirs = [ d +"/" for d in glob.glob(src + "*") if os.path.isdir(d)]
    dirs.sort()
    for d in dirs:
        if check_if_excluded(d): 
            print "[Skipping Excluded Path: %s]" % d
        else:
            print "[Decending into: %s]" % d
            doxygenate_source_tree(d,des_sub)

def prepare_directories(src,des):
    # make src directory exists
    if not os.path.exists(src):
        print "<Error>: Source Directory: %s does not exist!" % src
        sys.exit(-1)
    # make sure it is a directory
    if not os.path.isdir(src):
        print "<Error>: Source: %s is not a directory!" % src
        sys.exit(-1)
    # if the des directory does not exist create it 
    if not os.path.exists(des):
        print "[Creating Destination Directory: %s]" % des
        os.mkdir(des)
    elif not os.path.isdir(des): # check if it exists but is not a directory
        print "<Error>: Destination: %s exists, but is not a directory!" % des
        sys.exit(-1)
    # make sure we have trailing "/" on both paths
    if src[-1] != "/": src +="/"
    if des[-1] != "/": des +="/"
    # at this point we are sure both exist and are directories, make sure
    # they are not the same directory 
    if os.path.abspath(src) == os.path.abspath(des):
        print "<Error>: Source Directory == Destination Directory, ",
        print "Refusing to destroy source tree!"
        sys.exit(-1)
    return src, des

   
def get_visit_version(src):
    """
    Attempts to get the VisIt Version number from the source tree.
    """
    version = "<Unknown>"
    if os.path.isfile(src +"VERSION"):
        f = open(src +"VERSION")
        version = f.readline().strip()
        f.close()    
    return version
    
def get_visit_svn_rev(src):
    """
    Attempts to get the VisIt SVN Reversion number from the source tree.
    """
    svnrev = "<Unknown>"
    proc = subprocess.Popen('svn info %s' % src, shell=True,stdout=subprocess.PIPE)
    result = proc.communicate()[0]
    res = re.compile(r'Revision:\s[0-9]+\s').search(result)
    if not res is None:
        svnrev = result[res.start():res.end()].split()[1].strip()
    return svnrev
            
def prepare_doxyfile(src,des):
    """
    Creates a Doxyfile from skeleton Doxyfile.in
    Note: This currently assumes doxygenate.py is run in 
    the src/tools/doxygenate dir 
    (so it can obtain Doxyfile.in and Index.doxy)
    """
    f = open("Doxyfile.in")
    data = f.read()
    f.close()
    rpairs =[ ["$$VERSION$$",get_visit_version(src)],
              ["$$SVN_REV$$",get_visit_svn_rev(src)],
              ["$$DOXY_DIR$$",des ],
              ["$$OUTPUT_DIR$$",des + "doxyout/"]]
    for p in rpairs:
        data = data.replace(p[0],p[1])
    f = open("Doxyfile","w")
    f.write(data)
    f.close()
    shutil.copyfile("Index.doxy",des + "Index.doxy")
            
if __name__ == "__main__":
    # if des dir already exists, prompt user to delete it
    if len(sys.argv) < 3:
        print "usage: doxygenate [src_dir] [des_dir]"
        sys.exit(-1)
    try:
        src,des = prepare_directories(sys.argv[1],sys.argv[2])
        doxygenate_source_tree(src,des)
        prepare_doxyfile(src,des)
    except KeyboardInterrupt:
        print "<Error>: Stopped by user"

