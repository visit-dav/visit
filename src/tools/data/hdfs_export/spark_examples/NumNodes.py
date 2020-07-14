import sys
from pyspark import SparkContext
from pyspark import SparkConf

#
# Performs equivalent of a VisIt NumNodes query over all blocks of specified mesh
# Returns total node count and count of just ghost nodes
#
if __name__ == "__main__":
  if len(sys.argv) < 3:
    print("Usage: NumNodes <dbfile> <mesh>", file=sys.stderr)
    exit(-1)

  sc = SparkContext()
  coord_data = sc.textFile("file:/home/training/vdbroot.silo/miller86/%s/*/%s/*/coords.txt.bz2"%\
     (sys.argv[1],sys.argv[2]))

  # 0.087 seconds on laptop VM
  # find foo -name coords.txt.bz2 -exec bzcat {} \; | wc -l
  # count of all nodes
  all_node_count = coord_data.count()

  # .203 seconds
  # find foo -name coords.txt.bz2 -exec bzcat {} \; | grep '0[a-zA-Z0-9#%]*,1,' | wc -l
  # count of just ghost nodes
  ghost_node_count = coord_data.filter(lambda line: line[19]=='1').count()

  # 9.2 seconds (on laptop VM running pyspark) cating output to /dev/null
  print("Number of nodes is", all_node_count)
  print("Number of ghost nodes is", ghost_node_count)
