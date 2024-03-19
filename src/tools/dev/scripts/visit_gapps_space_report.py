#
# LLNL /usr/gapps/visit space report script
#

import glob
import subprocess
import json
import os

def sexe(cmd,ret_output=False,echo = True):
    """ Helper for executing shell commands. """
    if echo:
        print("[exe: {}]".format(cmd))
    if ret_output:
        p = subprocess.Popen(cmd,
                             shell=True,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.STDOUT)
        res = p.communicate()[0]
        res = res.decode('utf8')
        return p.returncode,res
    else:
        return subprocess.call(cmd,shell=True)

total_bytes = 0
vals = {}
vals["_total_bytes"] = 0
fs = glob.glob("/usr/gapps/visit/*")
for f in fs:
    rc,rv = sexe("du -b {0}".format(f),ret_output=True)
    lines = [ l.strip() for l in rv.split("\n") if  l.strip() != ""]
    value = lines[-1]
    f_key = os.path.split(f)[1]
    bytes = int(value.split("\t")[0])
    vals[f] = bytes
    vals["_total_bytes"] += bytes

print("Bytes used")
print(json.dumps(vals,indent=2))
print("Total MB: {0}".format(float(vals["_total_bytes"] / 1e+6)))
print("Total GB: {0}".format(float(vals["_total_bytes"] / 1e+9)))
print("Quota info")
sexe("df -h /usr/gapps/visit")
