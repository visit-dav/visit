# This is a simple example of a script that behaves "GrizIt-like" in that it
# reads text commands from a file, example.in, and then executes various
# VisIt python methods.
#
# The script is intended to be able to run stand-alone, like GrizIt would,
# and also within VisIt's test framework. Search for 'VisIt Test' in the
# code here and you will find the lines that are relevant to integrating
# this "Application" with the VisIt test framework.
#
# This is intended to be a template for helping GrizIt developers integrate
# GrizIt testing with VisIt's test harness
#
# The commands this simple example recognizes are
#     setroot <path>                       sets root prefix for databases
#     opendb <db-name>
#     closedb <db-name>
#     plot <plot-name> <var-name>             plots and saves to png file
#     query <plot-name> <query-name>       queries and saves to text file
#     exit
#
# The question this example answers is...if I have a .py file that does
# a bunch of work with VisIt including saving image and textual results,
# how do I have it run as if it were a VisIt test so it integrates with
# VisIt's test framework?
#

import sys

dbroot = ''
#
# Telltale signs this is running as a VisIt Test
#
def isRunningAsVisItTest():
    s = sorted(globals())
    if 'VisItException' not in s:
        return False
    if 'VisItInterrupt' not in s:
        return False
    if 'TestText' not in s:
        return False
    if 'TestSection' not in s:
        return False
    return True

def myOpenDatabase(db):
    OpenDatabase('%s/%s'%(dbroot,db))

def myCloseDatabase(db):
    CloseDatabase('%s/%s'%(dbroot,db))

def myPlot(plot,var):
    AddPlot(plot,var)
    DrawPlots()
    if plot == 'Curve': # we'll treat as text
       cdata = GetPlotInformation()['Curve'] 
       s = ''
       for i in range(0,len(cdata),2):
           s = s + '%f %f\n'%(cdata[i], cdata[i+1])
       with open('%s_%s_%s.txt'%(testName,plot,var), 'w') as txtfil:
           txtfil.write(s)
       if isRunningAsVisItTest():
           TestText('%s_%s_%s'%(testName,plot,var),s) # VisIt Test Text Output
    else:
        swa = SaveWindowAttributes()
        swa.family = 0
        swa.format = swa.PNG
        swa.fileName = '%s_%s_%s.png'%(testName,plot,var)
        SetSaveWindowAttributes(swa)
        SaveWindow()
        if isRunningAsVisItTest():
            Test('%s_%s_%s'%(testName,plot,var)) # VisIt Test Image Output
    DeleteAllPlots()

def myQuery(plot,var,queryList):
    AddPlot(plot,var)
    DrawPlots()
    s = ''
    for q in queryList:
        Query(q)
        s = s + GetQueryOutputString() + '\n'
    with open('%s_%s_%s.txt'%(testName,plot,var), 'w') as txtfil:
        txtfil.write(s)
    if isRunningAsVisItTest():
        TestText('%s_%s_%s'%(testName,plot,var),s) # VisIt Test Text Output
    DeleteAllPlots()

#
# Main "Application" loops and processes textual commands from a file
#
testName = TestEnv.params["file"][:-3] # prepends all result file names
with open(test_root_path('tests','grizit','%s.in'%testName),'r') as txtin:
    for line in txtin:
        words = line.split()
        if words[0] == 'opendb':
            myOpenDatabase(words[1])
        elif words[0] == 'closedb':
            myCloseDatabase(words[1])
        elif words[0] == 'plot':
            myPlot(words[1], words[2])
        elif words[0] == 'setroot':
            dbroot = words[1]
        elif words[0] == 'query':
            myQuery(words[1], words[2], words[3:])
        elif words[0] == 'exit':
            if isRunningAsVisItTest():
                Exit() # VisIt Test Exit Utility
            else:
                sys.exit(0)
        else:
            print 'Uncrecognized command, \'', words[0], '\''

if isRunningAsVisItTest():
    Exit() # VisIt Test Exit Utility

