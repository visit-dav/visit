import sys
import os
from pyspark import SparkContext
from pyspark import SparkConf

#
# Think these all need to be broadcast vars to work. Maybe
# put in a dict and broadcast
#
VTK_EMPTY_CELL     = 0
VTK_VERTEX         = 1
VTK_POLY_VERTEX    = 2
VTK_LINE           = 3
VTK_POLY_LINE      = 4
VTK_TRIANGLE       = 5
VTK_TRIANGLE_STRIP = 6
VTK_POLYGON        = 7
VTK_PIXEL          = 8
VTK_QUAD           = 9
VTK_TETRA          = 10
VTK_VOXEL          = 11
VTK_HEXAHEDRON     = 12
VTK_WEDGE          = 13
VTK_PYRAMID        = 14

#
# Logic necessary to decode mesh entity keys produced by hdfs_export utils
#
KeyDigits = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ%#"
KeyTable = [-1 for i in range(256)]
for i in range(len(KeyDigits)):
    KeyTable[ord(KeyDigits[i])] = i

#
# Convert mesh element index to key
#
def IndexToAsciiKey(keybase, idx, max_chars):
    negate = False
    if idx < 0:
        negate = True
        idx = -idx
    nchars = 0  
    result = ""
    while True:
        if (idx < 64):
            result = KeyDigits[idx] + result
            nchars += 1
            while nchars < max_chars:
                result = '0' + result
                nchars += 1
            break
        s = idx % 64
        result = KeyDigits[s] + result
        nchars += 1
        if nchars == max_chars:
            return None
        idx = idx / 64
    if negate:
        result = '-' + result
    return keybase + result

#
# Convert mesh element key to Python int index
#
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
# Compare two keys without resorting to computing their
# Python integer equivs because this is faster
#
def AsciiKeyCmp(keyA, keyB):
    keyA = keyA.lstrip('0')
    keyB = keyB.lstrip('0')
    lenA = len(keyA)
    lenB = len(keyB)
    if lenA != lenB:
        if lenA < lenB:
            return -1
        else:
            return 1
    for i in range(lenA):
        if keyA[i] != keyB[i]:
            diff = KeyTable[ord(keyA[i])] - KeyTable[ord(keyB[i])]
            if diff > 0:
                return 1
            else:
                return -1
    return 0

#
# Always add edges to list such that edge is identified by
# lowest index node first. Note: cost to compute all these
# indices makes this slow. And, we're re-computing a node's
# index many times (typically 8x) each time we see that node.
# Can improve performance later
#
def addEdge(edgeList, nodeKey0, nodeKey1, isGhost):
#    idx0 = AsciiKeyToIndex(nodeKey0)
#    idx1 = AsciiKeyToIndex(nodeKey1)
#    if idx0 < idx1:
    if AsciiKeyCmp(nodeKey0, nodeKey1) < 0:
        edgeList.append(("%s%s"%(nodeKey0,nodeKey1),isGhost))
    else:
        edgeList.append(("%s%s"%(nodeKey1,nodeKey0),isGhost))

