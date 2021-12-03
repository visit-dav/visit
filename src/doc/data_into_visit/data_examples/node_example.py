import sys
sys.path.append("/usr/gapps/conduit/software/ascent/0.7.1/toss_3_x86_64_ib/openmp/gnu/conduit-install/lib/python3.7/site-packages")

import conduit

n = conduit.Node()
n["my"] = "data"
n["a/b/c"] = "d"
n["a"]["b"]["e"] = 64.0
print(n)

print("total bytes: %d" % n.total_strided_bytes())
