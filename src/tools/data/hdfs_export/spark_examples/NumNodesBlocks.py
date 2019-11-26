import sys
from pyspark import SparkContext
from pyspark import SparkConf

#
# This is the basic logic necessary to decode hdfs_export's keys
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
# Parition based on the block field in a mesh entity key 
#
def ParitionByBlockKeyField(key):
    return AsciiKeyToIndex(key[8:12])

#
# Perform a NumNodes query but do it on a block by block basis returning for
# each block, a count of all nodes and a count of just ghost nodes
#
if __name__ == "__main__":
  if len(sys.argv) < 3:
    print("Usage: NumNodes <dbfile> <mesh>", file=sys.stderr)
    exit(-1)

  sc = SparkContext()
  coord_data = sc.textFile("file:/home/training/vdbroot.silo/miller86/%s/*/%s/*/coords.txt.bz2"%\
     (sys.argv[1],sys.argv[2]))

  nodes_per_block = coord_data.map(lambda line: line[8:12]).countByValue()
  ghost_nodes_per_block = coord_data.filter(lambda line: line[19]=='1').map(lambda line: line[8:12]).countByValue()

  for p in sorted(nodes_per_block):
    print("Block %d has %d nodes and %d ghost nodes"%(AsciiKeyToIndex(p),nodes_per_block[p],ghost_nodes_per_block[p]))
      
