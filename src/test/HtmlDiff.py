# ----------------------------------------------------------------------------
#       Code to difference two text files and output the result as HTML
#
#  Programmer: Jeremy Meredith
#  Date:       June  9, 2003
#
#  Note: The core logic was taken from Sean Ahern's xcompare tool
#        (then turned into python and refactored.....)
#
#  Modifications:
#    Brad Whitlock, Thu Mar 18 16:26:26 PST 2004
#    Added exception handling code so it does not crash if baselines are
#    missing.
#
#    Kathleen Biagas, Tue Feb 9 20201 
#    Removed ParseDiff, which used *nix 'diff -f', in favor of
#    difflib.SequenceMatcher and it's get_opcodes function.  This allows
#    text diffs to work on Windows.  While difflib has an HtmlDiff option for
#    creating html output, the html created here serves our purposes better,
#    hence the choice of SequenceMatcher and get_opcodes.
#
#    Cyrus Harrison, Wed Sep  7 11:34:36 PDT 2022
#    Work around for dual use (inside and outside of py module)
#
# ----------------------------------------------------------------------------

import os, string, cgi, difflib

try:
    from HtmlPython import LeadingSpaceToHtmlFormat
except:
    pass

try:
    from .HtmlPython import LeadingSpaceToHtmlFormat
except:
    pass

class TextFile:
    def __init__(self, fn, msg):
        self.fileName = fn
        self.opened = 0
        try:
            self.filedes = open(self.fileName, "rt")
            self.opened = 1
        except IOError:
            print("%s %s." % (msg, self.fileName))

    def close(self):
        if(self.opened == 1):
            self.filedes.close()

    def readline(self):
        retval = ""
        if(self.opened == 1):
            try:
                retval = self.filedes.readline()
            except IOError:
                retval = ""
        return retval
            
class TextList:
    def __init__(self, list):
        self.list = list 
        self.idx = 0

    def close(self):
        self.idx = 0
        self.list = None

    def readline(self):
        retval = ""
        if self.idx < len(self.list):
            retval = self.list[self.idx]
            self.idx += 1
        return retval

