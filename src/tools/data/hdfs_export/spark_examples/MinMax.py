import sys
from pyspark import SparkContext
from pyspark import SparkConf

def min(a,b):
    if a[1] <= b[1]:
        return a
    return b

def max(a,b):
    if a[1] >= b[1]:
        return a
    return b

#
# Perform the equiv. of a VisIt MinMax query
#
if __name__ == "__main__":
  if len(sys.argv) < 4:
    print("Usage: MinMax <dbfile> <mesh> <var>", file=sys.stderr)
    exit(-1)

  sc = SparkContext()
  var_kv = sc.textFile("file:/home/training/vdbroot.silo/miller86/%s/*/%s/*/%s.txt.bz2"%\
     (sys.argv[1],sys.argv[2],sys.argv[3])).map(lambda line: (line[0:18], float(line[19:])))
  #
  # we compute min reduce and max reduce separately. More execution time but less memory
  #

  # find foo -name v.txt.bz2 -exec bzcat {} \; | sort -n -k 2 -t',' -r | tail -n 1
  # 0.39 seconds in shell
  min_kv = var_kv.reduce(min)

  # find foo -name v.txt.bz2 -exec bzcat {} \; | sort -n -k 2 -t',' | tail -n 1
  # 0.39 seconds in shell
  max_kv = var_kv.reduce(max)

  # 9 seconds on laptop VM
  print("Minimum value at %s is %g\n"%min_kv)
  print("Maximum value at %s is %g\n"%max_kv)
