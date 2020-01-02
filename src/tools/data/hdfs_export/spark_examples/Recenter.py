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

#
# Currently, this is just doing a nodal->zonal recenter
#
if __name__ == "__main__":
  if len(sys.argv) < 4:
    print("Usage: Recenter <dbfile> <mesh> <nodal-var>", file=sys.stderr)
    exit(-1)

  sc = SparkContext()

  # topology.txt is row-by-row: zonekey,ghost,type,node-cnt,node-keys
  #
  # this takes the line-by-line zone info and produces a KV rdd with nodekey as key and zonekey as value.
  # This results in many kv-pairs with same key because many zones share the same node
  # The first 'map' takes ascii lines from textFile and produces tuples (zoneKey, [nodeKeys])
  # the flatMapValues splits the lists for each zoneKey up into one kv-pair for each nodeKey
  # the last map reverses order of keys and values.
  #
  zones = sc.textFile("file:/home/training/vdbroot.silo/miller86/%s/*/%s/*/topology.txt.bz2"%\
     (sys.argv[1],sys.argv[2])).map(lambda line: (line[0:18], (line[19:].split(',')[3:]))).flatMapValues(lambda x: x).map(lambda x: (x[1],x[0]))

  #
  # read var data, take a moment to compute number of blocks
  #
  var_data = sc.textFile("file:/home/training/vdbroot.silo/miller86/%s/*/%s/*/%s.txt.bz2"%\
     (sys.argv[1],sys.argv[2],sys.argv[3]))
  nblocks = var_data.map(lambda line: line[8:12]).distinct().count()
  var_kv = var_data.map(lambda line: (line[0:18], float(line[19:])))

  #
  # produces an rdd with keys as nodes (will have many items with same key) and values as
  # zonekey and nodal value to be contributed to that zone. But, only need to values of that
  #
  # Here we're joining two KV rdds which are keyed using nodeKeys The first is the zone RDD which is
  # (nodeKey, zoneKey) pairs. The second is variable which is (nodeKey, value) pairs. The result is
  # (nodekey, (zonekey, value)) pairs. But, we only need the values() of this RDD. Thats what the
  # .values() does producing an RDD of (zoneKey value) pairs. Then we use combineByKey producing a
  # new RDD, (zoneKey, (sum,count)) and finally a map producing (zoneKey, sum/cnt).
  #
  zones_with_vals = list(zones.join(var_kv).values())\
                         .combineByKey(lambda value: (value, 1),\
                                       lambda x, value: (x[0] + value, x[1] + 1),\
                                       lambda x, y: (x[0] + y[0], x[1] + y[1]))\
                         .map(lambda label_value_sum_count: (label_value_sum_count[0], label_value_sum_count[1][0] / label_value_sum_count[1][1]))\
                         .partitionBy(nblocks,ParitionByBlockKeyField)
  zones_with_vals.saveAsTextFile("Recenter_%s.txt"%sys.argv[3]) 