#
# Starting with a single line entry from a topology.txt file, output
# a series of KV pairs of the form ((nodeKey0,nodeKey1), zoneKey) where
# nodeKey0/1 are the nodes of the edges of the input zones. This 
# function is designed to be used in a flatMap method.
#
def zoneEdges(topoFileTextLine):
    zoneKey = topoFileTextLine[0:18]
    zoneInfo = topoFileTextLine[19:].split(',')
    isGhost = zoneInfo[0] == '1'
    zoneType = int(zoneInfo[1])
    nodeCnt = int(zoneInfo[2])
    nodeKeyList = zoneInfo[3:]
    retval = []
    if zoneType == 5: #VTK_TRIANGLE:
        addEdge(retval, nodeKeyList[0], nodeKeyList[1], isGhost)
        addEdge(retval, nodeKeyList[1], nodeKeyList[2], isGhost)
        addEdge(retval, nodeKeyList[2], nodeKeyList[0], isGhost)
    elif zoneType == 9: #VTK_QUAD:
        addEdge(retval, nodeKeyList[0], nodeKeyList[1], isGhost)
        addEdge(retval, nodeKeyList[1], nodeKeyList[2], isGhost)
        addEdge(retval, nodeKeyList[2], nodeKeyList[3], isGhost)
        addEdge(retval, nodeKeyList[3], nodeKeyList[0], isGhost)
    elif zoneType == 10: #VTK_TETRA:
        addEdge(retval, nodeKeyList[0], nodeKeyList[1], isGhost)
        addEdge(retval, nodeKeyList[1], nodeKeyList[2], isGhost)
        addEdge(retval, nodeKeyList[2], nodeKeyList[0], isGhost)
        addEdge(retval, nodeKeyList[0], nodeKeyList[3], isGhost)
        addEdge(retval, nodeKeyList[1], nodeKeyList[3], isGhost)
        addEdge(retval, nodeKeyList[2], nodeKeyList[3], isGhost)
    elif zoneType == 12: # VTK_HEXAHEDRON
        addEdge(retval, nodeKeyList[0], nodeKeyList[1], isGhost)
        addEdge(retval, nodeKeyList[1], nodeKeyList[2], isGhost)
        addEdge(retval, nodeKeyList[2], nodeKeyList[3], isGhost)
        addEdge(retval, nodeKeyList[3], nodeKeyList[0], isGhost)
        addEdge(retval, nodeKeyList[4], nodeKeyList[5], isGhost)
        addEdge(retval, nodeKeyList[5], nodeKeyList[6], isGhost)
        addEdge(retval, nodeKeyList[6], nodeKeyList[7], isGhost)
        addEdge(retval, nodeKeyList[7], nodeKeyList[4], isGhost)
        addEdge(retval, nodeKeyList[4], nodeKeyList[0], isGhost)
        addEdge(retval, nodeKeyList[5], nodeKeyList[1], isGhost)
        addEdge(retval, nodeKeyList[6], nodeKeyList[2], isGhost)
        addEdge(retval, nodeKeyList[7], nodeKeyList[3], isGhost)
    elif zoneType == 11: # VTK_VOXEL, yep its node order isn't same as hex
        addEdge(retval, nodeKeyList[0], nodeKeyList[1], isGhost)
        addEdge(retval, nodeKeyList[1], nodeKeyList[3], isGhost)
        addEdge(retval, nodeKeyList[3], nodeKeyList[2], isGhost)
        addEdge(retval, nodeKeyList[2], nodeKeyList[0], isGhost)
        addEdge(retval, nodeKeyList[4], nodeKeyList[5], isGhost)
        addEdge(retval, nodeKeyList[5], nodeKeyList[7], isGhost)
        addEdge(retval, nodeKeyList[7], nodeKeyList[6], isGhost)
        addEdge(retval, nodeKeyList[6], nodeKeyList[4], isGhost)
        addEdge(retval, nodeKeyList[4], nodeKeyList[0], isGhost)
        addEdge(retval, nodeKeyList[5], nodeKeyList[1], isGhost)
        addEdge(retval, nodeKeyList[6], nodeKeyList[2], isGhost)
        addEdge(retval, nodeKeyList[7], nodeKeyList[3], isGhost)
    elif zoneType == 13: #VTK_WEDGE:
        addEdge(retval, nodeKeyList[0], nodeKeyList[1], isGhost)
        addEdge(retval, nodeKeyList[1], nodeKeyList[2], isGhost)
        addEdge(retval, nodeKeyList[2], nodeKeyList[0], isGhost)
        addEdge(retval, nodeKeyList[3], nodeKeyList[4], isGhost)
        addEdge(retval, nodeKeyList[4], nodeKeyList[5], isGhost)
        addEdge(retval, nodeKeyList[5], nodeKeyList[3], isGhost)
        addEdge(retval, nodeKeyList[0], nodeKeyList[3], isGhost)
        addEdge(retval, nodeKeyList[1], nodeKeyList[4], isGhost)
        addEdge(retval, nodeKeyList[2], nodeKeyList[5], isGhost)
    elif zoneType == 14: #VTK_PYRAMID:
        addEdge(retval, nodeKeyList[0], nodeKeyList[1], isGhost)
        addEdge(retval, nodeKeyList[1], nodeKeyList[2], isGhost)
        addEdge(retval, nodeKeyList[2], nodeKeyList[3], isGhost)
        addEdge(retval, nodeKeyList[3], nodeKeyList[0], isGhost)
        addEdge(retval, nodeKeyList[0], nodeKeyList[4], isGhost)
        addEdge(retval, nodeKeyList[1], nodeKeyList[4], isGhost)
        addEdge(retval, nodeKeyList[2], nodeKeyList[4], isGhost)
        addEdge(retval, nodeKeyList[3], nodeKeyList[4], isGhost)
    return retval

