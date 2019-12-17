import sys
from pyspark import SparkContext
from pyspark import SparkConf

#
# Logic necessary to decode mesh entity keys produced by hdfs_export utils
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
# Partition based on block field of key
#
def ParitionByBlockKeyField(key):
    return AsciiKeyToIndex(key[8:12])

if __name__ == "__main__":
  if len(sys.argv) < 5:
    print("Usage: Operator+ <dbfile> <mesh> <var1> <var2>", file=sys.stderr)
    exit(-1)

  sc = SparkContext()
  var1_data = sc.textFile("file:/home/training/vdbroot.silo/miller86/%s/*/%s/*/%s.txt.bz2"%\
     (sys.argv[1],sys.argv[2],sys.argv[3]))
  nblocks = var1_data.map(lambda line: line[8:12]).distinct().count()
  var1_kv = var1_data.map(lambda line: (line[0:18], float(line[19:])))
  var2_kv = sc.textFile("file:/home/training/vdbroot.silo/miller86/%s/*/%s/*/%s.txt.bz2"%\
     (sys.argv[1],sys.argv[2],sys.argv[4])).map(lambda line: (line[0:18], float(line[19:])))

  # forms a single kv-rdd with values from both variables where same node/zone appears
  # as key of multiple pairs (one from var1 and the other from var2)
  var12_kv = var1_kv.union(var2_kv)

  # aggregates all values with same key by summing the value
  # Here, you could replace '+' operator with any other of the operators in VisIt's binary
  # operators and achieve the equivalent outputs
  sum = var12_kv.reduceByKey(lambda a,b: a+b).partitionBy(nblocks,ParitionByBlockKeyField)

  # This will save the partitioned data, block by block to HDFS (by default)
  # We need to add compression too.
  # 15 seconds
  sum.saveAsTextFile("Operator+%s_%s.txt"%(sys.argv[3],sys.argv[4]))
