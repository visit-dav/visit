#!/usr/bin/python
# (Small) Modification of some code from Cyrus which ensures we have a proper
# number of TRY/ENDTRY lines.
# There is some code here to search for invalid characters which isn't
# enabled/used right now.

import sys
import subprocess
import re

#
# Helper Methods
#

def parse_extension(file):
    "Returns the extension of a file name"
    if file.rfind(".") >=0:
        return file[file.rfind(".")+1:]

def subexe(cmd):
    "Executes a system command and returns the output"
    sys.stderr.write("cmd: " + cmd)
    return subprocess.Popen(cmd.split(), stdout=subprocess.PIPE).communicate()[0]

#
# svnlook wrapper class
#

class SvnLook(object):
    "Wraps svnlook useful commands"
    def __init__(self,repo_path,svnlook_path):
        self.repo = repo_path
        self.look = svnlook_path

    def changed_files(self,rev):
        """
        Returns files changed/added at a given rev.
        Supports both transaction and revision numbers.
        """
        cmd = self.look + " changed " + self.repo + " "
        if rev.find("-") >=0 :
            cmd += "--transaction "
        else:
            cmd += "--revision "
        cmd += rev
        lines = [ l.strip() for l in subexe(cmd).split("\n") ]
        files = []
        stats = []
        for l in lines:
            if len(l) >0 :
                tok = l.split();
                stats.append(tok[0])
                files.append(tok[1])
        return files, stats

    def file_contents(self,file, rev = None):
        """
        Returns the contents of a file at a given rev.
        Supports both transation and revision numbers. 
        """
        cmd = self.look + " cat " + self.repo + " " 
        if not rev is None:
            if rev.find("-") >=0 :
                cmd += "--transaction " + rev + " "
            else:
                cmd += "--revision " + rev + " "
        cmd += file
        return subexe(cmd)

#
# Tests
#

class SvnTest(object):
    "Test base class"
    def __init__(self,svnlook,rev):
        self.svnlook = svnlook
        self.rev = rev

    def execute(self,file):
        "Base test method"
        return False

class TestInvalidChars(SvnTest):
    "Test that checks for invalid characters in *.C files"
    def __init__(self,svnlook,rev):
        SvnTest.__init__(self,svnlook,rev)

    def execute(self,file):
        "Checks for tabs & windows newlines in *.C files"
        if not parse_extension(file) == "C":
            return True
        ok = True
        line = 0
        lines = self.svnlook.file_contents(file,self.rev).split("\n")
        for l in lines:
            if l.find("\t") >=0:
                sys.stderr.write("[%s line %d] error: found tab char\n" % (file,line))
                ok = False
            if l[-1:] == "\r":
                sys.stderr.write("[%s line %d] error: found windows newline\n" % (file,line))
                ok = False
            line +=1
        return ok

class TestExceptions(SvnTest):
    "Test that checks for proper try/endtry blocks in *.C files"
    def __init__(self,svnlook,rev):
        SvnTest.__init__(self,svnlook,rev)

    def execute(self,file):
        "Checks for proper try/endtry blocks in *.C files"
        if not parse_extension(file) == "C":
            return True
        # read file contents
        lines   = self.svnlook.file_contents(file,self.rev)
        # count TRYs & ENDTRYs
        trys    = len( [m.start() for m in re.finditer(re.escape("TRY"), lines)])
        endtrys = len( [m.start() for m in re.finditer(re.escape("ENDTRY"), lines)])
        # TRYs are counted with ENDTRYS, correct for this:
        trys   += -endtrys
        if trys == endtrys:
            return True
        else:
            sys.stderr.write("[%s] error: # of TRYs (%d) != ENDTRYs (%d)\n" %(file,trys,endtrys))
            return False

if __name__ == "__main__":
    # check for proper # of args
    if len(sys.argv) < 3:
        sys.stderr.write("usage: pre-commit [repo_path] [transaction_id]\n")
        sys.exit(-1)

    # get the repo path and transaction id
    repo_path = sys.argv[1]
    trans_id  = sys.argv[2]

    # create svnlook wrapper
    svnlook = SvnLook(repo_path,"/usr/bin/svnlook")
    # get changed files
    files, status = svnlook.changed_files(trans_id)

    ok = True
    for f in files:
        if not TestExceptions(svnlook, trans_id).execute(f):
            ok = False
    if not ok:
        sys.stderr.write("error: failed one or more svn pre-commit tests!")
        sys.exit(-1)