#
# Not currently used. Would be used in a mapPartitions method.
# Need to investigate using python on full partition of data
# *b4* launching Spark methods on it
#
def zoneEdgesInPartition(lines):
    retval = set([])
    for line in lines:
        retval.union(set(zoneEdges(line)))
    return list(retval)
    
#
# Logic necessary to decode mesh entity keys produced by hdfs_export utils
#
# Partition based on block field of key
#
def PartitionByBlockKeyField(key):
    return AsciiKeyToIndex(key[8:12])

if __name__ == "__main__":
  if len(sys.argv) != 5:
    print("Usage: WireFrame <dbfile> <time-spec> <mesh> <keyBase>", file=sys.stderr)
    exit(-1)

  sc = SparkContext()

  #
  # Read list of coords. . .
  #     .map each text line into (%s, [%g,%g,%g]) KV-pairs keyed by node key
  # 
  coords = sc.textFile("file:/home/training/vdbroot.silo/miller86/%s/%s/%s/*/coords.txt.bz2"%\
     (sys.argv[1],sys.argv[2],sys.argv[3]))\
     .map(lambda line: (line[0:18],[float(x) for x in line[19:].split(',')[1:]]))

  #
  # Read list of zones...
  #     .flatMap each zone into num-edges KV pairs (%s, bool) where key is catenation of
  #         the edge's node keys and bool is whether the edge is from a ghost-zone or not.
  #     .reduceByKey reducing all dup'd keys (edges) to a single (%s, True) entry. Note that
  #         any edgekeys that occur only once, never enter the reduce method. These will 
  #         be either extreme edges from ghosts (where ghost bool is True) or extreme edges
  #         from non-ghosts (where ghost bool is False).
  #     .filter taking only those KV pairs where the bool is False; that is only those
  #         extreme edges that are NOT from ghosts
  #
  wireEdges = sc.textFile("file:/home/training/vdbroot.silo/miller86/%s/%s/%s/*/topology.txt.bz2"%\
     (sys.argv[1],sys.argv[2],sys.argv[3]))\
     .flatMap(lambda line: zoneEdges(line))\
     .reduceByKey(lambda a,b: True)\
     .filter(lambda pair: not pair[1])

  #
  # Flat map the wireEdges PairRDD yielding (%s,1) pairs keyed by the
  # node keys and then distinct() it to get only the unique nodes.
  # For each edge, we split the catenated edge key back out to yield
  # the two node keys that comprise it and use a phony value of '1'
  # for the values in the yielded pairs
  #
  wireNodes = wireEdges\
      .flatMap(lambda pair: [(pair[0][0:18],1),(pair[0][18:],1)])\
      .distinct()

  #
  # From coords, take only those entries for which nodes exist in
  # wireNodes. This will produce a PairRDD with pairs (%s, ([x,y,z],1))
  # keyed by the nodes of wireNodes.
  #
  wireCoords = coords.join(wireNodes).map(lambda x: (x[0], x[1][0]))

  #
  # The wire-frame object should be small enough to bring back to the
  # master. So, collect them into their Python list equivs on the master
  #
  collectedEdges = wireEdges.collect()
  collectedCoords = wireCoords.collect()

  #
  # Finally, master writes text files conforming to same structure as HDFS
  # export files for the coords and the edges (topology).
  # We really should 'insert' this data into the HDFS database but
  # for now, we'll just write external files from the master to cwd
  #
  print("WireFrame yielded %d edges and %d nodes"%(len(collectedEdges),len(collectedCoords)))
  f = os.popen("bzip2 > WireFrame_%s_coords.txt.bz2"%sys.argv[3],"w")
  for c in collectedCoords:
      f.write("%s,0,%g,%g,%g\n"%(c[0],c[1][0],c[1][1],c[1][2]))
  f.close()
  f = os.popen("bzip2 > WireFrame_%s_topology.txt.bz2"%sys.argv[3],"w")
  keyBase = AsciiKeyToIndex(sys.argv[4])
  keyBase /= 8
  keyBase += 1
  i = 0
  for e in collectedEdges:
      # key,ghost,type,count,nodeKeys
      f.write("%s,0,3,2,%s,%s\n"%(IndexToAsciiKey("",(keyBase+i)*8+1,len(sys.argv[4])),e[0][0:18],e[0][18:]))
      i += 1
  f.close()
