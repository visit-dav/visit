# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  embedded.py
#
#  Tests:      embedded viewer unit test
#
#  Mark C. Miller, Tue Jan 11 10:19:23 PST 2011
# ----------------------------------------------------------------------------
import os, subprocess

# Bogus call to TestText to create 'current' results dir
TestText("embedded1","PASSED")

# We cd to current results dir to ensure images get generated there
cdir = os.getcwd()
tdir = os.path.join(".","current","unit","embedded")
os.chdir(tdir)

tapp = os.path.join(visitTopDir,"src","exe","embedded")
tcmd = os.path.join(visitTopDir,"src","tools","embedviewer","commands2.txt")
subp = subprocess.Popen([tapp, tcmd], stderr=subprocess.PIPE, stdout=subprocess.PIPE)
(outdata,errdata) = subp.communicate()
subp.wait()

# Now, loop to Test the images.
diffResults = ""
os.chdir(cdir)
if usePIL:
    i = 0
    files = os.listdir(tdir)
    print files
    for f in files:
        print f
        if f == "visit%04d.jpeg"%i:
            i = i + 1
            tPixs = pPixs = dPixs = 0
            davg = 0.0
            cur = os.path.join(".","current","unit","embedded",f)
            diff = os.path.join(".","diff","unit","embedded",f)
            base = os.path.join(".","baseline","unit","embedded",f)
            print cur
            print diff
            print base
            (tPixs, pPixs, dPixs, davg) = DiffUsingPIL(file, cur, diff, base, "")
            result = "PASSED"
            if (dPixs > 0 and davg > 1):
                result = "FAILED, %f %f"%(dPixs,davg)
            diffResults += "%s:    %s\n"%(f,result)
    TestText("embedded2", diffResults)

Exit()
