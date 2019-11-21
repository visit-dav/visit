
import os
import subprocess
from os.path import join as pjoin

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

py_files = []
# traverse root directory, and list directories as dirs and files as files
for root, dirs, files in os.walk("."):
    path = root.split(os.sep)
    path_str = os.sep.join(path)
    #print((len(path) - 1) * '---', os.path.basename(root))
    for file in files:
        if file.endswith(".py"):
            #print(len(path) * '---', file)
            py_files.append(pjoin(path_str,file))

for p in py_files:
    print(p)
    sexe("2to3 {}".format(p))
    

