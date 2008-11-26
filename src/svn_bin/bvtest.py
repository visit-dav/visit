#!/usr/bin/env python
# *****************************************************************************
#   Script: bvtest.py
#
#   Purpose:
#       Provide an easy (and cron-able) way to test the build_visit script.
#   Usage:
#       bvtest.py [output_dir] [lib1] <lib2> ... <libn>
#   
#   Results:
#       Outputs a xml summary of build results to stdout.
# 
# *****************************************************************************
# Current Functionality: 
# foreach lib invoke build_visit with the following options:
#  build_visit --console --no-visit --no-thirdparty --no-hostconf --stdout \ 
#          --[libname] --thirdparty-path  [output] > [output]/bv_log_[libname]
#  Check for "Finished!" at tail of output log
#
# Possible Future Functionality:
#  Cleanup after (each?) build.
#  Figure out $VISITARCH and version of each lib &check for lib installed
#
# *****************************************************************************

import sys,os,time,socket

class BuildResult:
    """
    Class that holds the results for an attemped build of a library.
    """
    def __init__(self,lib,cmd,log,build_start,build_end,result):
        self.lib = lib
        self.cmd = cmd
        self.log = log
        self.build_start = build_start
        self.build_end   = build_end
        self.result = result 
    def to_xml(self,space=""):
        res =  space + "<build_result>\n"
        res += space + space +"<lib>%s</lib>\n" % self.lib
        res += space + space +"<cmd>%s</cmd>\n" % self.cmd
        res += space + space +"<log>%s</log>\n" % self.log
        res += space + space +"<start>%s</start>\n" % self.build_start
        res += space + space +"<end>%s</end>\n"  % self.build_end
        res += space + space +"<result>%s</result>\n" % self.result
        res += space + "</build_result>"
        return res

def get_current_time():
    """
    Helper to return a human readble string /w the current time.
    """
    return time.asctime(time.gmtime())
        
def sexe(cmd):
    """
    Execute a shell command.
    """
    sys.stderr.write("[exe:%s]\n" % cmd)
    os.system(cmd)

def log_file(lib):
    """
    Construct the output log file name.
    """
    return "bv_log_%s.txt" % lib
    
def build_visit_command(libs,odir=""):
    """
    Construct the proper build_visit command.
    """
    options = "--console --svn HEAD --no-visit --no-thirdparty --no-hostconf --stdout --makeflags -j4"
    blibs=""
    for l in libs:
        blibs+="--%s " % l
    lib = libs[-1]
    if odir == "":
        cmd = "echo yes | ./build_visit %s %s >> %s"
        cmd = cmd % (options,blibs,log_file(lib))
    else:
        cmd = "echo yes | ./build_visit %s %s --thirdparty-path  %s >> %s"
        cmd = cmd % (options,blibs,odir,log_file(lib))
    return cmd

def check_build(lib,odir):
    """
    Checks for sucessful build by examining at the end of the build log.
    """
    try:
        lines = open(log_file(lib)).readlines()
        nlines = len(lines)
        last = lines[nlines-1].strip()
        return last == "Finished!"
    except:
        return False

def build(libcmd,odir):
    """
    Build a library using build_visit and return a BuildResult.
    """
    libs = parse_libcmd(libcmd)
    bvc = build_visit_command(libs,odir);
    lib = libs[-1]
    lfile = log_file(lib)
    bresult = "failure"
    bstart = get_current_time()
    sexe(bvc)
    bend = get_current_time()
    if check_build(lib,odir):
        bresult = "success"
    return BuildResult(lib,libcmd,lfile,bstart,bend,bresult)
   
def build_libs(libcmds,odir):
    """
    Build a collection of libraries using build_visit and return a BuildResult for each.
    """
    if odir != "":
        if not os.path.exists(odir):
            os.mkdir(odir)
    return [ build(libcmd,odir) for libcmd in libcmds ]

    
def parse_libcmd(vstr):
    # lib >>> [lib[
    # lib_3:lib_1,lib_2 >>> [lib_1,lib_2,lib_3]
    #
    res = []
    cidx = vstr.find(":")
    if cidx >0:
        pre  = vstr[cidx+1:].split(",")
        post = vstr[:cidx]
        for p in pre:
            res.append(p)
        res.append(post)
    else:
        res.append(vstr)
    return res
    
    
def main():
    """
    Main driver routine.
    """
    nargs = len(sys.argv)
    if nargs < 3:
        sys.stderr.write("usage: bvtest.py [output_dir] [lib_1] <lib_2> .... <lib_n>\n")
        sys.exit(1)
    odir = sys.argv[1]
    odir = os.path.abspath(odir)
    libcmds = sys.argv[2:]
    if odir[-1] == "/":
        odir = odir[:-1]
    stime = get_current_time()
    results = build_libs(libcmds,odir)
    etime = get_current_time()
    print '<?xml version="1.0"?>'
    print '<?xml-stylesheet type="text/xsl" href="bvtest.xsl"?>'
    host=socket.gethostname()
    info = 'host="%s" odir="%s" libs="%s" start="%s" end="%s"' % (host,str(odir),str(libcmds),stime,etime)
    print "<bvtest %s >" % info
    print "  <results>"
    for r in results:
        print r.to_xml("  ")
    print "  </results>"
    print "</bvtest>"
    

if __name__ == "__main__":
    main()
    
