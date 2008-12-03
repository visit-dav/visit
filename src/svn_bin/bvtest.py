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

def sexe(cmd):
    """
    Execute a shell command.
    """
    sys.stderr.write("[exe:%s]\n" % cmd)
    os.system(cmd)

def current_time():
    """
    Helper to return a human readble string /w the current time.
    """
    return time.strftime("%Y-%m-%d %H:%M:%S")
    #time.asctime(time.gmtime())

class BuildResult(object):
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

class BuildTest(object):
    """
    Class executes build_visit tests and holds a set of results.
    """
    def __init__(self,output_dir,libcmds,bvexe,mode = "svn"):
        self.odir = os.path.abspath(output_dir)
        if self.odir[-1] == "/":
            self.odir = self.odir[:-1]
        self.libcmds = libcmds
        self.bvexe = bvexe
        self.mode = mode
        self.host = socket.gethostname()
        self.start_time = None
        self.end_time   = None
        self.results    = []
    
    def execute(self):
        self.start_time = current_time()
        self.results    = self.build_libs(self.libcmds)
        self.end_time   = current_time()
    
    def build_lib(self,libcmd):
        """
        Build a library using build_visit and return a BuildResult.
        """
        # make sure output dir exists!
        if self.odir != "":
            if not os.path.exists(self.odir):
                os.mkdir(self.odir)
        libs  = self.__parse_libcmd(libcmd)
        bvc   = self.__build_visit_command(libs);
        lib   = libs[-1]
        lfile = self.__log_file_name(lib)
        bresult = "failure"
        bstart = current_time()
        sexe(bvc)
        bend = current_time()
        if self.__check_build(lib):
            bresult = "success"
        return BuildResult(lib,libcmd,lfile,bstart,bend,bresult)
   
    def build_libs(self,libcmds):
        """
        Build a collection of libraries using build_visit and return a BuildResult for each.
        """
        return [ self.build_lib(libcmd) for libcmd in libcmds ]
        
            
    def result_xml(self):
        res  = '<?xml version="1.0"?>\n'
        res += '<?xml-stylesheet type="text/xsl" href="bvtest.xsl"?>'
        info  = 'host="%s" '  % self.host
        info += 'odir="%s" '  % self.odir
        info += 'libs="%s" '  % str(self.libcmds)
        info += 'start="%s" ' % self.start_time
        info += 'end="%s" '   % self.end_time
        res += "<bvtest %s >\n" % info
        res += "  <results>\n"
        for r in self.results:
            res += r.to_xml("  ")
        res +="  </results>\n"
        res +="</bvtest>\n"
        return res
        
    def __log_file_name(self,lib):
        """
        Construct the output log file name.
        """
        if self.odir != "":
            return  "%s/bv_log_%s.txt" % (self.odir,lib)
        else:
            return "bv_log_%s.txt" % (lib)
    
    def __build_visit_command(self,libs):
        """
        Construct the proper build_visit command.
        """
        options = "--console --no-visit --no-thirdparty --no-hostconf --stdout --makeflags -j4"
        if self.mode == "svn":
            options = "--svn HEAD " + options
        blibs=""
        for l in libs:
            blibs+="--%s " % l
        lib = libs[-1]
        if self.odir == "":
            cmd = "echo yes | %s %s %s >> %s"
            cmd = cmd % (self.bvexe,options,blibs,self.__log_file_name(lib))
        else:
            cmd = "echo yes | %s %s %s --thirdparty-path  %s >> %s"
            cmd = cmd % (self.bvexe,options,blibs,self.odir,self.__log_file_name(lib))
        return cmd

    def __check_build(self,lib):
        """
        Checks for sucessful build by examining the end of the build log.
        """
        try:
            lines = open(self.__log_file_name(lib)).readlines()
            nlines = len(lines)
            last = lines[nlines-1].strip()
            return last == "Finished!"
        except:
            return False

    
    def __parse_libcmd(self,vstr):
        """
        Parses two simple build command types:
            lib --> [lib]
            lib_3:lib_1,lib_2 -->  [lib_1,lib_2,lib_3]
        """
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
    odir    = sys.argv[1]
    libcmds = sys.argv[2:]
    bt = BuildTest(odir,libcmds,"./build_visit")
    bt.execute()
    print bt.result_xml()
    

if __name__ == "__main__":
    main()
    
