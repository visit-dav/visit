# walks this directory structure storing contents in a dictionary
# then writing out to networks.json for use with the remote
# host profiles import

import os
import json

h = {"hosts" : [] } 
for x in os.listdir("."):
  if os.path.isdir(x):
      d = {'network' :os.path.basename(x)}
      d["files"] = []
      for y in os.listdir(x):
          d["files"].append({'name' : os.path.basename(y)})
      h["hosts"].append(d)
          
with open('networks.json', 'w') as f:
    json.dump(h, f, indent=2)
