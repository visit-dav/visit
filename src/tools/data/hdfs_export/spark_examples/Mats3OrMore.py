import sys, os
from pyspark import SparkContext
from pyspark import SparkConf

#
# Decode hdfs_export mesh entity keys
#
KeyDigits = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ%#"
def AsciiKeyToIndex(key, keybase=None):
    if key is None:
        return 0
    kb = 0
    if keybase:
        kb = AsciiKeyToIndex(keybase)
    addone = 0;
    if key[0] == '-':
        addone = 1
    result = 0
    mult = pow(64,len(key)-1-addone)
    for i in range(len(key)-addone):
        result = result + mult * KeyDigits.index(key[i+addone])
        mult = mult / 64
    if addone:
        result = -result
    return int(result-kb)


#
# Count non-zero volfracs in zone
#
def count_mats(volfracs):
    nmats = 0
    for i in range(len(volfracs)):
        if (volfracs[i] > 0):
            nmats += 1
    return nmats
  
#
# Perform a query for all zones containing more than 2 matierals and then
# list the results. This query can be performed over any subset of the
# entire HDFS database. For example, to do all dbs, use "*" (a quoted
# star) as the dbfile-pattern arg.
#
if __name__ == "__main__":
  if len(sys.argv) < 2:
    print("Usage: Mats3OrMore <dbfile-pattern> ", file=sys.stderr)
    exit(-1)

  sc = SparkContext()
  material_data = sc.textFile("file:/home/training/vdbroot.silo/miller86/%s/*/*/*/materials.txt.bz2"%\
     (sys.argv[1]))

  # 29 seconds (over all Silo data in test suite) on VM
  material_counts = material_data.map(lambda line: (line[0:18], count_mats([float(v) for v in line[19:].split(',')]))).filter(lambda p: p[1]>2).collect()

  #
  # hard coded logic for decoding zone entity keys (can fix this with a utility method)
  #
  for zone in material_counts:
      userIdx = zone[0][0:2]
      f = os.popen("bzcat /home/training/vdbroot.silo/users.txt.bz2","r").readlines()
      for ul in f:
          u = ul.rstrip().split(',')
          if userIdx == u[0]:
              userStr = u[1]
      dbIdx = zone[0][2:5]
      f = os.popen("bzcat /home/training/vdbroot.silo/%s/dbs.txt.bz2"%userStr,"r").readlines()
      for dbl in f:
          db = dbl.rstrip().split(',')
          if dbIdx == db[0][2:]:
              dbStr = db[1]
      stateIdx = zone[0][5:7]
      f = os.popen("bzcat /home/training/vdbroot.silo/%s/%s/states.txt.bz2"%(userStr,dbStr),"r").readlines()
      for sl in f:
          s = sl.rstrip().split(',')
          if stateIdx == s[0][5:]:
              stateStr = s[1]
      meshIdx = zone[0][7:8]
      f = os.popen("bzcat /home/training/vdbroot.silo/%s/%s/%s/meshes.txt.bz2"%(userStr,dbStr,stateStr),"r").readlines()
      for ml in f:
          m = ml.rstrip().split(',')
          if meshIdx == m[0][7:]:
              meshStr = m[1]
      blockIdx = zone[0][8:12]
      f = os.popen("bzcat /home/training/vdbroot.silo/%s/%s/%s/%s/blocks.txt.bz2"%(userStr,dbStr,stateStr,meshStr),"r").readlines()
      for bl in f:
          b = bl.rstrip().split(',')
          if blockIdx == b[0][8:]:
              blockStr = b[1]
      zoneIdx = AsciiKeyToIndex(zone[0][12:]) / 8
      print("user=%s, db=%s, state=%s, mesh=%s, block=%s, zone=%d has %d materials"%(userStr,dbStr,stateStr,meshStr,blockStr,zoneIdx,zone[1]))
