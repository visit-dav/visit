import sys
from pyspark import SparkContext
from pyspark import SparkConf

min = 0
max = 1

# Input is (key, (min,max))
def minmax(a,b):
    if a[1][min] <= b[1][min]:
        outmin = a[1][min]
    else:
        outmin = b[1][min]
    if a[1][max] >= b[1][max]:
        outmax = a[1][max]
    else:
        outmax = b[1][max]
    return (a[0], (outmin,outmax))

#
# Perfrom the equivalent of a VisIt MinMax query
# Do so in a single reduce operation by updating both min and max
# Note that we double memory needed but reduce execution time over MinMax
# because we create a KV-RDD whose values are min,max pairs and update
# both. But, we then also loose the knowledge of which element got the
# min and which got the max. We could fix this at the expense of even more
# memory.
#
if __name__ == "__main__":
  if len(sys.argv) < 4:
    print("Usage: MinMax <dbfile> <mesh> <var>", file=sys.stderr)
    exit(-1)

  sc = SparkContext()
  var_kv = sc.textFile("file:/home/training/vdbroot.silo/miller86/%s/*/%s/*/%s.txt.bz2"%\
     (sys.argv[1],sys.argv[2],sys.argv[3])).map(lambda line: (line[0:18], (float(line[19:]),float(line[19:]))))

  minmax_kv = var_kv.reduce(minmax)

  print("Minimum value is %g"%minmax_kv[1][min])
  print("Maximum value is %g"%minmax_kv[1][max])
