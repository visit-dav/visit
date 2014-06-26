
#ifndef PARTITION_H
#define PARTITION_H

#include <vector>
using std::vector;

void PartitionWeights( const vector<int>& wlst,  // weight list
                       int npart, vector< vector<int> >& part );

#endif
