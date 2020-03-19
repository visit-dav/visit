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

def rotate(l, offset):
    return l[offset:] + l[:offset]

#
# Normalized face is lowest node first and ordered
# such that second lowest node is second
#
def normalize_face(l):
    minidx = 0
    minval = l[0]
    for i in range(1,len(l)):
        if AsciiKeyCmp(l[1],minval) < 0:
            minval = l[i]
            minidx = i
    l = rotate(l,minidx)
    if l[len(l)-1] < l[1]:
        l.reverse()
        l = rotate(l,-1)
    return l
        
def addFace(faceList, faceNodes, isGhost):
    faceNodes = normalize_face(faceNodes)
    faceList.append(("%s"%"".join(faceNodes),isGhost))

#
# Starting with a single line entry from a topology.txt file, output
# a series of KV pairs of the form (faceKey, isGhost) where
#
def zoneFaces(topoFileTextLine):
    zoneKey = topoFileTextLine[0:18]
    zoneInfo = topoFileTextLine[19:].split(',')
    isGhost = zoneInfo[0] == '1'
    zoneType = int(zoneInfo[1])
    nodeCnt = int(zoneInfo[2])
    nl = zoneInfo[3:]
    retval = []
    if zoneType == 5: #VTK_TRIANGLE:
        addFace(retval, nl, isGhost)
    elif zoneType == 9: #VTK_QUAD:
        addFace(retval, nl, isGhost)
    elif zoneType == 10: #VTK_TETRA:
        addFace(retval, [nl[0], nl[1], nl[2]], isGhost)
        addFace(retval, [nl[0], nl[1], nl[3]], isGhost)
        addFace(retval, [nl[1], nl[2], nl[3]], isGhost)
        addFace(retval, [nl[2], nl[0], nl[3]], isGhost)
    elif zoneType == 12: # VTK_HEXAHEDRON
        addFace(retval, [nl[0], nl[1], nl[2], nl[3]], isGhost)
        addFace(retval, [nl[0], nl[1], nl[5], nl[4]], isGhost)
        addFace(retval, [nl[1], nl[2], nl[6], nl[5]], isGhost)
        addFace(retval, [nl[2], nl[3], nl[7], nl[6]], isGhost)
        addFace(retval, [nl[3], nl[0], nl[4], nl[7]], isGhost)
        addFace(retval, [nl[4], nl[5], nl[6], nl[7]], isGhost)
    elif zoneType == 11: # VTK_VOXEL, yep its node order isn't same as hex
        addFace(retval, [nl[0], nl[1], nl[3], nl[2]], isGhost)
        addFace(retval, [nl[0], nl[1], nl[5], nl[4]], isGhost)
        addFace(retval, [nl[1], nl[3], nl[7], nl[5]], isGhost)
        addFace(retval, [nl[3], nl[2], nl[6], nl[7]], isGhost)
        addFace(retval, [nl[2], nl[0], nl[4], nl[6]], isGhost)
        addFace(retval, [nl[4], nl[5], nl[7], nl[6]], isGhost)
    elif zoneType == 13: #VTK_WEDGE:
        addFace(retval, [nl[0], nl[1], nl[2]], isGhost)
        addFace(retval, [nl[3], nl[4], nl[2]], isGhost)
        addFace(retval, [nl[0], nl[1], nl[4], nl[3]], isGhost)
        addFace(retval, [nl[1], nl[2], nl[5], nl[4]], isGhost)
        addFace(retval, [nl[2], nl[0], nl[3], nl[5]], isGhost)
    elif zoneType == 14: #VTK_PYRAMID:
        addFace(retval, [nl[0], nl[1], nl[2], nl[3]], isGhost)
        addFace(retval, [nl[0], nl[1], nl[4]], isGhost)
        addFace(retval, [nl[1], nl[2], nl[4]], isGhost)
        addFace(retval, [nl[2], nl[3], nl[4]], isGhost)
        addFace(retval, [nl[3], nl[9], nl[4]], isGhost)
    return retval

def faceNodes(faceKey):
    retval = []
    for i in range(0,len(faceKey),18):
        retval.append((faceKey[i:i+18],1))
    return retval

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
  #         the face's node keys and bool is whether the face is from a ghost-zone or not.
  #     .reduceByKey reducing all dup'd keys (faces) to a single (%s, True) entry. Note that
  #         any face keys that occur only once, never enter the reduce method. These will 
  #         be either extreme faces from ghosts (where ghost bool is True) or extreme faces 
  #         from non-ghosts (where ghost bool is False).
  #     .filter taking only those KV pairs where the bool is False; that is only those
  #         extreme faces that are NOT from ghosts (e.g. the true exterior faces)
  #
  extFaces = sc.textFile("file:/home/training/vdbroot.silo/miller86/%s/%s/%s/*/topology.txt.bz2"%\
     (sys.argv[1],sys.argv[2],sys.argv[3]))\
     .flatMap(lambda line: zoneFaces(line))\
     .reduceByKey(lambda a,b: True)\
     .filter(lambda pair: not pair[1])

  #
  # Flat map the extFaces PairRDD yielding (%s,1) pairs keyed by the
  # node keys and then distinct() it to get only the unique nodes.
  # For each face, we split the catenated face key back out to yield
  # the node keys that comprise it and use a phony value of '1'
  # for the values in the yielded pairs
  #
  extNodes = extFaces\
      .flatMap(lambda pair: faceNodes(pair[0]))\
      .distinct()

  #
  # From coords, take only those entries for which nodes exist in
  # extNodes. This will produce a PairRDD with pairs (%s, ([x,y,z],1))
  # keyed by the nodes of extNodes.
  #
  extCoords = coords.join(extNodes).map(lambda x: (x[0], x[1][0]))

  #
  # The set of faces should be small enough to bring back to the
  # master. So, collect them into their Python list equivs on the master
  #
  collectedFaces = extFaces.collect()
  collectedCoords = extCoords.collect()

  #
  # Finally, master writes text files conforming to same structure as HDFS
  # export files for the coords and the edges (topology).
  # We really should 'insert' this data into the HDFS database but
  # for now, we'll just write external files from the master to cwd
  #
  print("RuledFrame yielded %d faces and %d nodes"%(len(collectedFaces),len(collectedCoords)))
  f = os.popen("bzip2 > RuledFrame_%s_%s_coords.txt.bz2"%(sys.argv[1],sys.argv[3]),"w")
  for c in collectedCoords:
      f.write("%s,0,%g,%g,%g\n"%(c[0],c[1][0],c[1][1],c[1][2]))
  f.close()
  f = os.popen("bzip2 > RuledFrame_%s_%s_topology.txt.bz2"%(sys.argv[1],sys.argv[3]),"w")
  keyBase = AsciiKeyToIndex(sys.argv[4])
  keyBase /= 8
  keyBase += 1
  i = 0
  for e in collectedFaces:
      # key,ghost,type,count,nodeKeys
      if len(e[0]) == 3*18:
          f.write("%s,0,5,3,%s,%s,%s\n"%(IndexToAsciiKey("",(keyBase+i)*8+1,len(sys.argv[4])),e[0][0:18],e[0][18:36],e[0][36:]))
      else:
          f.write("%s,0,9,4,%s,%s,%s,%s\n"%(IndexToAsciiKey("",(keyBase+i)*8+1,len(sys.argv[4])),e[0][0:18],e[0][18:36],e[0][36:54],e[0][54:]))
      i += 1
  f.close()