class Differencer:
    INSERT="#00ff88"
    DELETE="#ff8888"
    CHANGE="#ffff00"
    NOCOLOR=""
    oneSidedColor=1

    def __init__(self, f1, f2):
        self.fn1 = f1
        self.fn2 = f2
    def InitIO(self, filename, testname):
        if type(self.fn1) == type([]):
            self.in1 = TextList(self.fn1)
        else:
            self.in1 = TextFile(self.fn1, "Warning: No baseline text file: ")
        if type(self.fn2) == type([]):
            self.in2 = TextList(self.fn2)
        else:
            self.in2 = TextFile(self.fn2, "Warning: No current text file: ")
        self.nextleft  = self.in1.readline()
        self.nextright = self.in2.readline()
        self.out = open(filename, "wt")
        self.out.write("<html><head><title>Results for test case %s</title></head>\n"%testname)
        self.out.write("<body bgcolor=\"#a0b0f0\">\n")
        self.out.write("<h1>Results for test case <i>%s</i></h1>\n" % testname)
        self.out.write("<table border=\"2\">\n")
        self.out.write("<tr><td align=\"center\"><b><i>Legend</i></b></td></tr>\n")
        self.out.write("<tr><td bgcolor=\"%s\">%d Lines Added</td></tr>\n"%(self.INSERT,self.nAdded))
        self.out.write("<tr><td bgcolor=\"%s\">%d Lines Deleted</td></tr>\n"%(self.DELETE,self.nDeleted))
        self.out.write("<tr><td bgcolor=\"%s\">%d Lines Changed</td></tr>\n"%(self.CHANGE,self.nChanged))
        self.out.write("</table><br>\n")
        self.out.write("<h3>%d modifications, totalling %d lines<h3>\n"%(self.nModifications,self.nModifiedLines))
        
        self.out.write("<table width=\"100%\" border=\"2\" rules=\"groups\">\n")
        self.out.write("<colgroup></colgroup>\n")
        self.out.write("<colgroup></colgroup>\n")
        self.out.write("<colgroup></colgroup>\n")
        self.out.write("<colgroup></colgroup>\n")
        self.out.write("<thead>\n")
        self.out.write("<tr>\n")
        self.out.write("  <td align=\"center\"><i>%s</i></td>\n" % "&nbsp;Line&nbsp;")
        self.out.write("  <td align=\"center\"><b><i>%s</b></i></td>\n" % "Baseline")
        self.out.write("  <td align=\"center\"><i>%s</i></td>\n" % "&nbsp;Line&nbsp;")
        self.out.write("  <td align=\"center\"><b><i>%s</b></i></td>\n" % "Current")
        self.out.write("</tr>\n")
        self.out.write("</thead>\n")
        self.out.write("<tbody>\n")

    def FiniIO(self):
        self.out.write("</tbody>\n")
        self.out.write("</table>\n")
        self.out.close()
        self.in1.close()
        self.in2.close()

    def GetNextLeft(self):
        astr=cgi.escape(self.nextleft.rstrip())
        self.nextleft = self.in1.readline()
        self.leftline = self.leftline+1
        if astr=="":
            return "&nbsp;"
        else:
            return astr

    def GetNextRight(self):
        astr=cgi.escape(self.nextright.rstrip())
        self.nextright = self.in2.readline()
        self.rightline = self.rightline+1
        if astr=="":
            return "&nbsp;"
        else:
            return astr

    def WriteLineNumber(self, num=""):
        self.out.write("    <td align=\"right\"><b><code>%d:&nbsp;</code></b></td>\n"%num)

    def WriteLineText(self, color, text):

        #
        # Before writing the text, replace leading spaces with their
        # html equivalent.
        #
        html_text = LeadingSpaceToHtmlFormat(text)

        if color == "":
            self.out.write("    <td><code>%s</code></td>\n"%html_text)
        else:
            self.out.write("    <td bgcolor=\"%s\"><code>%s</code></td>\n"%(color,html_text))

    def WriteLeft(self,cl):
        l = self.GetNextLeft()
        self.WriteLineNumber(self.leftline)
        self.WriteLineText(cl, l)

    def WriteRight(self,cr):
        r = self.GetNextRight()
        self.WriteLineNumber(self.rightline)
        self.WriteLineText(cr, r)

    def WriteBlank(self,color):
        self.out.write("    <td></td>\n")
        if color == "" or self.oneSidedColor:
            self.out.write("    <td></td>\n")
        else:
            self.out.write("    <td bgcolor=\"%s\"></td>\n"%color)

    def InsertLine(self,cl, cr):
        self.out.write("<tr valign=\"top\">")
        self.WriteLeft(cl)
        self.WriteRight(cr)
        self.out.write("</tr>\n")

    def InsertLeft(self,cl):
        self.out.write("<tr valign=\"top\">")
        self.WriteLeft(cl)
        self.WriteBlank(cl)
        self.out.write("</tr>\n")

    def InsertRight(self,cr):
        self.out.write("<tr valign=\"top\">")
        self.WriteBlank(cr)
        self.WriteRight(cr)
        self.out.write("</tr>\n")

    def EOFReached(self):
        return (self.nextleft == "" or self.nextright == "")

    def Difference(self, filename, testname):

        self.nModifications = 0
        self.nModifiedLines = 0
        self.nChanged = 0
        self.nAdded   = 0
        self.nDeleted = 0
        self.leftline=0
        self.rightline=0

        # open ... as handles the close
        with open(self.fn1) as bf:
            blines = bf.readlines()
        with open(self.fn2) as cf:
            clines = cf.readlines()

        sm = difflib.SequenceMatcher(None, blines, clines)

        # get mod counts before calling InitIO 
        for opcode, b1, b2, c1, c2 in sm.get_opcodes():
            #print("%7s base[%d:%d] cur[%d:%d] "%(opcode, b1, b2, c1, c2))
            if opcode == 'delete':
                self.nModifications += 1
                self.nDeleted += (b2-b1)
            elif opcode == 'insert':
                self.nModifications += 1
                self.nAdded += (c2-c1)
            elif opcode == 'replace':
                self.nModifications += 1
                brange = b2-b1
                crange = c2-c1
                for i in range(min(brange,crange)):
                    self.nChanged += 1
                self.nChanged += abs(brange-crange) 

        self.nModifiedLines = self.nAdded + self.nChanged + self.nDeleted

        self.InitIO(filename, testname)

        for opcode, b1, b2, c1, c2 in sm.get_opcodes():
            if opcode == 'delete':
                for i in range(b2-b1):
                    self.InsertLeft(self.DELETE)
            elif opcode == 'insert':
                for i in range(c2-c1):
                    self.InsertRight(self.INSERT)
            elif opcode == 'replace':
                brange = b2-b1
                crange = c2-c1
                for i in range(min(brange,crange)):
                    self.InsertLine(self.CHANGE, self.CHANGE)
                if crange>brange:
                    for i in range(brange,crange):
                        self.InsertRight(self.CHANGE)
                else:
                    for i in range(crange,brange):
                        self.InsertLeft(self.CHANGE)
            else: # opcode == 'equal'
                for i in range(b2-b1):
                    self.InsertLine(self.NOCOLOR, self.NOCOLOR)

        self.FiniIO()
        
        return (self.nModifications, self.nModifiedLines)
